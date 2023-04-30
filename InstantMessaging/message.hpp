#pragma once

class message {
public:
	static constexpr int max_length = 1000;
	char msg[max_length];
	char username[20];
};