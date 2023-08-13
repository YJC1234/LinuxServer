#include "EventLoop.h"
#include"Poller.h"
#include"Channel.h"
#include<vector>

EventLoop::EventLoop()
{
	poller_ = std::make_unique<Poller>();
}

EventLoop::~EventLoop()
{
}

void EventLoop::Loop() const
{
	while (true) {
		auto acitveChannels = poller_->poll();
		for (Channel* ch : acitveChannels) {
			ch->HandleEvent();
		}
	}
}

RC EventLoop::UpdateChannel(Channel* ch) const
{
	return poller_->UpdateChannel(ch);
}

RC EventLoop::DeleteChannel(Channel* ch) const
{
	return poller_->DeleteChannel(ch);
}