#include "pine.h"
#include <iostream>
#include <memory>
#include <string>
#include "Poller.h"
#include "EventLoop.h"

//�����ҳ�����Ҫͬʱ�����������,ʹ�ö���һ���̴߳�������

int main() {
	Socket* sock = new Socket();
	sock->Create();
	sock->Connect("127.0.0.1", 8888);
	sock->SetNonBlocking();
	EventLoop* loop = new EventLoop();
	Connection* conn = new Connection(sock->fd(), loop);	//loop1�������
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