#include "zv.h"

#include <iostream>
#include <string>
#include <cstdio>


FileEvent::FileEvent(int fd) :
	fd(fd), mask(0), whenRead(NULL), whenWrite(NULL){}


bool WhenAccept::operator()(FileEvent* fe)
{
	struct sockaddr_in client_addr;
	int client_fd, addr_len;
	client_fd = accept(server.fd, (struct sockaddr*)&client_addr, (socklen_t*) &addr_len);

	Client *client = new Client(client_fd,  client_addr, addr_len);
	
	server.clients[client_fd] = client;

	FileEvent *readEvent = new FileEvent(client->fd);
	readEvent->mask |= FILE_EVENT_READ;
	readEvent->whenRead = CallbackEvent::getObj<WhenRead>();

	server.eventLoop->addFileEvent(readEvent);
	return true;
}

bool WhenRead::operator()(FileEvent* fe)
{
	char buf[READ_BUF];
	int readn = read(fe->fd, buf, READ_BUF);
	
	if (readn < 0) {
		std::cerr << "read error" << std::endl;
	} else if (readn == 0) {
		std::cout << "get 0" << std::endl;
		server.eventLoop->delFileEvent(fe->fd);
		server.del_client(fe->fd);
	} else {
		std::cout << "in read" << std::endl;
		Client* client = server.clients[fe->fd];
		if (client == NULL) {
			return false;
		}
		if (!client->processInputBuf(buf, readn)) {
			client->writeErr("parse protocol error");
			return false;
		}
		std::cout << "processInputBuf done" << std::endl;
		if (!server.process(client)) {
			client->writeErr("process error");
		}
		client->reset();
		client->prepareToSend();
		std::cout << "preapre to send done" << std::endl;
	}
	return true;
}

bool WhenWrite::operator()(FileEvent* fe)
{
	std::cout << "in write" << std::endl;
	Client* client = server.clients[fe->fd];
	int writen = write(fe->fd, client->output_buf.c_str(),client->output_buf.size());
	if (writen < 0) {
		std::cerr << "write err" << std::endl;
		return false;
	}
	server.eventLoop->delFileEvent(fe->fd, FILE_EVENT_WRITE);
	return true;
}

EventLoop::EventLoop() : 
	fileEvents(EVENTLOOP_MAX_EVENT)
{
	fd = epoll_create1(0);
	events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EVENTLOOP_MAX_EVENT);
	event_max = EVENTLOOP_MAX_EVENT;
	for (size_t i = 0; i < fileEvents.size(); i++) {
		fileEvents[i] = NULL;
	}
}
EventLoop::~EventLoop()
{
	struct epoll_event ev;
	for (size_t i = 0; i < fileEvents.size(); i++) {
		if (fileEvents[i]) {
			epoll_ctl(fd, EPOLL_CTL_DEL, fileEvents[i]->fd, &ev);
			delete fileEvents[i];
		}
	}
	close(fd);
}

bool EventLoop::addFileEvent(FileEvent* fe)
{
	struct epoll_event ev;
	ev.data.u64 = 0;
	ev.events = 0;
	ev.data.ptr = fe;
	if (fe->mask & FILE_EVENT_READ) {
		ev.events |= EPOLLIN;
	}
	if (fe->mask & FILE_EVENT_WRITE) {
		ev.events |= EPOLLOUT;
	}
	epoll_ctl(fd, EPOLL_CTL_ADD, fe->fd, &ev);

	if ((size_t)fe->fd < fileEvents.size()) {
		fileEvents[fe->fd] = fe;
	} else {
		fileEvents.push_back(fe);
	}

	return true;
}

bool EventLoop::addFileEvent(int fe_fd, uint32_t mask, CallbackEvent* callback)
{
	FileEvent *fe = fileEvents[fe_fd];
	if (fe == NULL) {
		fe = new FileEvent(fe_fd);
		fe->mask = mask;
		if (mask & FILE_EVENT_READ) {
			fe->whenRead = callback;
		}
		if (mask & FILE_EVENT_WRITE) {
			fe->whenWrite = callback;
		}
		addFileEvent(fe);
	} else {
		fe->mask |= mask;
		if (mask & FILE_EVENT_READ) {
			fe->whenRead = callback;
		}
		if (mask & FILE_EVENT_WRITE) {
			fe->whenWrite = callback;
		}
		
		struct epoll_event ev;
		ev.data.u64 = 0;
		ev.events = 0;
		ev.data.ptr = fe;
		if (fe->mask & FILE_EVENT_READ) {
			ev.events |= EPOLLIN;
		}
		if (fe->mask & FILE_EVENT_WRITE) {
			ev.events |= EPOLLOUT;
		}
		int res = epoll_ctl(fd, EPOLL_CTL_MOD, fe->fd, &ev);
		perror("epoll_ctl");
		std::cout << "res  " << res << std::endl;
	}
	return true;
}

bool EventLoop::delFileEvent(int fe_fd, uint32_t del_mask)
{
	struct epoll_event ev;
	ev.data.u64 = 0;
	ev.events = 0;

	FileEvent *fe = fileEvents[fe_fd];
	if (fe == NULL) {
		return true;
	}
	ev.data.ptr = fe;

	int op;
	fe->mask = fe->mask & (~del_mask);
	if (fe->mask == 0) {
		op = EPOLL_CTL_DEL;
	} else {
		op = EPOLL_CTL_MOD;
		if (fe->mask & FILE_EVENT_READ) {
			ev.events |= EPOLLIN;
		}
		if (fe->mask & FILE_EVENT_WRITE) {
			ev.events |= EPOLLOUT;
		}
	}
	epoll_ctl(fd, op, fe->fd, &ev);
	if (fe->mask == 0) {
		fileEvents[fe->fd] = NULL;
		delete fe;
	}

	return true;
}

bool EventLoop::delFileEvent(int fe_fd)
{
	struct epoll_event ev;
	ev.data.u64 = 0;
	ev.events = 0;

	FileEvent *fe = fileEvents[fe_fd];
	if (fe == NULL) {
		return true;
	}

	epoll_ctl(fd, EPOLL_CTL_DEL, fe->fd, &ev);
	fileEvents[fd] = NULL;
	delete fe;

	return true;
}

void EventLoop::loop()
{
	while (true) {
		std::cout << "in lloop" << std::endl;
		int read_cnt = epoll_wait(fd, events, event_max, -1);
		for (int i = 0; i < read_cnt; i++) {
			std::cout << read_cnt  << std::endl;
			struct epoll_event& epoll_ev = events[i];
			auto* fe = static_cast<FileEvent*>(events[i].data.ptr);
			std::cout << fe->fd << std::endl;
			if (epoll_ev.events & EPOLLIN) {
				(*fe->whenRead)(fe);
			}
			if (epoll_ev.events & EPOLLOUT) {
				(*fe->whenWrite)(fe);
			}
		}
	}
	
}
