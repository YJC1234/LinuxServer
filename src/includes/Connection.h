#pragma once
#include "common.h"
#include<memory>
#include<functional>

class Connection {
public:
	enum State {
		InValid = 0,
		Connecting,
		Connected,
		Closed
	};
	DISALLOW_COPY_AND_MOVE(Connection);
	Connection(int fd, EventLoop* loop);
	~Connection();

	void set_delete_connection_(std::function<void(int)>& fn);
	void set_send_buf(const char* str);
	void set_on_recv(std::function<void(Connection*)> fn);
	void set_on_write(std::function<void(Connection*)> fn);
	State state() const;
	Socket* socket() const;
	Buffer* Read_buf() const;

	void canWrite();
	void Bussiness_read();
	void BUssiness_write();

	RC Read();
	RC Write();
	RC Send(std::string msg);

	void Close();

private:
	RC ReadNonBlocking();
	RC ReadBlocking();
	RC WriteNonBlocking();
	RC WriteBlocking();

private:
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;

	std::unique_ptr<Buffer> read_buf_;
	std::unique_ptr<Buffer> send_buf_;
	State state_;

	std::function<void(int)> delete_connection_;
	std::function<void(Connection*)> on_recv_;
	std::function<void(Connection*)> on_write_;
};