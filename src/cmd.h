#ifndef KV_CMD_H_
#define KV_CMD_H_

#include <string>
#include "client.h"

struct Cmd {
	Cmd(std::string name) : name(name) {}
	std::string name;

	virtual bool check(Client *c) = 0;
	virtual bool process(Client* c) = 0;
};

struct Ping : public Cmd {
	Ping();
	virtual bool check(Client *c);
	virtual bool process(Client* c);
};

struct Set : public Cmd {
	Set();
	virtual bool check(Client *c);
	virtual bool process(Client* c);
};

struct Err : public Cmd {
	Err();
	virtual bool check(Client *c);
	virtual bool process(Client* c);
};

#endif