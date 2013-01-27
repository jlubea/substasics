#include "substasics/platform/mutex.h"

#include <Windows.h>

namespace substasics { namespace platform {

	mutex::mutex()
	{
		_win32Mutex = CreateMutex(NULL, FALSE, NULL);
	}

	mutex::~mutex()
	{
		destroy();
	}

	bool mutex::lock(DWORD waitTimeout)
	{
		if (_win32Mutex == NULL)
		{
			return false;
		}

		DWORD rc = WaitForSingleObject(_win32Mutex, waitTimeout);
		return rc == WAIT_OBJECT_0;
	}

	bool mutex::unlock()
	{
		if (_win32Mutex == NULL)
		{
			return true;
		}

		BOOL rc = ReleaseMutex(_win32Mutex);
		return rc == TRUE;
	}

	void mutex::destroy()
	{
		if (_win32Mutex != NULL)
		{
			CloseHandle(_win32Mutex);
			_win32Mutex = NULL;
		}
	}


	scoped_lock::scoped_lock(mutex &m, DWORD waitTimeout) :
		_mutex(m)
	{
		_mutex.lock(waitTimeout);
	}
	scoped_lock::~scoped_lock()
	{
		_mutex.unlock();
	}

}
}