#include "server.hpp"

void room::join(std::shared_ptr<session> user) {
	users_.insert(user);
}

void room::leave(std::shared_ptr<session> user) {
	users_.erase(user);
}

void room::deliver(const message& message) {
	recent_messages_.push_back(message);

	while (recent_messages_.size() > max_recent_messages) {
		recent_messages_.pop_front();
	}

	for (auto user : users_) {
		user->deliver(message);
	}
}

void session::start() {
	room_.join(shared_from_this());

	do_read();
}

void session::deliver(const message& message) {
	bool write_in_progress = !write_messages_.empty();
	write_messages_.push_back(message);

	if (!write_in_progress) {
		do_write();
	}
}

void session::do_read() {
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

void session::do_write() {
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

void server::do_accept() {
	acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
		if (!ec) {
			std::make_shared<session>(std::move(socket), room_)->start();
		}
		do_accept();
		});
}
