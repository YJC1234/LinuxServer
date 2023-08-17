#include "pine.h"
#include <iostream>
#include <memory>
#include <string>
#include "Poller.h"
#include "EventLoop.h"

//聊天室程序，需要同时处理输入输出,使用额外一个线程处理输入

int main() {
	Socket* sock = new Socket();
	sock->Create();
	sock->Connect("127.0.0.1", 8888);
	sock->SetNonBlocking();
	EventLoop* loop = new EventLoop();
	Connection* conn = new Connection(sock->fd(), loop);	//loop1处理输出
	conn->set_on_recv([](Connection* _conn) {
		std::cout << _conn->Read_buf()->buf() << std::endl;
		});
	auto res = std::async([&conn]() {
		std::string s;
		while (true) {
			std::getline(std::cin, s);
			conn->Send(s);
		}
		});
	loop->Loop();
	res.get();

	delete conn;
	delete loop;
	delete sock;
	return 0;
}