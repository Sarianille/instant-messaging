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
	std::unordered_set<std::shared_ptr<session>> users_; // We have one user per session, which is why we can consider them one and the same
	std::deque<message> recent_messages_;
	static constexpr int max_recent_messages = 100;
};

class session : std::enable_shared_from_this<session> {
public:
	session(boost::asio::ip::tcp::socket socket, room& room) : socket_(std::move(socket)), room_(room) {
	}

	void start() {
		room_.join(shared_from_this());
	}

	void deliver(const message& message) {
		bool write_in_progress = !write_messages_.empty();
		write_messages_.push_back(message);

		if (!write_in_progress) {
		}
	}

private:
	boost::asio::ip::tcp::socket socket_;
	room& room_;
	message read_message_;
	std::deque<message> write_messages_;
};

class server {

};