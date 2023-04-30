#pragma once

#include "message.hpp"
#include <boost/asio.hpp>
#include <deque>

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
				
			}
		});
	}

	void write(const message& message) {

	}
	void close() {
		
	}

private:
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context& io_context_;
	message read_message_;
	std::deque<message> write_messages_;


};