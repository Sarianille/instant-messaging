#pragma once

#include "..\message.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <iostream>
#include <string>
#include <optional>
#include <mutex>
#include <functional>
#include <sstream>
#include <atomic>
#include <thread>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class client {
public:
	void do_connect();

	void receive_header();

	void receive_body();

	void write(char content[]);

	void close();

	bool is_open();

	void render_messages();

	static client create_client(char host[], int port, char username[], boost::asio::io_context& io_context);

	std::optional<std::string>& get_error_message();

	void clear_error_message();

	client(client&& other) noexcept : socket_(std::move(other.socket_)), io_context_(other.io_context_), endpoints_(std::move(other.endpoints_)), read_message_(std::move(other.read_message_)), read_messages_(std::move(other.read_messages_)), write_messages_(std::move(other.write_messages_)), read_messages_old_size_(other.read_messages_old_size_), error_message_(other.error_message_), read_messages_mutex_() {
		strcpy_s(username_, other.username_);
	}

	client& operator=(client&& other) noexcept {
		socket_ = std::move(other.socket_);
		io_context_ = other.io_context_;
		endpoints_ = std::move(other.endpoints_);
		read_message_ = std::move(other.read_message_);
		read_messages_ = std::move(other.read_messages_);
		write_messages_ = std::move(other.write_messages_);
		strcpy_s(username_, other.username_);
		read_messages_old_size_ = other.read_messages_old_size_;
		error_message_ = other.error_message_;
		return *this;
	}

private:
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context* io_context_;
	boost::asio::ip::tcp::resolver::results_type endpoints_;
	char username_[20];
	message read_message_;
	std::deque<std::string> read_messages_;
	std::deque<message> write_messages_;
	std::mutex read_messages_mutex_;
	int read_messages_old_size_;
	static constexpr int max_messages = 100;
	static constexpr int connection_aborted_error = 1236;
	std::optional<std::string> error_message_ = std::nullopt;
	std::atomic<bool> is_connected_ = false;

	client(boost::asio::io_context* io_context, boost::asio::ip::tcp::resolver::results_type&& endpoints, char username[]) : io_context_(io_context), socket_(*io_context), endpoints_(std::move(endpoints)) {
		strcpy_s(username_, username);
	}

	void save_new_message(message&& message);

	void write_new_message();

	void handle_error(const boost::system::error_code& ec);
};