#include "client.hpp"
#include "errors.hpp"
#include <string.h>

void client::do_connect() {
	boost::asio::async_connect(socket_, endpoints_, [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
		if (!ec) {
			is_connected_ = true;
			receive_header();
		}
		else {
			handle_error(ec);
		}});
}

void client::receive_header() {
	boost::asio::async_read(socket_, boost::asio::buffer(&read_message_, sizeof(message::header)), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			read_message_.set_host_byte_order();
			receive_body();
		}
		else {
			handle_error(ec);
		}});
}

void client::receive_body() {
	boost::asio::async_read(socket_, boost::asio::buffer(read_message_.msg, read_message_.header.message_length), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			save_new_message(std::move(read_message_));
			receive_header();
		}
		else {
			handle_error(ec);
		}});
}

void client::write(char content[]) {
	message message;
	strcpy_s(message.header.username, username_);
	message.header.message_length = strlen(content) + 1;
	strcpy_s(message.msg, content);

	bool write_in_progress = !write_messages_.empty();
	write_messages_.push_back(message);
	save_new_message(std::move(message));
	if (!write_in_progress) {
		write_new_message();
	}
}

void client::close() {
	boost::asio::post(*io_context_, [this]() {
		if (is_open())
		{
			//socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			socket_.close();
		}
		io_context_->stop();
		});

	/*socket_.close();
	io_context_->stop();*/
}

bool client::is_open() {
	return is_connected_;
}

void client::render_messages() {
	std::lock_guard<std::mutex> lock(read_messages_mutex_);
	int read_messages_new_size = read_messages_.size();

	if (read_messages_new_size > read_messages_old_size_) {
		ImGui::SetScrollHereY(1.0f);
	}

	while (read_messages_new_size > max_messages) {
		read_messages_.pop_front();
		read_messages_new_size--;
	}

	read_messages_old_size_ = read_messages_new_size;
	
	for (auto& message : read_messages_) {
		ImGui::TextWrapped(message.c_str());
	}
}

client client::create_client(char host[], int port, char username[], boost::asio::io_context& io_context) {
	char port_str[64];
	sprintf_s(port_str, "%d", port);

	boost::asio::ip::tcp::resolver resolver(io_context);
	auto endpoints = resolver.resolve(host, port_str);

	client client(&io_context, std::move(endpoints), username);

	return client;
}

std::optional<std::string>& client::get_error_message() {
	return error_message_;
}

void client::clear_error_message() {
	error_message_.reset();
}

void client::save_new_message(message&& message) {
	std::stringstream new_message = std::stringstream();
	new_message << "[" << message.header.username << "]: " << message.msg;

	std::lock_guard<std::mutex> lock(read_messages_mutex_);
	read_messages_.push_back(std::move(new_message.str()));
}

void client::write_new_message() {
	if (!write_messages_.empty()) {
		unsigned int message_length = write_messages_.front().header.message_length;
		write_messages_.front().set_network_byte_order();

		boost::asio::async_write(socket_, boost::asio::buffer(&write_messages_.front(), sizeof(message::header) + message_length), [this](boost::system::error_code ec, size_t written_bytes) {
			if (!ec) {
				write_messages_.pop_front();
				write_new_message();
			}
			else {
				handle_error(ec);
			}});
	}
}

void client::handle_error(const boost::system::error_code& ec) {
	if (ec.value() == errors::connection_aborted) {
		close();
		return;
	}

	std::stringstream error_message;
	auto error_message_optional = errors::get_error_message(ec);

	if (error_message_optional.has_value()) {
		error_message << error_message_optional.value();
	}
	else {
		error_message << "Encountered error " << ec;
	}

	error_message << ". Closing chat." << std::endl;
	error_message_ = error_message.str();

	close();
}