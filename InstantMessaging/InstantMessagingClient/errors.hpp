#pragma once

#include <boost/asio.hpp>
#include <optional>

namespace errors {
	enum common_error_types {
		connection_aborted = 1236,
		server_disconnected = 10054,
		connection_timed_out = 10060,
		connection_refused = 10061
	};

	static constexpr char connection_aborted_error_message[] = "Connection aborted";
	static constexpr char server_disconnected_error_message[] = "Server disconnected";
	static constexpr char connection_timed_out_error_message[] = "Connection timed out";
	static constexpr char connection_refused_error_message[] = "Connection refused (is this server running?)";
	static constexpr char host_error_msg[] = "Host name is empty. Please enter host name.";
	static constexpr char username_error_msg[] = "Username is empty. Please enter username.";
	static constexpr char exception_error_msg[] = "Exception occurred. Ensure your data is correct.";

	std::optional<const char*> get_error_message(const boost::system::error_code& ec) {
		switch (ec.value()) {
		case connection_aborted:
			return connection_aborted_error_message;
		case server_disconnected:
			return server_disconnected_error_message;
		case connection_timed_out:
			return connection_timed_out_error_message;
		case connection_refused:
			return connection_refused_error_message;
		default:
			return std::nullopt;
		}
	}
}