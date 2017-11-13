#include "client.h"
#include <iostream>
#include <unistd.h>
#include "event.h"
#include "zv.h"

Client::Client(int fd, const struct sockaddr_in& client_addr, int addr_len) :
fd(fd), client_addr(client_addr), addr_len(addr_len) {
	reset();
}

#define EXPECT(buf, ch) \
do { \
	if (*buf++ != ch) { \
		return false; \
	} \
} while(0)

#define is_digit(ch) (ch) >= '0' && (ch) <= '9'

inline int toInt(const char* &buf)
{
	int res = 0;
	while (is_digit(*buf)) {
		res = res * 10 + *buf - '0';
		buf++;
	}
	return res;
}

bool Client::processInputBuf(const char* buf, int size)
{
	EXPECT(buf, '*');
	argc = toInt(buf);
	EXPECT(buf, '\r');
	EXPECT(buf, '\n');

	for (int i = 0; i < argc; i++) {
		EXPECT(buf, '$');
		int len = toInt(buf);
		EXPECT(buf, '\r');
		EXPECT(buf, '\n');
		std::string str(buf, len);
		buf += len;
		EXPECT(buf, '\r');
		EXPECT(buf, '\n');
		argv.push_back(str);
	}
	if (argc > 0) {
		for (auto& ch : argv[0]) {
			if (ch >= 'A' && ch <= 'Z') {
				ch = ch - 'A' + 'a';
			}
		}
	}
	return true;
}

void Client::writeErr(std::string str)
{
	std::cerr << str << std::endl;
}

void Client::prepareToSend()
{
	server.eventLoop->addFileEvent(fd, FILE_EVENT_WRITE,
		CallbackEvent::getObj<WhenWrite>());

}