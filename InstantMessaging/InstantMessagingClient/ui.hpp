#pragma once

#include "client.hpp"

class login_window {
public:
    bool logged_in = false;

    login_window() {
        memset(host_name_buffer, 0, host_name_buffer_size * sizeof(char));
        memset(username_buffer, 0, username_buffer_size * sizeof(char));
    }

    void render();

    const char* get_host_name();

    int get_port_number();

    const char* get_username();

private:
	int port_number = 0;
    static constexpr int host_name_buffer_size = 256;
    static constexpr int username_buffer_size = 20;
    char host_name_buffer[host_name_buffer_size];
    char username_buffer[username_buffer_size];
    const char* error_msg = nullptr;

    bool is_valid_port(int port_number);

    bool is_valid_info(char name_buffer[]);
};

class chat_window {
public:
    bool running = true;

    chat_window() {
        memset(message_buffer, 0, message_buffer_size * sizeof(char));
	}

    void render();

    void open_chat_window(const char host[], int port, const char username[]);

private:
    std::unique_ptr<boost::asio::io_context> io_context;
    std::optional<std::thread> thread;
	std::optional<client> client = std::nullopt;
    static constexpr int message_buffer_size = 1000;
    char message_buffer[message_buffer_size];
};

class ui {
public:
    void render();

private:
	login_window login_window;
	chat_window chat_window;
	bool show_login_window = true;
	bool show_chat_window = false;
};