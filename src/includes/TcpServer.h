#pragma once
#include"common.h"
#include <memory>
#include <vector>
#include <functional>

class TcpServer {
public:
	DISALLOW_COPY_AND_MOVE(TcpServer);
	TcpServer();
	~TcpServer();
	void Start();

	RC NewConnection(int fd);
	RC DeleteConnection(int fd);

	void OnConnect(std::function<void(Connection*)> fn);
	void OnRecv(std::function<void(Connection*)> fn);
private:
	std::unique_ptr<EventLoop> main_reactor_;
	std::unique_ptr<Acceptor> acceptor_;

	std::vector<std::unique_ptr<EventLoop>> sub_reactors_;
	std::unordered_map<int, std::unique_ptr<Connection>> connections_;

	std::unique_ptr<ThreadPool> threadPool_;

	std::function<void(Connection*)> on_connect_;
	std::function<void(Connection*)> on_recv_;
};
