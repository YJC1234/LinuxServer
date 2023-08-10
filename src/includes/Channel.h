#pragma once
#include<functional>
#include"common.h"

class Channel {
public:
	DISALLOW_COPY_AND_MOVE(Channel);
	Channel(int fd, EventLoop* loop);
	~Channel();

	void HandleEvent() const;
	void EnableRead();
	void EnableWrite();

private:
	int fd_;
	EventLoop* loop;
	short listen_events_;
	short ready_events_;
	bool exist_;
	std::function<void()> read_callback_;
	std::function<void()> write_callback_;
};