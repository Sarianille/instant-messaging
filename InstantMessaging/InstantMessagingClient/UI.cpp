#include "ui.hpp"
#include "errors.hpp"

void login_window::render() {
	int old_port_number = port_number;
    logged_in = false;

    ImGui::Begin("Log in");
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
            error_msg = errors::host_error_msg;
            ImGui::OpenPopup("Log in error.");
        }
        else if (!is_valid_info(username_buffer)) {
            error_msg = errors::username_error_msg;
            ImGui::OpenPopup("Log in error.");

        }
        else {
            logged_in = true;
        }
    }

    if (ImGui::BeginPopupModal("Log in error.")) {
        ImGui::Text(error_msg);
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
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
    ImGui::Begin("Chatroom", &running);

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
        client = client::create_client(host, port, username, *new_io_context);
        io_context = std::move(new_io_context);

        client->do_connect();
        thread = std::thread([&io_context = this->io_context]() { io_context->run(); });
    }
    catch (std::exception& e) {
        error_msg = errors::exception_error_msg;
        ImGui::OpenPopup("Log in error.");
    }
}

void ui::render() {
	if (show_login_window) {
		login_window.render();
	}

    if (login_window.logged_in) {
		chat_window.open_chat_window(login_window.get_host_name(), login_window.get_port_number(), login_window.get_username());

        show_chat_window = true;
        show_login_window = false;
	}

	if (show_chat_window) {
		chat_window.render();
	}
}