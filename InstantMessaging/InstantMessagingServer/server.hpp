#pragma once

#include "..\message.hpp"
#include <boost/asio.hpp>
#include <string>
#include <unordered_set>
#include <memory>
#include <deque>

class session;

class room {
public:
	void join(std::shared_ptr<session> user);

	void leave(std::shared_ptr<session> user);

	void deliver(const message& message);

private:
	std::unordered_set<std::shared_ptr<session>> users_; // We have one session per user, which is why we can consider them one and the same
};

class session : public std::enable_shared_from_this<session> {
public:
	session(boost::asio::ip::tcp::socket socket, room& room) : socket_(std::move(socket)), room_(room) { }

	bool username_picked = false;

	void start();

	void ask_for_username();

	void read_username();

	void deliver(const message& message);

	void do_read();

	void do_write();

private:
	boost::asio::ip::tcp::socket socket_;
	room& room_;
	message read_message_;
	std::deque<message> write_messages_;
	std::string username = "anonymous";
};

class server {
public:
	server(boost::asio::io_context& io_context, const boost::asio::ip::tcp::endpoint& endpoint) : io_context_(io_context), acceptor_(io_context, endpoint)
	{
		do_accept();
	}

	void do_accept();

private:
	boost::asio::io_context& io_context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	room room_;
};