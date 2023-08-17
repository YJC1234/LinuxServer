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

void Connection::set_on_recv(std::function<void(Connection*)> fn)
{
	on_recv_ = std::move(fn);
	std::function<void()> cb = std::bind(&Connection::Bussiness_read, this);
	channel_->set_read_callback(cb);
}

void Connection::set_on_write(std::function<void(Connection*)> fn)
{
	on_write_ = std::move(fn);
	std::function<void()> cb = std::bind(&Connection::BUssiness_write, this);
	channel_->set_write_callback(cb);
}

Connection::State Connection::state() const {
	return state_;
}

Socket* Connection::socket() const
{
	return socket_.get();
}

Buffer* Connection::Read_buf() const
{
	return read_buf_.get();
}

void Connection::canWrite()
{
	channel_->EnableWrite();
}

//Ԥ���� ��tcp����������read_buf_�У�����û���Ҫ�Լ�����
void Connection::Bussiness_read()
{
	Read();
	if (state_ == State::Closed) return;	//����Read֮���Ѿ��ر�
	on_recv_(this);
}
//Ԥ���� ��send_bufд��tcp��������
void Connection::BUssiness_write()
{
	on_write_(this);
	if (state_ == State::Closed) return;
	Write();
}

RC Connection::Read() {
	if (state_ != State::Connected) {
		perror("Connection is not connected, can't read");
		return RC_CONNECTION_ERROR;
	}
	assert(state_ == State::Connected && "connection is disconnected");
	read_buf_->Clear();	//д��ǰ��ջ�����
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
		rc = WriteBlocking();
	}
	send_buf_->Clear();	//���ͺ���ջ�����
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
	while (true) {	//������io ѭ����������
		memset(buf, 0, sizeof(buf));
		auto readBytes = read(sockfd, buf, sizeof(buf));
		if (readBytes > 0) {	//�ɹ���������
			read_buf_->Append(buf, readBytes);
		}
		else if (readBytes == -1 && errno == EINTR) {	//���������жϣ�������ȡ
			printf("continue Reading");
			continue;
		}
		else if (readBytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {	//���ݽ������
			break;
		}
		else if (readBytes == 0) {	//�ͻ��˶Ͽ�����
			printf("read EOF, client fd %d disconnected\n", sockfd);
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
	ssize_t bytes_read = read(sockfd, buf, sizeof(buf));	//��������read����Ҫѭ����������
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
	int data_size = send_buf_->size();	//Ҫ���͵��ܴ�С
	int data_left = data_size;	//δ���͵Ĵ�С
	while (data_left > 0) {
		ssize_t writeBytes = write(sockfd, buf + data_size - data_left, data_left);
		if (writeBytes == -1 && errno == EINTR) {	//�����ж�
			printf("continue writing");
			continue;
		}
		else if (writeBytes == -1 && errno == EAGAIN) {
			break;
		}
		else if (writeBytes == -1) {
			printf("Other error on client fd %d\n", sockfd);
			state_ = State::Closed;
			Close();
			break;
		}

		data_left -= writeBytes;
	}
	return RC_SUCCESS;
}

RC Connection::WriteBlocking()
{	//!û�д���send_buf_����TCP�����������
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