#pragma once

#include "message.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <iostream>

class client {
public:
	client(boost::asio::io_context& io_context, const boost::asio::ip::tcp::resolver::results_type& endpoints) : io_context_(io_context), socket_(io_context) {
		do_connect(endpoints);
	}

	void do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
		boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
			if (!ec) {
				receive(read_message_);
			}
		});
	}

	void receive(const message& message) {
		boost::asio::async_read(socket_, boost::asio::buffer(read_message_.msg), [this](boost::system::error_code ec) {
			if (!ec) {
				std::cout << read_message_.msg << std::endl;
				receive(read_message_);
			}
		});
	}

	void write(const message& message) {
		bool write_in_progress = !write_messages_.empty();
		write_messages_.push_back(message);

		if (!write_in_progress) {
			boost::asio::async_write(socket_, boost::asio::buffer(write_messages_.front().msg), [this](boost::system::error_code ec) {
				if (!ec) {
					write_messages_.pop_front();
					if (!write_messages_.empty()) {
						write(write_messages_.front());
					}
				}
				});
		}
	}

	void close() {
		boost::asio::post(io_context_, [this]() { socket_.close(); });
	}

private:
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context& io_context_;
	message read_message_;
	std::deque<message> write_messages_;


};