#include "zv.h"


Server::Server(std::string ipv4, int port) 
{
	fd = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(util::ipv4ToUint32(ipv4));
	serv_addr.sin_port = htons(port);

	int on = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	bind(fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in));

	// max of establishd but not accept connection
	if (listen(fd, MAX_ESTABLISHED_COUNT) == -1) {
		std::cerr << "server initialization failded" << std::endl;
	} else {
		std::cout << "server initialization success" << std::endl;
	}

	eventLoop = new EventLoop();
	eventLoop->addFileEvent(fd, FILE_EVENT_READ,
		CallbackEvent::getObj<WhenAccept>());

	cmds["err"] = new Err();
	cmds["ping"] = new Ping();
	cmds["set"] = new Set();
}



bool Server::process(Client* client)
{
	if (client == NULL) {
		return false;
	}

	auto iter = cmds.find(client->argv[0]);
	if (iter == cmds.end()) {
		iter = cmds.find("err");
	}
	Cmd* real_cmd = iter->second;

	if (real_cmd->arity  > client->argc) {
		client->setErr("+(error) ERR wrong number of arguments for '%s' command\r\n", client->argv[0].c_str());
		return false;
	}
	
	return real_cmd->process(client);
}

Server::~Server()
{
	delete eventLoop;
	for (auto iter = clients.begin(); iter != clients.end(); iter++) {
		if (iter->second) {
			del_client(iter->first);
		}
	}
	close(fd);
}

Server server(SERVER_IP, SERVER_PORT);

int main(int argc, char* argv[]) 
{
	server.eventLoop->loop();

	return 0;
}