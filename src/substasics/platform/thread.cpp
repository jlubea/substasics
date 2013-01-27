#include "substasics/platform/thread.h"

#include <process.h>
#include <algorithm>

#pragma warning (push)
#pragma warning (disable:4127)

#include <boost/date_time.hpp>
#include <boost/shared_array.hpp>

#pragma warning (pop)

namespace substasics { namespace platform {

	runnable::runnable()
		: canceled(false)
	{
	}

	//
	// Thread
	//
	extern "C" unsigned int __stdcall OperationThreadStart(void *arg)
	{
		thread *t = reinterpret_cast<thread *>(arg);
	
		t->run();
	
		_endthreadex(0);
		return 0;
	}

	thread::thread(runnable *op, bool ownsOp) :
		_op(op),
		_hThread(NULL),
		_ownsOp(ownsOp),
		_started(false),
		_terminated(false)
	{
	}

	thread::~thread()
	{
		if (_hThread)
		{
			join();
			CloseHandle((HANDLE)_hThread);
			_hThread = NULL;
		}

		if (_ownsOp)
			delete _op;
	}

	bool thread::is_started() const
	{
		return _started;
	}

	void thread::start()
	{
		_started = true;
		_hThread = (HANDLE)_beginthreadex( NULL,
											0,
											OperationThreadStart,
											this,
											0,
											NULL );
	}

	void thread::join(DWORD waitTimeout)
	{
		using namespace boost::posix_time;
		ptime beginClock(microsec_clock::universal_time());
		while (_started && !_terminated && WaitForSingleObject(_hThread, waitTimeout) != WAIT_OBJECT_0)
		{
			if (waitTimeout != INFINITE)
			{
				ptime endClock(microsec_clock::universal_time());
				time_duration elapsedTime = endClock - beginClock;
				if (elapsedTime >= milliseconds(waitTimeout))
				{
					break;
				}
			}
		}
	}

	void thread::run()
	{
		_op->run();
		_terminated = true;
	}

	void thread::cancel()
	{
		_op->canceled = true;
	}

	runnable *thread::get_runnable() const
	{
		return _op;
	}

	HANDLE thread::get_raw_thread() const
	{
		return _hThread;
	}



	namespace detail
	{
		class operation_queue_spooler : public runnable
		{
		public:
			operation_queue_spooler(operation_queue *opQueue)
				: runnable(),
				_opQueue(opQueue)
			{
			}

			virtual ~operation_queue_spooler()
			{
			}

			void run()
			{
				for (size_t i = 0; i < _opQueue->_threads.size(); ++i)
				{
					_pendingThreads.push_back(_opQueue->_threads[i]);
				}

				while (_pendingThreads.size() > 0 || _runningThreads.size() > 0)
				{
					if (this->canceled)
					{
						for (size_t i = 0; i < _runningThreads.size(); ++i)
						{
							_runningThreads[i]->cancel();
						}
						for (size_t i = 0; i < _runningThreads.size(); ++i)
						{
							_runningThreads[i]->join();
						}

						_runningThreads.clear();
						_pendingThreads.clear();
					}
					else
					{
						size_t maxThreadsToAdd = _opQueue->_maxConcurrentOperations - _runningThreads.size();
						if (maxThreadsToAdd > 0 && _pendingThreads.size() > 0)
						{
							size_t i = 0;
							for ( ; i < maxThreadsToAdd && i < _pendingThreads.size(); ++i)
							{
								_pendingThreads[i]->start();
								_runningThreads.push_back(_pendingThreads[i]);
							}
							_pendingThreads.erase(_pendingThreads.begin(), _pendingThreads.begin()+i);
						}

						// wait for 1 thread to complete
						boost::shared_array<HANDLE> _runningThreadCompletionEvents(new HANDLE[_runningThreads.size()]);
						for (size_t i = 0; i < _runningThreads.size(); ++i)
						{
							_runningThreadCompletionEvents[i] = _runningThreads[i]->get_raw_thread();
						}

						DWORD waitResult = WaitForMultipleObjects(static_cast<DWORD>(_runningThreads.size()), _runningThreadCompletionEvents.get(), FALSE, INFINITE);
						// a thread has ended, so remove it from the running list
						if (waitResult >= WAIT_OBJECT_0 && waitResult < WAIT_OBJECT_0+_runningThreads.size())
						{
							_runningThreads.erase(_runningThreads.begin() + (waitResult - WAIT_OBJECT_0));
						}
					}
				}
			}

		private:
			operation_queue *_opQueue;
			std::vector<thread *> _runningThreads;
			std::vector<thread *> _pendingThreads;
		};
	}

	operation_queue::operation_queue(size_t maxConcurrentOperations)
		: _maxConcurrentOperations(maxConcurrentOperations),
		_spooler(NULL),
		_spoolerThread(NULL)
	{
	}

	operation_queue::~operation_queue()
	{
		for (size_t i = 0; i < _threads.size(); ++i)
		{
			delete _threads[i];
		}
		_threads.clear();
	
		if (_spooler)
		{
			delete _spooler;
			_spooler = NULL;
		}

		if (_spoolerThread)
		{
			delete _spoolerThread;
			_spoolerThread = NULL;
		}
	}

	void operation_queue::add(runnable *op, bool ownsOp)
	{
		_threads.push_back(new thread(op, ownsOp));
	}

	void operation_queue::start()
	{
		if (!_spooler)
		{
			_spooler = new detail::operation_queue_spooler(this);
		}

		if (!_spoolerThread)
		{
			_spoolerThread = new thread(_spooler, false);
			_spoolerThread->start();
		}
	}

	void operation_queue::reset()
	{
		for (size_t i = 0; i < _threads.size(); ++i)
		{
			_threads[i]->cancel();
		}

		wait_for_all();

		for (size_t i = 0; i < _threads.size(); ++i)
		{
			delete _threads[i];
		}
		_threads.clear();
	
		if (_spooler)
		{
			delete _spooler;
			_spooler = NULL;
		}

		if (_spoolerThread)
		{
			delete _spoolerThread;
			_spoolerThread = NULL;
		}
	}

	void operation_queue::wait_for_all()
	{
		for (size_t i = 0; i < _threads.size(); ++i)
		{
			_threads[i]->join();
		}
		
		if (_spoolerThread != NULL)
		{
			_spoolerThread->join();
		}
	}

	const std::vector<thread *> &operation_queue::get_threads() const
	{
		return _threads;
	}

}
}
