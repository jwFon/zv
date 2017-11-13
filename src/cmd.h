#ifndef KV_CMD_H_
#define KV_CMD_H_

#include <string>
#include "client.h"

struct Cmd {
	Cmd(std::string name, int arity) : name(name), arity(arity) {}
	std::string name;

	int arity;
	
	virtual bool process(Client* c) = 0;
};

struct Ping : public Cmd {
	Ping();
	virtual bool process(Client* c);
};

struct Set : public Cmd {
	Set();
	virtual bool process(Client* c);
};

struct Err : public Cmd {
	Err();
	virtual bool process(Client* c);
};

#endif