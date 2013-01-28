#ifdef _WIN32
# pragma once
#endif

#ifndef SUBSTASICS_PLATFORM_THREAD_H
#define SUBSTASICS_PLATFORM_THREAD_H

#include "substasics/platform/exports.h"
#include "substasics/platform/signal.h"
#include "substasics/platform/mutex.h"
#include "substasics/platform/exceptions.h"

#include <vector>
#include <stdint.h>

namespace substasics { namespace platform {

	class SUBSTASICS_API runnable
	{
		friend class thread;

	public:
		runnable();
		virtual ~runnable() { }

		virtual void run() = 0;

	protected:
		volatile bool canceled;
	};


	class SUBSTASICS_API thread
	{
	public:
		thread(runnable *op, bool ownsOp = true);
		~thread();

		bool is_started() const;

		void start();
		void join(DWORD waitTimeout = INFINITE);
		void run();
		void cancel();

		runnable *get_runnable() const;
		HANDLE get_raw_thread() const;

	private:
		runnable *_op;
		bool _ownsOp;

		HANDLE _hThread;

		volatile bool _started;
		volatile bool _terminated;
	};

	// forward declaration
	namespace detail
	{
		class operation_queue_spooler;
	}

	class SUBSTASICS_API operation_queue
	{
		friend class detail::operation_queue_spooler;

	public:
		operation_queue(size_t maxConcurrentOperations);
		~operation_queue();

		void add(runnable *op, bool ownsOp = true);
		void start();
		void reset();

		size_t get_max_concurrent_operation_count() const;
		void set_max_concurrent_operation_count(size_t maxConcurrentOperations);

		void wait_for_all();

		const std::vector<thread *> &get_threads() const;

	private:
		size_t _maxConcurrentOperations;
		std::vector<thread *> _threads;
		std::vector<thread *> _queuedThreads;
		
		detail::operation_queue_spooler *_spooler;
		thread *_spoolerThread;
		signal _concurrentOperationCountChangedEvent;
		signal _addedOperationEvent;
		signal _terminateSpoolerEvent;

		mutex _spoolerMutex;
	};


	// lambda based threading
	template <typename _Ctx, class _Fn1>
	class lambda_runnable : public runnable
	{
	public:
		lambda_runnable(_Ctx &ctx, _Fn1 func) :
			runnable(),
			_ctx(ctx),
			_func(func)
		{
		}
		virtual ~lambda_runnable()
		{
		}

		virtual void run()
		{
			_func(_ctx);
		}

	private:
		_Ctx &_ctx;
		_Fn1 _func;
	};

	template <typename _Ctx, class _Collection, class _Fn1>
	void parallel_for_each(uint32_t maxConcurrentOperations, _Collection &contextObjects, _Fn1 func)
	{
		size_t threadCount = contextObjects.size();

		if (threadCount == 0)
		{
			throw exception("substasics::platform::parallel_for_each", "Invalid number of threads requested: %d", threadCount);
		}
		else if (threadCount == 1)
		{
			func(*contextObjects.begin());
		}
		else
		{
			operation_queue opQueue(maxConcurrentOperations);
			for (_Collection::iterator iter = contextObjects.begin(); iter != contextObjects.end(); ++iter)
			{
				opQueue.add(new lambda_runnable<_Ctx, _Fn1>(*iter, func));
			}
			opQueue.start();
			opQueue.wait_for_all();
		}
	}

	template <typename _Ctx, class _Collection, class _Fn1>
	void parallel_for_each(_Collection &contextObjects, _Fn1 func)
	{
		size_t threadCount = contextObjects.size();
		parallel_for_each<_Ctx, _Collection, _Fn1>(threadCount, contextObjects, func);
	}

}
}

#endif