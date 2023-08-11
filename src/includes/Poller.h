#pragma once
#include<common.h>
#include<sys/epoll.h>
#include<vector>
#include<memory>

class Poller {
public:
	DISALLOW_COPY_AND_MOVE(Poller);
	Poller();
	~Poller();

	RC UpdateChannel(Channel*) const;
	RC DeleteChannel(Channel*) const;

	std::vector<Channel*> poll(long timeout = -1) const;
private:
	int fd_;
	epoll_event* events_{ nullptr };
};