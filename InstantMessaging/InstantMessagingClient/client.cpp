#include "client.hpp"
#include <string.h>

void client::do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
	boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
		if (!ec) {
			receive_header();
		}
		});
}

void client::receive_header() {
	boost::asio::async_read(socket_, boost::asio::buffer(&read_message_, sizeof(message::header)), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			read_message_.set_host_byte_order();
			receive_body();
		}
		});
}

void client::receive_body() {
	boost::asio::async_read(socket_, boost::asio::buffer(read_message_.msg, read_message_.header.message_length), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			std::cout << "[" << read_message_.header.username << "]: " << read_message_.msg << std::endl;
			receive_header();
		}
		});
}

void client::write(const message& message) {
	bool write_in_progress = !write_messages_.empty();
	write_messages_.push_back(message);

	if (!write_in_progress) {
		unsigned int message_length = write_messages_.front().header.message_length;
		write_messages_.front().set_network_byte_order();

		boost::asio::async_write(socket_, boost::asio::buffer(&write_messages_.front(), sizeof(message::header) + message_length), [this](boost::system::error_code ec, size_t written_bytes) {
			if (!ec) {
				write_messages_.pop_front();
				if (!write_messages_.empty()) {
					write(write_messages_.front());
				}
			}
			});
	}
}

void client::close() {
	boost::asio::post(io_context_, [this]() { socket_.close(); });
}

std::string set_username() {
	std::cout << "Enter username: ";

	char username[message::max_username_length];
	std::cin.getline(username, message::max_username_length);

	return username;
}

int main(int argc, char* argv[]) {
	try {
		if (argc < 3)
		{
			std::cout << "Host or port not specified." << std::endl;
			return 1;
		}
		else if (argc > 3)
		{
			std::cout << "Too many arguments." << std::endl;
			return 1;
		}

		message message;
		strcpy_s(message.header.username, set_username().c_str());

		boost::asio::io_context io_context;
		boost::asio::ip::tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(argv[1], argv[2]);

		client client(io_context, endpoints);
		std::thread thread([&io_context]() { io_context.run(); });

		while (std::cin.getline(message.msg, message.max_length)) {
			message.header.message_length = std::strlen(message.msg) + 1;
			client.write(message);
		}
		client.close();
		thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}