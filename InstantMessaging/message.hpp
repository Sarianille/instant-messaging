#pragma once

class message {
public:
	message(const char* message) 
	{
		strcpy_s(msg, message);
		current_msg_length = strlen(msg);
	}
	static constexpr int max_length = 1000;
	char msg[max_length];
	size_t current_msg_length = 0;
};