#include "client.hpp"
#include "errors.hpp"
#include <string.h>

void message_handler::render_messages() {
	std::lock_guard<std::mutex> lock(read_messages_mutex_);
	int read_messages_new_size = read_messages_.size();

	if (read_messages_new_size > read_messages_old_size_) {
		ImGui::SetScrollHereY(-1.0f);
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

void message_handler::save_incoming_message(const message& message) {
	std::stringstream new_message = std::stringstream();
	new_message << "[" << message.header.username << "]: " << message.msg;

	std::lock_guard<std::mutex> lock(read_messages_mutex_);
	read_messages_.push_back(std::move(new_message.str()));
}

void message_handler::save_incoming_message(message&& message) {
	std::stringstream new_message = std::stringstream();
	new_message << "[" << message.header.username << "]: " << message.msg;

	std::lock_guard<std::mutex> lock(read_messages_mutex_);
	read_messages_.push_back(std::move(new_message.str()));
}

bool message_handler::write_queue_empty() {
	return write_messages_.empty();
}

void message_handler::enqueue_message_to_be_written(const message& message) {
	write_messages_.push_back(message);
}

void message_handler::enqueue_message_to_be_written(message&& message) {
	write_messages_.push_back(std::move(message));
}

message& message_handler::message_to_be_written() {
	return write_messages_.front();
}

void message_handler::pop_message_to_be_written() {
	write_messages_.pop_front();
}

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
	boost::asio::async_read(socket_, boost::asio::buffer(&message_handler_.read_message_, sizeof(message::header)), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			message_handler_.read_message_.set_host_byte_order();
			receive_body();
		}
		else {
			handle_error(ec);
		}});
}

void client::receive_body() {
	boost::asio::async_read(socket_, boost::asio::buffer(message_handler_.read_message_.msg, message_handler_.read_message_.header.message_length), [this](boost::system::error_code ec, size_t read_bytes) {
		if (!ec) {
			save_new_message(std::move(message_handler_.read_message_));
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

	bool write_in_progress = !message_handler_.write_queue_empty();
	message_handler_.enqueue_message_to_be_written(message);
	save_new_message(std::move(message));

	if (!write_in_progress) {
		write_new_message();
	}
}

void client::close() {
	boost::asio::post(*io_context_, [this]() {
		if (is_open())
		{
			socket_.close();
		}
		io_context_->stop();
		});
}

bool client::is_open() {
	return is_connected_;
}

void client::clear_error_message() {
	error_message_.reset();
}

void client::render_messages() {
	message_handler_.render_messages();
}

client client::create_client(const char host[], int port, const char username[], boost::asio::io_context& io_context) {
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

void client::save_new_message(message&& message) {
	message_handler_.save_incoming_message(std::move(message));
}

void client::write_new_message() {
	if (!message_handler_.write_queue_empty()) {
		unsigned int message_length = message_handler_.message_to_be_written().header.message_length;
		message_handler_.message_to_be_written().set_network_byte_order();

		boost::asio::async_write(socket_, boost::asio::buffer(&message_handler_.message_to_be_written(), sizeof(message::header) + message_length), [this](boost::system::error_code ec, size_t written_bytes) {
			if (!ec) {
				message_handler_.pop_message_to_be_written();
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