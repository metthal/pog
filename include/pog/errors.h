#pragma once

#include <exception>
#include <string>

namespace pog {

class Error : public std::exception
{
public:
	template <typename T>
	Error(T&& msg) noexcept : _msg(std::forward<T>(msg)) {}
	Error(const Error& o) noexcept : _msg(o._msg) {}
	virtual ~Error() noexcept {}

	virtual const char* what() const noexcept override { return _msg.c_str(); }

private:
	std::string _msg;
};

class SyntaxError : public Error
{
public:
	using Error::Error;
};

} // namespace pog
