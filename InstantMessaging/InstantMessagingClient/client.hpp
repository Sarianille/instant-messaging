#pragma once

#include "..\message.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <iostream>
#include <string>
#include <optional>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class client {
public:
	void do_connect();

	void receive_header();

	void receive_body();

	void write(const message& message);

	void close();

	bool is_open();

	static std::optional<client> create_client(char host[], int port, char username[], boost::asio::io_context& io_context);

	client(client&& other) noexcept : socket_(std::move(other.socket_)), io_context_(other.io_context_), endpoints_(std::move(other.endpoints_)), read_message_(std::move(other.read_message_)), write_messages_(std::move(other.write_messages_)) {}

	client& operator=(client&& other) noexcept {
		socket_ = std::move(other.socket_);
		io_context_ = other.io_context_;
		endpoints_ = std::move(other.endpoints_);
		read_message_ = std::move(other.read_message_);
		write_messages_ = std::move(other.write_messages_);
		return *this;
	}

private:
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context* io_context_;
	boost::asio::ip::tcp::resolver::results_type endpoints_;
	message read_message_;
	std::deque<message> write_messages_;

	client(boost::asio::io_context* io_context, boost::asio::ip::tcp::resolver::results_type&& endpoints) : io_context_(io_context), socket_(*io_context), endpoints_(std::move(endpoints)) {}
};