#pragma once

#include <boost/asio.hpp>
#include <optional>
#include <string>
#include <functional>
#include <mutex>

namespace errors {
	enum common_error_types {
		connection_aborted = 1236,
		server_disconnected = 10054,
		connection_timed_out = 10060,
		connection_refused = 10061
	};

	static constexpr char login_error_title[] = "Log in error";
	static constexpr char chatroom_error_title[] = "Error";

	static constexpr char connection_aborted_error_message[] = "Connection aborted";
	static constexpr char server_disconnected_error_message[] = "Server disconnected";
	static constexpr char connection_timed_out_error_message[] = "Connection timed out";
	static constexpr char connection_refused_error_message[] = "Connection refused (is this server running?)";
	static constexpr char host_error_msg[] = "Host name is empty. Please enter host name.";
	static constexpr char username_error_msg[] = "Username is empty. Please enter username.";
	static constexpr char exception_error_msg[] = "Exception occurred. Ensure your data is correct.";

	std::optional<const char*> get_error_message(const boost::system::error_code& ec);

	class error_handler {
	public:
		std::optional<std::string> error_message_ = std::nullopt;

		void set_error_message(std::string&& error_message);

		void potentially_display_error_message(const char* error_title, const std::function<void()>& on_close);

	private:
		std::mutex error_message_mutex;

		void clear_error_message();
	};
}