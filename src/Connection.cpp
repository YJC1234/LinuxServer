#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include"Buffer.h"
#include "EventLoop.h"
#include<cassert>
#include<unistd.h>
#include <cstring>

Connection::Connection(int fd, EventLoop* loop)
{
	socket_ = std::make_unique<Socket>();
	socket_->set_fd(fd);
	if (loop != nullptr) {
		channel_ = std::make_unique<Channel>(fd, loop);
		channel_->EnableET();
		channel_->EnableRead();
	}
	read_buf_ = std::make_unique<Buffer>();
	send_buf_ = std::make_unique<Buffer>();
	state_ = State::Connected;
}

Connection::~Connection()
{
}

void Connection::set_delete_connection_(std::function<void(int)>& fn)
{
	delete_connection_ = std::move(fn);
}

void Connection::set_send_buf(const char* str)
{
	send_buf_->set_buf(str);
}

Connection::State Connection::state() const {
	return state_;
}

RC Connection::Read() {
	if (state_ != State::Connected) {
		perror("Connection is not connected, can't read");
		return RC_CONNECTION_ERROR;
	}
	assert(state_ == State::Connected && "connection is disconnected");
	read_buf_->Clear();	//写入前清空缓冲区
	if (socket_->IsNonBlocking()) {
		return ReadNonBlocking();
	}
	else {
		return ReadBlocking();
	}
}

RC Connection::Write() {
	if (state_ != State::Connected) {
		perror("Connection is not connected, can't write");
		return RC_CONNECTION_ERROR;
	}
	assert(state_ == State::Connected && "connection is disconnected");
	RC rc = RC_UNDEFINED;
	if (socket_->IsNonBlocking()) {
		rc = WriteNonBlocking();
	}
	else {
		rc = WriteNonBlocking();
	}
	send_buf_->Clear();	//发送后清空缓冲区
	return rc;
}

RC Connection::Send(std::string msg)
{
	set_send_buf(msg.c_str());
	return Write();
}

void Connection::Close()
{
	delete_connection_(socket_->fd());
}

RC Connection::ReadNonBlocking()
{
	char buf[1024];
	int sockfd = socket_->fd();
	while (true) {	//非阻塞io 循环处理数据
		memset(buf, 0, sizeof(buf));
		auto readBytes = read(sockfd, buf, sizeof(buf));
		if (readBytes > 0) {	//成功接受数据
			read_buf_->Append(buf, readBytes);
		}
		else if (readBytes == -1 && errno == EINTR) {	//程序正常中断，继续读取
			printf("continue Reading");
			continue;
		}
		else if (readBytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {	//数据接受完成
			break;
		}
		else if (readBytes == 0) {	//客户端断开连接
			state_ = State::Closed;
			Close();
			break;
		}
		else {
			printf("Other error on client fd %d\n", sockfd);
			state_ = State::Closed;
			Close();
			break;
		}
	}
	return RC_SUCCESS;
}

RC Connection::ReadBlocking() {
	int sockfd = socket_->fd();
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	ssize_t bytes_read = read(sockfd, buf, sizeof(buf));	//对于阻塞read不需要循环处理数据
	if (bytes_read > 0) {
		read_buf_->Append(buf, bytes_read);
	}
	else if (bytes_read == 0) {
		printf("read EOF, blocking client fd %d disconnected\n", sockfd);
		state_ = State::Closed;
		Close();
	}
	else if (bytes_read == -1) {
		printf("Other error on blocking client fd %d\n", sockfd);
		state_ = State::Closed;
		Close();
	}
	return RC_SUCCESS;
}

RC Connection::WriteNonBlocking()
{
	int sockfd = socket_->fd();
	char buf[send_buf_->size()];
	memcpy(buf, send_buf_->c_str(), send_buf_->size());
	int data_size = send_buf_->size();	//要发送的总大小
	int data_left = data_size;	//未发送的大小
	while (data_left > 0) {
		ssize_t writeBytes = write(sockfd, buf + data_size - data_left, data_left);
		if (writeBytes == 0 && errno == EINTR) {	//正常中断
			printf("continue writing");
			continue;
		}
		else if (writeBytes == 0 && errno == EAGAIN) {
			break;
		}
		else {
			printf("Other error on client fd %d\n", sockfd);
			state_ = State::Closed;
			Close();
			break;
		}

		data_left -= writeBytes;
	}
	return RC_SUCCESS;
}

RC Connection::WriteBlocing()
{	//!没有处理send_buf_大于TCP缓冲区的情况
	int sockfd = socket_->fd();
	char buf[send_buf_->size()];
	memcpy(buf, send_buf_->c_str(), send_buf_->size());
	ssize_t writeBytes = write(sockfd, buf, send_buf_->size());
	if (writeBytes == -1) {
		printf("Other error on client fd %d\n", sockfd);
		state_ = State::Closed;
		Close();
	}
	return RC_SUCCESS;
}