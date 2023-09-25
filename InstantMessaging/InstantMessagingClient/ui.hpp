#pragma once

#include "client.hpp"

class login_window {
public:
    bool show_login_window = true;
    bool logged_in = false;

    login_window(errors::error_handler& error_handler) : error_handler(error_handler) {
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
    errors::error_handler& error_handler;

    bool is_valid_port(int port_number);

    bool is_valid_info(char name_buffer[]);
};

class chat_window {
public:
    bool show_chat_window = false;

    chat_window(errors::error_handler& error_handler) : error_handler(error_handler) {
        memset(message_buffer, 0, message_buffer_size * sizeof(char));
	}

    void render();

    void open_chat_window(const char host[], int port, const char username[]);

    void close_chat_window();

private:
    std::unique_ptr<boost::asio::io_context> io_context;
    std::optional<std::thread> thread;
	std::optional<client> client = std::nullopt;
    errors::error_handler& error_handler;
    static constexpr int message_buffer_size = 1000;
    char message_buffer[message_buffer_size];
};

class ui {
public:
    ui() : error_handler(), login_window(error_handler), chat_window(error_handler) {}

    void render();

private:
    errors::error_handler error_handler;
	login_window login_window;
	chat_window chat_window;
};