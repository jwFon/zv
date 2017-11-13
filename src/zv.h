#ifndef KV_ZV_H_
#define KV_ZV_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <string>
#include <unistd.h>

#include "client.h"
#include "event.h"
#include "util.h"
#include "cmd.h"
#include "dict.h"
#include "value.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3003
#define MAX_ESTABLISHED_COUNT 1000

struct Server {
	Server(std::string ipv4, int port);
	bool process(Client* client);
	~Server();

	int fd;
	struct sockaddr_in serv_addr;
	EventLoop *eventLoop;
	std::unordered_map<int, Client*> clients;
	std::unordered_map<std::string, Cmd*> cmds;
	inline void del_client(int fd);
	Dict<std::string, zv::Value*> dict;
};

void Server::del_client(int fd)
{
	delete clients[fd];
	clients[fd] = NULL;
	close(fd);
}

extern Server server;


#endif