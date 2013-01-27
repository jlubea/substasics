#ifdef _WIN32
# pragma once
#endif

#ifndef SUBSTASICS_PLATFORM_MUTEX_H
#define SUBSTASICS_PLATFORM_MUTEX_H

#include "substasics/platform/exports.h"
#include <boost/noncopyable.hpp>

#include <Windows.h>

namespace substasics { namespace platform {

	class SUBSTASICS_API mutex : private boost::noncopyable
	{
	public:
		mutex();
		~mutex();

		bool lock(DWORD waitTimeout = INFINITE);
		bool unlock();
		void destroy();

	private:
		HANDLE _win32Mutex;
	};

	// disable "cannot generate an assignment operator for this class".  yay references!
	//#pragma warning (push)
	//#pragma warning (disable: 4512)

	class SUBSTASICS_API scoped_lock : private boost::noncopyable
	{
	public:
		scoped_lock(mutex &m, DWORD waitTimeout = INFINITE);
		~scoped_lock();

	private:
		mutex &_mutex;
	};

	//#pragma warning (pop)

}
}

#endif