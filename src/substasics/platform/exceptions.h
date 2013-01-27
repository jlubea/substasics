#ifdef _WIN32
# pragma once
#endif

#ifndef SUBSTASICS_PLATFORM_EXCEPTIONS_H
#define SUBSTASICS_PLATFORM_EXCEPTIONS_H

#include <stdexcept>
#include <string>

#include "substasics/platform/exports.h"

namespace substasics { namespace platform {

class SUBSTASICS_API exception : public std::runtime_error {
public:
	typedef std::runtime_error base_type;
	
public:
	exception(const std::string &func, const std::string &format, ...);
	virtual ~exception() throw();

	virtual const char *what() const throw();

private:
	std::string m_what;
};

class SUBSTASICS_API last_system_error : public std::runtime_error {
public:
	typedef std::runtime_error base_type;

	last_system_error();
	last_system_error(const std::string &func);
	virtual ~last_system_error();

	virtual const char *what() const throw();

private:
	std::string m_what;
};

}
}

#endif