#pragma once

#include "..\message.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <iostream>

class client {
public:
	client(boost::asio::io_context& io_context, const boost::asio::ip::tcp::resolver::results_type& endpoints) : io_context_(io_context), socket_(io_context) {
		do_connect(endpoints);
	}

	void do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);

	void receive_header();

	void receive_body();

	void write(const message& message);

	void close();

private:
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context& io_context_;
	message read_message_;
	std::deque<message> write_messages_;


};