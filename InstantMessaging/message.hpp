#pragma once

class message {
public:
	struct {
		unsigned int message_length;
		char username[20];
	} header ; 

	static constexpr int max_length = 1000;
	char msg[max_length];

	void set_network_byte_order() {
		header.message_length = htonl(header.message_length);
	}

	void set_host_byte_order() {
		header.message_length = ntohl(header.message_length);
	}
};