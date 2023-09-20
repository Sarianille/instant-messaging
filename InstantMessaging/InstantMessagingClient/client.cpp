#include "client.hpp"
#include <string.h>

void client::do_connect() {
	boost::asio::async_connect(socket_, endpoints_, [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
		if (!ec) {
			receive_header();
		}
		else {
			close();
		}});
}

void client::receive_header() {
	boost::asio::async_read(socket_, boost::asio::buffer(&read_message_, sizeof(message::header)), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			read_message_.set_host_byte_order();
			receive_body();
		}
		else {
			close();
		}});
}

void client::receive_body() {
	boost::asio::async_read(socket_, boost::asio::buffer(read_message_.msg, read_message_.header.message_length), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			std::cout << "[" << read_message_.header.username << "]: " << read_message_.msg << std::endl;
			receive_header();
		}
		else {
			close();
		}});
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
			else {
				close();
			}});
	}
}

void client::close() {
	boost::asio::post(*io_context_, [this]() { socket_.close(); });
}

bool client::is_open() {
	return socket_.is_open();
}

std::optional<client> client::create_client(char host[], int port, char username[], boost::asio::io_context& io_context) {
	message message;
	strcpy_s(message.header.username, username);

	char port_str[64];
	sprintf_s(port_str, "%d", port);

	boost::asio::ip::tcp::resolver resolver(io_context);
	auto endpoints = resolver.resolve(host, port_str);

	client client(&io_context, std::move(endpoints));

	return client;

	/*std::thread thread([&io_context]() { io_context.run(); });

	while (std::cin.getline(message.msg, message.max_length) && client.is_open()) {
		message.header.message_length = std::strlen(message.msg) + 1;
		client.write(message);
	}
	client.close();
	thread.join();*/
}