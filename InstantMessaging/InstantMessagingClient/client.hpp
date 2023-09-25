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

class message_handler {
public:
	message read_message_;

	void render_messages();

	void save_incoming_message(const message& message);

	void save_incoming_message(message&& message);

	bool write_queue_empty();

	void enqueue_message_to_be_written(const message& message);

	void enqueue_message_to_be_written(message&& message);

	message& message_to_be_written();

	void pop_message_to_be_written();

	message_handler(message_handler&& other) noexcept : read_message_(std::move(other.read_message_)), read_messages_mutex_(),
		read_messages_old_size_(other.read_messages_old_size_), read_messages_(std::move(other.read_messages_)), write_messages_(std::move(other.write_messages_)) { }

	message_handler& operator=(message_handler&& other) noexcept {
		read_message_ = std::move(other.read_message_);
		read_messages_old_size_ = other.read_messages_old_size_;
		read_messages_ = std::move(other.read_messages_);
		write_messages_ = std::move(other.write_messages_);
		return *this;
	}

	message_handler() : read_message_(), read_messages_mutex_(), read_messages_old_size_(0), read_messages_(), write_messages_() { }

private:
	static constexpr int max_messages = 100;

	std::mutex read_messages_mutex_;
	std::size_t read_messages_old_size_;
	std::deque<std::string> read_messages_;
	std::deque<message> write_messages_;

	message_handler(const message_handler&) = delete;

	message_handler& operator=(const message_handler&) = delete;
};

class client {
public:
	void do_connect();

	void receive_header();

	void receive_body();

	void write(char content[]);

	void close();

	bool is_open();

	void clear_error_message();

	void render_messages();

	static client create_client(char host[], int port, char username[], boost::asio::io_context& io_context);

	std::optional<std::string>& get_error_message();

	client(client&& other) noexcept : username_(), message_handler_(std::move(other.message_handler_)), socket_(std::move(other.socket_)), io_context_(other.io_context_), endpoints_(std::move(other.endpoints_)), error_message_(std::move(other.error_message_)), is_connected_(other.is_connected_.load()) {
		strcpy_s(username_, other.username_);
	}

	client& operator=(client&& other) noexcept {
		strcpy_s(username_, other.username_);
		message_handler_ = std::move(other.message_handler_);
		socket_ = std::move(other.socket_);
		io_context_ = other.io_context_;
		endpoints_ = std::move(other.endpoints_);
		error_message_ = std::move(other.error_message_);
		is_connected_ = other.is_connected_.load();
		return *this;
	}

private:
	char username_[20];
	message_handler message_handler_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context* io_context_;
	boost::asio::ip::tcp::resolver::results_type endpoints_;
	std::optional<std::string> error_message_ = std::nullopt;
	std::atomic<bool> is_connected_ = false;

	client(boost::asio::io_context* io_context, boost::asio::ip::tcp::resolver::results_type&& endpoints, char username[]) : io_context_(io_context), socket_(*io_context), endpoints_(std::move(endpoints)) {
		strcpy_s(username_, username);
	}

	void save_new_message(message&& message);

	void write_new_message();

	void handle_error(const boost::system::error_code& ec);
};