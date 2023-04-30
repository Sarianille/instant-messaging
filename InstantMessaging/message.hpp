#pragma once

class message {
public:
	static constexpr int max_length = 1000;
	char msg[max_length];

	struct header {
		size_t message_length;
		char message_length_digits[5];
		char username[20];

		void set_message_length(size_t message_length) {
			this->message_length = message_length;
			sprintf_s(message_length_digits, "%04d", message_length);
		}

		void set_message_length(char* message_length_digits) {
			this->message_length = atoi(message_length_digits);
			strcpy_s(this->message_length_digits, message_length_digits);
		}
	};
};