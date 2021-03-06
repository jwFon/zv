#include "cmd.h"
#include "zv.h"
#include "value.h"

#define CHECK(condition) \
do { \
	if (!(condition)) { \
		return false; \
	} \
} while(0)

Ping::Ping()  : Cmd("ping", 0) {}
bool Ping::process(Client* c)
{
	c->output_buf = "+pong\r\n";
	return true;
}

Set::Set() : Cmd("set", 3) {}
bool Set::process(Client *c)
{
	std::string& key = c->argv[1], &value = c->argv[2];
	zv::Value*& value_obj = server.dict[key];
	if (value_obj == NULL) {
		std::cout << "a" << std::endl;
		value_obj = new zv::String(value.c_str());
	} else {
		std::cout << "b" << std::endl;
		zv::String& str = dynamic_cast<zv::String&>(*value_obj);
		str = value.c_str();
	}
	c->output_buf = "+OK\r\n";

	return true;
}

Err::Err() : Cmd("err", 0) {}
bool Err::process(Client *c)
{
	c->setErr("+(error) ERR unknown command '%s'\r\n", c->argv[0].c_str());
	
	return true;
}