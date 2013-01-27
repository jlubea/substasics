#include "substasics/platform/signal.h"

#pragma warning (push)
#pragma warning (disable:4127)

#include <boost/date_time.hpp>

#pragma warning (pop)

namespace substasics { namespace platform {

	signal::signal(bool signaled, bool autoReset)
	{
		_hEvent = CreateEvent(NULL,
			autoReset == true ? FALSE : TRUE,
			signaled == true ? TRUE : FALSE,
			NULL);
	}

	signal::~signal()
	{
		CloseHandle(_hEvent);
		_hEvent = NULL;
	}

	bool signal::wait(BOOL waitTimeout)
	{
		using namespace boost::posix_time;
		ptime beginClock(microsec_clock::universal_time());
		while (WaitForSingleObject(_hEvent, waitTimeout) != WAIT_OBJECT_0)
		{
			if (waitTimeout != INFINITE)
			{
				ptime endClock(microsec_clock::universal_time());
				time_duration elapsedTime = endClock - beginClock;
				if (elapsedTime >= milliseconds(waitTimeout))
				{
					return false;
				}
			}
		}
		return true;
	}

	void signal::notify()
	{
		SetEvent(_hEvent);
	}

	HANDLE signal::get_raw_event() const
	{
		return _hEvent;
	}

}
}