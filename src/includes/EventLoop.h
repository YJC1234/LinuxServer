#pragma once
#include"common.h"
#include<memory>

class EventLoop {
public:
	DISALLOW_COPY_AND_MOVE(EventLoop);
	EventLoop();
	~EventLoop();

	void Loop() const;
	RC UpdateChannel(Channel*) const;
	RC DeleteChannel(Channel*) const;
private:
	std::unique_ptr<Poller> poller_;
};