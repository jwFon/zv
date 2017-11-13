#include "util.h"
#include <stdio.h>

namespace util {
uint32_t ipv4ToUint32(const std::string& ipv4)
{
	uint32_t part[4] = {0, 0, 0, 0};
	for (size_t i = 0, j = 0; i < 4; i++) {
		while (ipv4[j] != '.' && j < ipv4.size()) {
			part[i] = part[i] * 10 + ipv4[j] -'0';
			j++;
		}
		j++;
	}
	return part[0] << 24 | part[1] << 16 | part[2] << 8 | part[3];
}

}// namespace util


#ifdef TEST

#include <iostream>
#include <stdio.h>

int main() {
	uint32_t num = util::ipv4ToUint32("127.255.0.128");
	printf("%x\n", num);
	return 0;
}

#endif