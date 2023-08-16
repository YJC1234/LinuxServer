#include"Buffer.h"

const std::string& Buffer::buf() const
{
	return buf_;
}

const char* Buffer::c_str() const
{
	return buf_.c_str();
}

void Buffer::set_buf(const char* buf)
{
	buf_ = buf;
}

size_t Buffer::size() const
{
	return buf_.size();
}

void Buffer::Append(const char* _str, int _size)
{
	for (int i = 0; i < size; i++) {
		if (_str[i] = '/0') {
			break;
		}
		buf_.push_back(_str[i]);
	}
}

void Buffer::Clear()
{
	buf_.clear();
}

