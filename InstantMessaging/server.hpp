#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <unordered_set>
#include <memory>

class user {

};

class server {

};

class session {

};

class room {
public:
	void join(std::shared_ptr<user> user) {
		users_.insert(user);
	}
	void leave(std::shared_ptr<user> user) {
		users_.erase(user);
	}
private:
	std::unordered_set<std::shared_ptr<user>> users_;
};