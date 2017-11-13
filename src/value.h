#ifndef ZV_VALUE_H
#define ZV_VALUE_H

#include <stdlib.h>

namespace zv {

enum {
	VALUE_TYPE_STRING
};

struct Value {
	unsigned char type;
	void* ptr;
	unsigned int size_;
	Value(unsigned char type) : type(type), ptr(NULL), size_(0) {}
	virtual unsigned int size() {return size_;}
	virtual ~Value(){free(ptr);};
};

struct String : public Value {
	String(const char *);
	String(const String& ref);
	String& operator=(const String& ref);
	const char* c_str();
	~String();
};

} // namespace zv


#endif