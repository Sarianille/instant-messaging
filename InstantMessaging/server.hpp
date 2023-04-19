#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <string>
#include <unordered_set>

class user {

};

class server {

};

class session {

};

class room {
public:
	void join(boost::shared_ptr<user> user) {
		users_.insert(user);
	}
	void leave(boost::shared_ptr<user> user) {
		users_.erase(user);
	}
private:
	std::unordered_set<boost::shared_ptr<user>> users_;
};