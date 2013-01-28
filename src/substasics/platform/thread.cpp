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
		while ((_op->canceled == false || (_started && !_terminated)) && WaitForSingleObject(_hThread, waitTimeout) != WAIT_OBJECT_0)
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
				while (true)
				{
					if (this->canceled)
					{
						break;
					}
					else
					{
						{
							scoped_lock(_opQueue->_spoolerMutex);

							// wait for an operation to be added
							if (_runningThreads.size() == 0 && _opQueue->_queuedThreads.size() == 0)
							{
								size_t completionEventCount = 2;
								boost::shared_array<HANDLE> _completionEvents(new HANDLE[completionEventCount]);
								_completionEvents[0] = _opQueue->_terminateSpoolerEvent.get_raw_event();
								_completionEvents[1] = _opQueue->_addedOperationEvent.get_raw_event();

								_opQueue->_spoolerMutex.unlock();
								DWORD waitResult = WaitForMultipleObjects(static_cast<DWORD>(completionEventCount), _completionEvents.get(), FALSE, INFINITE);
								_opQueue->_spoolerMutex.lock();

								// if terminate is requested, break
								if (waitResult == WAIT_OBJECT_0) break;
								// else an operation was added
							}

							// queue up operations to run
							while (_runningThreads.size() < _opQueue->_maxConcurrentOperations && _opQueue->_queuedThreads.size() > 0)
							{
								thread *t = _opQueue->_queuedThreads.back();
								_opQueue->_queuedThreads.pop_back();
						
								t->start();
								_runningThreads.push_back(t);
							}
						}

						// wait for any of the threads to complete, or wait for the max concurrent operation
						// count to change (where we may be allowed to queue up additional operations)
						size_t completionEventCount = _runningThreads.size() + 3;
						boost::shared_array<HANDLE> _completionEvents(new HANDLE[completionEventCount]);
						for (size_t i = 0; i < _runningThreads.size(); ++i)
						{
							_completionEvents[i] = _runningThreads[i]->get_raw_thread();
						}
						_completionEvents[_runningThreads.size()]     = _opQueue->_terminateSpoolerEvent.get_raw_event();
						_completionEvents[_runningThreads.size() + 1] = _opQueue->_concurrentOperationCountChangedEvent.get_raw_event();
						_completionEvents[_runningThreads.size() + 2] = _opQueue->_addedOperationEvent.get_raw_event();

						DWORD waitResult = WaitForMultipleObjects(static_cast<DWORD>(completionEventCount), _completionEvents.get(), FALSE, INFINITE);
						// if a thread has ended
						if (waitResult >= WAIT_OBJECT_0 && waitResult < (WAIT_OBJECT_0 + _runningThreads.size()))
						{
							_runningThreads.erase(_runningThreads.begin() + (waitResult - WAIT_OBJECT_0));
						}
						// if termination requested
						else if (waitResult == (WAIT_OBJECT_0 + _runningThreads.size()))
						{
							break;
						}
					}
				}

				for (size_t i = 0; i < _runningThreads.size(); ++i)
				{
					_runningThreads[i]->cancel();
				}
				for (size_t i = 0; i < _runningThreads.size(); ++i)
				{
					_runningThreads[i]->join();
				}
				_runningThreads.clear();
			}

		private:
			operation_queue *_opQueue;
			std::vector<thread *> _runningThreads;
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
		reset();
	}

	void operation_queue::add(runnable *op, bool ownsOp)
	{
		scoped_lock lock(_spoolerMutex);

		thread *t = new thread(op, ownsOp);
		_threads.push_back(t);
		_queuedThreads.push_back(t);
		_addedOperationEvent.notify();
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
	
		if (_spoolerThread)
		{
			_spoolerThread->cancel();
			_terminateSpoolerEvent.notify();
			_spoolerThread->join();

			delete _spoolerThread;
			_spoolerThread = NULL;
		}

		if (_spooler)
		{
			delete _spooler;
			_spooler = NULL;
		}
	}

	size_t operation_queue::get_max_concurrent_operation_count() const
	{
		return _maxConcurrentOperations;
	}

	void operation_queue::set_max_concurrent_operation_count(size_t maxConcurrentOperations)
	{
		scoped_lock lock(_spoolerMutex);

		_maxConcurrentOperations = max(1, maxConcurrentOperations);
		_concurrentOperationCountChangedEvent.notify();
	}

	void operation_queue::wait_for_all()
	{
		for (size_t i = 0; i < _threads.size(); ++i)
		{
			_threads[i]->join();
		}
	}

	const std::vector<thread *> &operation_queue::get_threads() const
	{
		return _threads;
	}

}
}
