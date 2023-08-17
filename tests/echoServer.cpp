#include "pine.h"
#include <iostream>

int main() {
	TcpServer* server = new TcpServer();
	server->OnConnect([](Connection* conn) {
		std::cout << "New connection fd : " << conn->socket()->fd() << std::endl;
		});
	server->OnRecv([](Connection* conn) {
		std::cout << "Message from client : " << conn->Read_buf()->c_str() << std::endl;
		conn->Send(conn->Read_buf()->c_str());
		});
	server->Start();

	delete server;
	return 0;
}