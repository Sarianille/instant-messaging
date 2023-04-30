#pragma once

#include <boost/asio.hpp>

class message {
public:
	struct {
		unsigned int message_length;
		char username[20];
	} header ; 

	static constexpr int max_username_length = 19;
	static constexpr int max_length = 1000;
	char msg[max_length];

	void set_network_byte_order() {
		header.message_length = boost::asio::detail::socket_ops::host_to_network_long(header.message_length);
	}

	void set_host_byte_order() {
		header.message_length = boost::asio::detail::socket_ops::network_to_host_long(header.message_length);
	}
};