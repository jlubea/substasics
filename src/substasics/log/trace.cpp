#include "substasics/log/trace.h"

#include <log4cplus/logger.h>
#include <iostream>

#pragma warning (disable:4127)

namespace substasics { namespace log {

trace::trace(const char *func, const wchar_t *loggerName)
	: _func(func), _loggerName(loggerName)
{
	log4cplus::Logger logger = log4cplus::Logger::getInstance(_loggerName);
	LOG4CPLUS_TRACE(logger, "Begin " << _func);
}


trace::~trace()
{
	log4cplus::Logger logger = log4cplus::Logger::getInstance(_loggerName);
	LOG4CPLUS_TRACE(logger, "End   " << _func);
}

}
}