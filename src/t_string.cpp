#include "value.h"
#include <stdlib.h>
#include <iostream>

namespace zv {

inline static void assign(void* ptr, const void* right, unsigned int len)
{
	for (unsigned int i = 0; i < len; i++) {
		static_cast<char*>(ptr)[i] = static_cast<char*>(const_cast<void*>(right))[i];
	}
}

String::String(const char *str = "") : Value(VALUE_TYPE_STRING)
{
	char* tmp = const_cast<char*>(str);
	while (*tmp++) {
		size_++;
	}
	ptr = malloc(sizeof(char) * (size_ + 1));
	assign(ptr, str, size_);
	*(static_cast<char*>(ptr)+size_) = 0;
}


String::String(const String& ref) : Value(VALUE_TYPE_STRING)
{
	size_ = ref.size_;
	ptr = malloc(sizeof(char) * (size_ + 1));
	assign(ptr, ref.ptr, size_);
	*(static_cast<char*>(ptr)+size_) = 0;
}
String& String::operator=(const String& ref)
{
	if (&ref == this) {
		return *this;
	}
	free(ptr);
	ptr = NULL;
	size_ = ref.size_;
	ptr = malloc(sizeof(char) * (size_ + 1));
	assign(ptr, ref.ptr, size_);
	*(static_cast<char*>(ptr)+size_) = 0;
	return *this;
}

String::~String()
{
	// free(ptr);
}
const char* String::c_str() 
{
	return static_cast<char*>(ptr);
}
} // namespace zv

std::ostream& operator<<(std::ostream& os, zv::String& str)
{
	os << str.c_str();
	return os;
}

#ifdef TEST
#include <string>

int main() {
	zv::String a("abcd"), b(a), c;
	std::cout << a << b << c << std::endl;
	std::cout << a.size() << b.size() << c.size() << std::endl;
	a = b;
	b = c;
	c = a;
	std::cout << a << b << c << std::endl;
	std::cout << a.size() << b.size() << c.size() << std::endl;

	std::string str("adasd");
	zv::String as_str(str.c_str());
	std::cout << as_str << std::endl;
	return 0;
}

#endif