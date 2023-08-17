#include "TcpServer.h"
#include"EventLoop.h"
#include "Acceptor.h"
#include "ThreadPool.h"
#include "Connection.h"
#include "Socket.h"
#include<thread>
#include<functional>
#include<cassert>

TcpServer::TcpServer()
{
	main_reactor_ = std::make_unique<EventLoop>();
	acceptor_ = std::make_unique<Acceptor>(main_reactor_.get());
	std::function<void(int)> cb = std::bind(&TcpServer::NewConnection, this, std::placeholders::_1);
	acceptor_->set_new_connection_callback(cb);
	unsigned int size = std::thread::hardware_concurrency();
	threadPool_ = std::make_unique<ThreadPool>(size);
	for (ssize_t i = 0; i < size; i++) {
		auto sub_reactor = std::make_unique<EventLoop>();
		sub_reactors_.push_back(std::move(sub_reactor));
	}
}

TcpServer::~TcpServer()
{
}

void TcpServer::Start()
{
	for (ssize_t i = 0; i < sub_reactors_.size(); i++) {
		std::function<void()> sub_loop = std::bind(&EventLoop::Loop, sub_reactors_[i].get());
		threadPool_->Add(std::move(sub_loop));
	}
	main_reactor_->Loop();
}

RC TcpServer::NewConnection(int fd)
{
	assert(fd != -1);
	uint64_t random = fd % sub_reactors_.size();	//随机分配给一个sub_reactor
	std::unique_ptr<Connection> conn = std::make_unique<Connection>(fd, sub_reactors_[random].get());
	std::function<void(int)> cb = std::bind(&TcpServer::DeleteConnection, this, std::placeholders::_1);
	conn->set_delete_connection_(cb);
	conn->set_on_recv(on_recv_);
	connections_[fd] = std::move(conn);
	if (on_connect_) {
		on_connect_(connections_[fd].get());
	}
	return RC_SUCCESS;
}

RC TcpServer::DeleteConnection(int fd)
{
	auto it = connections_.find(fd);
	assert(it != connections_.end());
	connections_.erase(fd);
	return RC_SUCCESS;
}

void TcpServer::OnConnect(std::function<void(Connection*)> fn)
{
	on_connect_ = std::move(fn);
}

void TcpServer::OnRecv(std::function<void(Connection*)> fn)
{
	on_recv_ = std::move(fn);
}