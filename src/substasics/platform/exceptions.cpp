#include "substasics/platform/exceptions.h"
#include <boost/shared_array.hpp>

#include <stdarg.h>

#ifdef _WIN32
# include <Windows.h>
#endif

namespace substasics { namespace platform {

	exception::exception(const std::string &func, const char *format, ...)
		: base_type(func)
	{
		va_list args;

		va_start(args, format);
		// _vscprintf doesn't count terminating NULL char
		int len = _vscprintf(format, args) + 1;
		boost::shared_array<char> buffer(new char[len]);
		vsprintf_s(buffer.get(), len, format, args);
		m_what = func + ": " + buffer.get();
	}

	exception::~exception() throw()
	{
	}

	const char *exception::what() const throw()
	{
		return m_what.c_str();
	}

#ifdef _WIN32
	last_system_error::last_system_error() :
		base_type("")
	{
		LPSTR systemError = NULL;
		if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						   NULL,
						   GetLastError(),
						   0,
						   (LPSTR)&systemError,
						   0,
						   NULL))
		{
			if (systemError) LocalFree(systemError);
			static const size_t bufLen = 256;
			systemError = (char *)LocalAlloc(0, bufLen);
			sprintf_s(systemError, bufLen, "Format Message failed with error: 0x%x", GetLastError());
		}

		m_what = systemError;
		if (systemError) LocalFree(systemError);
	}
	last_system_error::last_system_error(const std::string &func) :
		base_type("")
	{
		LPSTR systemError = NULL;
		if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						   NULL,
						   GetLastError(),
						   0,
						   (LPSTR)&systemError,
						   0,
						   NULL))
		{
			if (systemError) LocalFree(systemError);
			static const size_t bufLen = 256;
			systemError = (char *)LocalAlloc(0, bufLen);
			sprintf_s(systemError, bufLen, "Format Message failed with error: 0x%x", GetLastError());
		}

		m_what = func + ": " + systemError;
		if (systemError) LocalFree(systemError);
	}

	last_system_error::~last_system_error()
	{
	}

	const char *last_system_error::what() const throw()
	{
		return m_what.c_str();
	}
#endif

}
}