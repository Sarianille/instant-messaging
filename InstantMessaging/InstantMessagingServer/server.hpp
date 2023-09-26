#pragma once

#include "..\message.hpp"
#include <boost/asio.hpp>
#include <string>
#include <unordered_set>
#include <memory>
#include <deque>
#include <optional>
#include <string>

class session;

class room {
public:
	/// <summary>
	/// Adds user to the room.
	/// </summary>
	/// <param name="user">User to be added.</param>
	void join(std::shared_ptr<session> user);

	/// <summary>
	/// Removes user from the room.
	/// </summary>
	/// <param name="user">User to be removed.</param>
	void leave(std::shared_ptr<session> user);

	/// <summary>
	/// Sends message to all other users in the room.
	/// </summary>
	/// <param name="message">Message to be sent.</param>
	/// <param name="sender">User who sent the message.</param>
	void deliver(const message& message, std::optional<std::shared_ptr<session>> sender);

	/// <summary>
	/// Broadcasts an announcement to all users in the room.
	/// </summary>
	/// <param name="announcement">Announcement to be sent.</param>
	void send_announcement(const std::string& announcement);

	/// <summary>
	/// Logs an event to the console.
	/// </summary>
	/// <param name="event_message">Event to be logged.</param>
	void log_event(const std::string& event_message);

private:
	std::unordered_set<std::shared_ptr<session>> users_; // We have one session per user, which is why we can consider them one and the same
	std::string name_ = "SERVER";
};

class session : public std::enable_shared_from_this<session> {
public:
	session(boost::asio::ip::tcp::socket socket, room& room) : socket_(std::move(socket)), room_(room), read_message_() { }

	/// <summary>
	/// Joins the user to the room and starts reading remote messages.
	/// </summary>
	void start();

	/// <summary>
	/// Enqueues a message to be sent to the user. If there are no messages being sent, it will start sending the message immediately.
	/// </summary>
	/// <param name="message">Message to be sent.</param>
	void deliver(const message& message);

	/// <summary>
	/// Reads the header of the message.
	/// </summary>
	void do_read_header();

	/// <summary>
	/// Reads the body of the message.
	/// </summary>
	void do_read_body();

	/// <summary>
	/// Sends the next message in the queue asynchronously.
	/// </summary>
	void do_write();

private:
	boost::asio::ip::tcp::socket socket_;
	room& room_;
	message read_message_;
	std::deque<message> write_messages_;
};

class server {
public:
	server(boost::asio::io_context& io_context, const boost::asio::ip::tcp::endpoint& endpoint) : io_context_(io_context), acceptor_(io_context, endpoint)
	{
		do_accept();
	}

	/// <summary>
	/// Asynchronously listens for new connections and accepts them.
	/// </summary>
	void do_accept();

private:
	boost::asio::io_context& io_context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	room room_;
};