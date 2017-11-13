#ifndef KV_UTIL_H
#define KV_UTIL_H

#include <cstdint>
#include <string>
#include <stdio.h>

namespace util {
uint32_t ipv4ToUint32(const std::string& ipv4);
}// namespace util

#endif