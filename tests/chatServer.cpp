#include"pine.h"
#include<iostream>
#include <string>
#include <map>

int main() {
	TcpServer* server = new TcpServer();
	std::map<int, Connection*> clients;

	server->OnConnect([&clients](Connection* conn) {
		std::cout << "New connection fd : " << conn->socket()->fd() << std::endl;
		conn->Send("welcome!");
		clients[conn->socket()->fd()] = conn;
		});

	server->OnRecv([&clients](Connection* conn) {
		std::string s = "fd: " + std::to_string(conn->socket()->fd()) + " says:" + conn->Read_buf()->buf();
		for (auto& each : clients) {
			Connection* client = each.second;
			client->Send(s);
		}
		});

	server->Start();

	delete server;
	return 0;
}