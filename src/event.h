#ifndef KV_EVENTLOOP_H_
#define KV_EVENTLOOP_H_

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <vector>
#include <cstdint>
#include <unistd.h>


#define EVENTLOOP_MAX_EVENT 1024 * 16
#define READ_BUF 1024
#define WRITE_BUF 1024

enum FILE_EVENTS{
	FILE_EVENT_READ = 1,
	FILE_EVENT_WRITE = 2
};

struct CallbackEvent;

struct FileEvent {
	FileEvent(int fd);

	int fd;
	uint32_t mask;
	CallbackEvent* whenRead;
	CallbackEvent* whenWrite;
};

struct CallbackEvent {
	virtual bool operator()(FileEvent*) = 0;
	template<typename T>
	static CallbackEvent* getObj();
};


struct WhenAccept : public CallbackEvent {
	virtual bool operator()(FileEvent*);
};
struct WhenRead: public CallbackEvent {
	virtual bool operator()(FileEvent*);
};
struct WhenWrite : public CallbackEvent {
	virtual bool operator()(FileEvent*);
};


struct EventLoop {
	EventLoop();
	bool addFileEvent(FileEvent* fe);
	bool addFileEvent(int fd, uint32_t mask, CallbackEvent* callback);
	void loop();
	~EventLoop();
	bool delFileEvent(int fd, uint32_t del_mask);
	bool delFileEvent(int fd);
	int fd;
	std::vector<FileEvent*> fileEvents;
	struct epoll_event* events;
	int event_max;
};


template<typename T>
CallbackEvent* CallbackEvent::getObj()
{
	static T* obj = new T;
	return obj;
}

#endif