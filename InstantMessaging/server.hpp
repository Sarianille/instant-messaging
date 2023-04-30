#pragma once

#include "message.hpp"
#include <boost/asio.hpp>
#include <string>
#include <unordered_set>
#include <memory>
#include <deque>

class room {
public:
	void join(std::shared_ptr<session> user) {
		users_.insert(user);
	}

	void leave(std::shared_ptr<session> user) {
		users_.erase(user);
	}

	void deliver(const message& message) {
		recent_messages_.push_back(message);

		while (recent_messages_.size() > max_recent_messages) {
			recent_messages_.pop_front();
		}

		for (auto user : users_) {
			user->deliver(message);
		}
	}

private:
	std::unordered_set<std::shared_ptr<session>> users_; // We have one session per user, which is why we can consider them one and the same
	std::deque<message> recent_messages_;
	static constexpr int max_recent_messages = 100;
};

class session : std::enable_shared_from_this<session> {
public:
	session(boost::asio::ip::tcp::socket socket, room& room) : socket_(std::move(socket)), room_(room) {
	}

	void start() {
		room_.join(shared_from_this());

		do_read();
	}

	void deliver(const message& message) {
		bool write_in_progress = !write_messages_.empty();
		write_messages_.push_back(message);

		if (!write_in_progress) {
			do_write();
		}
	}

	void do_read() {
		boost::asio::async_read(socket_, boost::asio::buffer(read_message_.msg), [this](boost::system::error_code ec) {
			if (!ec) {
				room_.deliver(read_message_);
				do_read();
			}
			else {
				room_.leave(shared_from_this());
			}
		});
	}

	void do_write() {
		boost::asio::async_write(socket_, boost::asio::buffer(write_messages_.front().msg), [this](boost::system::error_code ec) {
			if (!ec) {
				write_messages_.pop_front();
				if (!write_messages_.empty()) {
					do_write();
				}
			}
			else {
				room_.leave(shared_from_this());
			}
		});
	}

private:
	boost::asio::ip::tcp::socket socket_;
	room& room_;
	message read_message_;
	std::deque<message> write_messages_;
};

class server {
public:
	server(boost::asio::io_context& io_context, const boost::asio::ip::tcp::endpoint& endpoint) : io_context_(io_context), acceptor_(io_context, endpoint) { }

private:
	boost::asio::io_context& io_context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	room room_;
};