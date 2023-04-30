#include "server.hpp"
#include <iostream>

void room::join(std::shared_ptr<session> user) {
	users_.insert(user);

	message welcome;
	std::string welcome_msg = "A new user has joined! Welcome!";
	std::copy(welcome_msg.begin(), welcome_msg.end(), welcome.msg);
	deliver(welcome, std::nullopt);
}

void room::leave(std::shared_ptr<session> user) {
	users_.erase(user);

	message goodbye;
	std::string goodbye_msg = "A user has left. Goodbye!";
	std::copy(goodbye_msg.begin(), goodbye_msg.end(), goodbye.msg);
	deliver(goodbye, std::nullopt);
}

void room::deliver(const message& message, std::optional<std::shared_ptr<session>> sender) {
	for (auto user : users_) {
		if (user != sender)
		{
			user->deliver(message);
		}
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
	boost::asio::async_read(socket_, boost::asio::buffer(read_message_.msg), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			room_.deliver(read_message_, shared_from_this());
			do_read();
		}
		else {
			room_.leave(shared_from_this());
		}
		});
}

void session::do_write() {
	boost::asio::async_write(socket_, boost::asio::buffer(write_messages_.front().msg), [this](boost::system::error_code ec, size_t written_bytes) {
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

int main(int argc, char* argv[]) {
	try {
		if (argc < 2)
		{
			std::cout << "Port not specified." << std::endl;
			return 1;
		}
		else if (argc > 2)
		{
			std::cout << "Too many arguments." << std::endl;
			return 1;
		}

		boost::asio::io_context io_context;
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), std::atoi(argv[1]));

		server server(io_context, endpoint);
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}