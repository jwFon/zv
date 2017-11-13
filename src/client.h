#ifndef KV_CLIENT_H_
#define KV_CLIENT_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <stdarg.h>

struct Server;
struct Client 
{
	explicit Client(int fd, const struct sockaddr_in& client_addr, int addr_len);
	bool processInputBuf(const char* buf, int size);
	inline void reset();
	void writeErr(std::string str);
	void prepareToSend();
	void setErr(const char* format, ...);

	int fd;
	struct sockaddr_in client_addr;
	int addr_len;
	int argc;
	std::vector<std::string> argv;

	std::string output_buf;

};

void Client::reset()
{
	argc = 0;
	argv.clear();
}

#endif