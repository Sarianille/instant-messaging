#include "client.hpp"

void client::do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
	boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
		if (!ec) {
			receive(read_message_);
		}
		});
}

void client::receive(const message& message) {
	boost::asio::async_read(socket_, boost::asio::buffer(read_message_.msg), [this](boost::system::error_code ec) {
		if (!ec) {
			std::cout << read_message_.msg << std::endl;
			receive(read_message_);
		}
		});
}

void client::write(const message& message) {
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

void client::close() {
	boost::asio::post(io_context_, [this]() { socket_.close(); });
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

		boost::asio::io_context io_context;
		boost::asio::ip::tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(argv[1], argv[2]);

		client client(io_context, endpoints);
		std::thread thread([&io_context]() { io_context.run(); });

		message message;
		while (std::cin.getline(message.msg, message.max_length)) {
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