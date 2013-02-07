#ifndef SUBSTASICS_LOG_TRACE_H
#define SUBSTASICS_LOG_TRACE_H

#include "substasics/platform/exports.h"

namespace substasics { namespace log {

class SUBSTASICS_API trace
{
public:
	trace(const char *func, const wchar_t *loggerName = L"main");
	~trace();

private:
	const char *_func;
	const wchar_t *_loggerName;
};

}
}

#endif