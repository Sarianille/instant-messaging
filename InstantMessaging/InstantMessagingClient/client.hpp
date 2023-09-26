#pragma once

#include "..\message.hpp"
#include "errors.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <iostream>
#include <string>
#include <optional>
#include <mutex>
#include <sstream>
#include <atomic>
#include <thread>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class message_handler {
public:
	message read_message_;

	/// <summary>
	/// Handles rendering of messages.
	/// </summary>
	void render_messages();

	/// <summary>
	/// Saves a message to be rendered.
	/// </summary>
	/// <param name="message">Message to be saved.</param>
	void save_incoming_message(const message& message);
	void save_incoming_message(message&& message);

	/// <summary>
	/// Checks if there are any messages to be sent.
	/// </summary>
	/// <returns>Whether the queue is empty or not.</returns>
	bool write_queue_empty();

	/// <summary>
	/// Enqueues a message to be sent.
	/// </summary>
	/// <param name="message">Message to be sent.</param>
	void enqueue_message_to_be_written(const message& message);
	void enqueue_message_to_be_written(message&& message);

	/// <summary>
	/// Returns the message to be currently sent.
	/// </summary>
	/// <returns>Message to be sent.</returns>
	message& message_to_be_written();

	/// <summary>
	/// Pops the recently sent message.
	/// </summary>
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
	/// <summary>
	/// Asynchronously connects the client to the server and starts receiving messages.
	/// </summary>
	void do_connect();

	/// <summary>
	/// Receives the header of the message asynchonously.
	/// </summary>
	void receive_header();

	/// <summary>
	/// Receives the body of the message asynchronously.
	/// </summary>
	void receive_body();

	/// <summary>
	/// Creates a message from the given content and enqueues it to be sent.
	/// </summary>
	/// <param name="content">Message content.</param>
	void write(char content[]);

	/// <summary>
	/// Closes the connection to the server.
	/// </summary>
	void close();

	/// <summary>
	/// Checks if the client is connected to the server.
	/// </summary>
	/// <returns>Whether the client is connected.</returns>
	bool is_open();

	/// <summary>
	/// Handles rendering of messages.
	/// </summary>
	void render_messages();

	/// <summary>
	/// Creates a client with the given parameters.
	/// </summary>
	/// <param name="host">Server to which the client connects.</param>
	/// <param name="port">Port to which the client connects.</param>
	/// <param name="username">Client's username in the chatroom.</param>
	/// <param name="io_context">Context in which client's async methods are run in.</param>
	/// <param name="error_handler">Class used for handling errors.</param>
	/// <returns>Created client.</returns>
	static client create_client(const char host[], int port, const char username[], boost::asio::io_context& io_context, errors::error_handler& error_handler);

	client(client&& other) noexcept : username_(), message_handler_(std::move(other.message_handler_)), error_handler_(other.error_handler_), 
		socket_(std::move(other.socket_)), io_context_(other.io_context_), endpoints_(std::move(other.endpoints_)), is_connected_(other.is_connected_.load()) {
		strcpy_s(username_, other.username_);
	}

	client& operator=(client&& other) noexcept {
		strcpy_s(username_, other.username_);
		message_handler_ = std::move(other.message_handler_);
		socket_ = std::move(other.socket_);
		io_context_ = other.io_context_;
		endpoints_ = std::move(other.endpoints_);
		is_connected_ = other.is_connected_.load();
		return *this;
	}

private:
	char username_[20];
	message_handler message_handler_;
	errors::error_handler& error_handler_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context* io_context_;
	boost::asio::ip::tcp::resolver::results_type endpoints_;
	std::atomic<bool> is_connected_ = false;

	client(boost::asio::io_context* io_context, boost::asio::ip::tcp::resolver::results_type&& endpoints, const char username[], errors::error_handler& error_handler)
		: message_handler_(), error_handler_(error_handler), socket_(*io_context), io_context_(io_context), endpoints_(std::move(endpoints)) {
		strcpy_s(username_, username);
	}

	void save_new_message(message&& message);

	void write_new_message();

	void handle_error(const boost::system::error_code& ec);
};