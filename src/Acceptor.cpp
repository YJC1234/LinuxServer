#include"Acceptor.h"
#include"Channel.h"
#include"EventLoop.h"
#include"Socket.h"
#include<cassert>
#include<sys/socket.h>
#include<fcntl.h>

Acceptor::Acceptor(EventLoop* loop)
{
	socket_ = std::make_unique<Socket>();
	assert(socket_->Create() == RC_SUCCESS);
	assert(socket_->Bind("127.0.0.1", 8888) == RC_SUCCESS);
	assert(socket_->Listen() == RC_SUCCESS);

	channel_ = std::make_unique<Channel>(socket_->fd(), loop);
	std::function<void()> cb = std::bind(&Acceptor::AcceptConnection, this);
	channel_->set_read_callback(cb);
	channel_->EnableRead();
}

Acceptor::~Acceptor()
{
}

RC Acceptor::AcceptConnection()
{
	int clnt_fd = -1;
	if (socket_->Accept(clnt_fd) == RC_SOCKET_ERROR) return RC_SOCKET_ERROR;
	//设置新连接为非阻塞模式
	fcntl(clnt_fd, F_SETFL, fcntl(clnt_fd, F_GETFL) | O_NONBLOCK);
	if (new_connection_callback_) {
		new_connection_callback_(clnt_fd);
	}
	return RC_SUCCESS;
}

void Acceptor::set_new_connection_callback(std::function<void(int)> const& callback)
{
	new_connection_callback_ = std::move(callback);
}