#ifdef _WIN32
# pragma once
#endif

#ifndef SUBSTASICS_PLATFORM_SIGNAL_H
#define SUBSTASICS_PLATFORM_SIGNAL_H

#include "substasics/platform/exports.h"

#include <Windows.h>

namespace substasics { namespace platform {

class SUBSTASICS_API signal
{
public:
	signal(bool signaled = false, bool autoReset = true);
	~signal();

	bool wait(BOOL waitTimeout = INFINITE);
	void notify();

	HANDLE get_raw_event() const;

private:
	HANDLE _hEvent;
};

}
}

#endif