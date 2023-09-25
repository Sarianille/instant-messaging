#include "ui.hpp"
#include "errors.hpp"

void login_window::render() {
	int old_port_number = port_number;
    logged_in = false;

    ImGui::Begin("Log in", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Please choose host:");
    ImGui::InputText("###Host:", host_name_buffer, host_name_buffer_size, 1);
    ImGui::Text("Please choose port:");
    if (ImGui::InputInt("###Port:", &port_number, 0)) {
        if (!is_valid_port(port_number)) {
            port_number = old_port_number;
        }
    }
    ImGui::Text("Please choose username:");
    bool logged_in_with_enter = ImGui::InputText("###Username:", username_buffer, username_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);

    if (ImGui::Button("Log in") || logged_in_with_enter) {
        if (!is_valid_info(host_name_buffer)) {
            error_handler.set_error_message(errors::host_error_msg);
        }
        else if (!is_valid_info(username_buffer)) {
            error_handler.set_error_message(errors::username_error_msg);
        }
        else {
            logged_in = true;
        }
    }

    ImGui::End();
}

const char* login_window::get_host_name() {
	return host_name_buffer;
}

int login_window::get_port_number() {
	return port_number;
}

const char* login_window::get_username() {
	return username_buffer;
}

bool login_window::is_valid_port(int port_number) {
    return (0 <= port_number && port_number <= 65535);
}

bool login_window::is_valid_info(char name_buffer[]) {
	return strlen(name_buffer) > 0;
}

void chat_window::render() {
    ImGui::Begin("Chatroom", &show_chat_window);

    ImGui::BeginChild("Chat", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    client->render_messages();
    ImGui::EndChild();

    if (client->is_open()) {
        ImGui::PushItemWidth(-45);
        bool message_sent_with_enter = ImGui::InputText("###Message:", message_buffer, message_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Send") || message_sent_with_enter) {
            if (strlen(message_buffer) != 0)
            {
                client->write(message_buffer);
                memset(message_buffer, 0, message_buffer_size * sizeof(char));
            }
        }
    }
    else {
        ImGui::Text("Connecting...");
    }

    ImGui::End();
}

void chat_window::open_chat_window(const char host[], int port, const char username[]) {
    try {
        // This is required to avoid destroying the old io_context before the socket contained within the client
        std::unique_ptr<boost::asio::io_context> new_io_context = std::make_unique<boost::asio::io_context>();
        client = client::create_client(host, port, username, *new_io_context, error_handler);
        io_context = std::move(new_io_context);

        client->do_connect();
        thread = std::thread([&io_context = this->io_context]() { 
            try {
                io_context->run();
            }
            catch (std::exception& e) {
                std::cout << e.what();
                throw;
            }
            });
    }
    catch (std::exception& e) {
        error_handler.set_error_message(errors::exception_error_msg);
    }
}

void chat_window::close_chat_window() {
	client->close();
	thread->join();
	thread.reset();
}

void ui::render() {
	if (login_window.show_login_window) {
		login_window.render();
        error_handler.potentially_display_error_message(errors::login_error_title, []() {});
	}

    if (login_window.logged_in) {
		chat_window.open_chat_window(login_window.get_host_name(), login_window.get_port_number(), login_window.get_username());

        chat_window.show_chat_window = true;
        login_window.show_login_window = false;
        login_window.logged_in = false;

        if (error_handler.error_message_.has_value()) {
            error_handler.potentially_display_error_message(errors::login_error_title, []() {});

            chat_window.show_chat_window = false;
            login_window.show_login_window = true;
        }
	}

	if (chat_window.show_chat_window) {
		chat_window.render();
        error_handler.potentially_display_error_message(errors::chatroom_error_title, [this]() {
            chat_window.show_chat_window = false;
            });
	}

    if (!login_window.show_login_window && !chat_window.show_chat_window) {
        chat_window.close_chat_window();
        login_window.show_login_window = true;
    }
}