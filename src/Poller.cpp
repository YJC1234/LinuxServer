#include "Poller.h"
#include"Channel.h"
#include<cassert>
#include<cstring>
#include<unistd.h>
#include<cstdio>

#define MAX_EVENTS 1000

Poller::Poller()
{
	fd_ = epoll_create1(0);
	assert(fd_ != -1);
	events_ = new epoll_event[MAX_EVENTS];
	memset(events_, 0, sizeof(*events_) * MAX_EVENTS);
}

Poller::~Poller()
{
	if (fd_ != -1) {
		close(fd_);
		fd_ = -1;
	}
	delete[] events_;
}

RC Poller::UpdateChannel(Channel* ch) const
{
	epoll_event ev{};
	ev.data.ptr = ch;
	if (ch->listen_events() & Channel::READ_EVENT) {
		ev.events |= EPOLLIN | EPOLLPRI;
	}
	if (ch->listen_events() & Channel::WRITE_EVENT) {
		ev.events |= EPOLLOUT;
	}
	if (ch->listen_events() & Channel::ET) {
		ev.events |= EPOLLET;
	}
	if (!ch->exist()) {
		int r = epoll_ctl(fd_, EPOLL_CTL_ADD, ch->fd(), &ev);
		if (r == -1)
		{
			perror("epoll add error");
			return RC_POLLER_ERROR;
		}
		ch->set_Exist();
	}
	else {
		int r = epoll_ctl(fd_, EPOLL_CTL_MOD, ch->fd(), &ev);
		if (r == -1)
		{
			perror("epoll modify error");
			return RC_POLLER_ERROR;
		}
	}
	return RC_SUCCESS;
}

RC Poller::DeleteChannel(Channel* ch) const
{
	int r = epoll_ctl(fd_, EPOLL_CTL_DEL, ch->fd(), nullptr);
	if (r == -1) {
		perror("epoll delete error");
		return RC_POLLER_ERROR;
	}
	return RC_SUCCESS;
}

std::vector<Channel*> Poller::poll(long timeout) const
{
	std::vector<Channel*> activeChannels;
	int nfds = epoll_wait(fd_, events_, MAX_EVENTS, timeout);
	if (nfds == -1) {
		perror("epoll wait error");
	}
	for (int i = 0; i < nfds; i++) {
		Channel* ch = static_cast<Channel*> (events_[i].data.ptr);
		auto events = events_[i].events;
		if (events & EPOLLIN) {
			ch->set_ready_events(Channel::READ_EVENT);
		}
		if (events & EPOLLOUT) {
			ch->set_ready_events(Channel::WRITE_EVENT);
		}
		if (events & EPOLLET) {
			ch->set_ready_events(Channel::ET);
		}
		activeChannels.push_back(ch);
	}
	return activeChannels;
}