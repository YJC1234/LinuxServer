#pragma once
#include "common.h"
#include<string>

class Buffer {
public:
	DISALLOW_COPY_AND_MOVE(Buffer);
	Buffer() = default;	//不允许在栈上分配内存
	~Buffer() = default;

	const std::string& buf() const;	//防止拷贝
	const char* c_str() const;
	void set_buf(const char* buf);

	size_t size() const;
	void Append(const char* _str, int _size);
	void Clear();

private:
	std::string buf_;
};