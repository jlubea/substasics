#include "substasics/platform/exceptions.h"
#include "substasics/platform/thread.h"
#include "substasics/platform/mutex.h"
#include "substasics/platform/signal.h"
#include "substasics/platform/text.h"
#include "substasics/log/trace.h"
#include "substasics/dependency_injection/kernel.h"

#include <iostream>
#include <string>
#include <vector>

namespace sp = substasics::platform;
namespace di = substasics::dependency_injection;

void demo_exceptions()
{
	static const char *func = "demo_exceptions";

	std::cout << std::endl;
	std::cout << "demo_exceptions:" << std::endl;
	try
	{
		throw sp::exception(func, "This is a sample format string: %s, %d", "Just some sample text", 500);
	}
	catch (std::exception &ex)
	{
		std::cerr << "substasics::platform::exception: " << std::endl;
		std::cerr << ex.what() << std::endl;
	}

	try
	{
		throw sp::last_system_error(func);
	}
	catch (std::exception &ex)
	{
		std::cerr << "substasics::platform::last_system_error: " << std::endl;
		std::cerr << ex.what() << std::endl;
	}
}

void demo_text()
{
	static const char *func = "demo_text";

	std::cout << std::endl;
	std::cout << "demo_text:" << std::endl;

	std::string testString   =  "  --- this is a test of the ebs ---  ";
	std::wstring testWString = L"  --- this is a test of the ebs ---  ";
	
	std::cout  << "narrow with c string: '" << sp::text::narrow(testString) << "'" << std::endl;
	std::cout  << "narrow with w string: '" << sp::text::narrow(testWString) << "'" << std::endl;
	std::wcout << "wide with c string:   '" << sp::text::wide(testString) << "'" << std::endl;
	std::wcout << "wide with w string:   '" << sp::text::wide(testWString) << "'" << std::endl;
	std::cout  << "utf8 with c string:   '" << sp::text::utf8(testString) << "'" << std::endl;
	std::cout  << "utf8 with w string:   '" << sp::text::utf8(testWString) << "'" << std::endl;

	std::cout  << "trim with c string:   '" << sp::text::trim(testString) << "'" << std::endl;
	std::wcout << "trim with w string:   '" << sp::text::trim(testWString) << "'" << std::endl;
}



class demo_runnable : public sp::runnable
{
public:
	demo_runnable(int id) :
		_id(id)
	{
	}

	void run()
	{
		// the output will probably be interleaved, but we don't care for this demo
		std::cout << "runnable " << _id << " starting" << std::endl;
		Sleep(2000);
		std::cout << "runnable " << _id << " ending" << std::endl;
	}

private:
	int _id;
};

void demo_operation_queue()
{
	std::cout << std::endl;
	std::cout << "demo_operation_queue:" << std::endl;

	sp::operation_queue opQueue(5);

	for (int i = 0; i < 12; ++i)
	{
		opQueue.add(new demo_runnable(i + 1));
	}

	opQueue.start();
	opQueue.wait_for_all();
}

void demo_operation_queue_with_concurrent_operation_count_change()
{
	std::cout << std::endl;
	std::cout << "demo_operation_queue_with_concurrent_operation_count_change:" << std::endl;

	int concurrentOperationCount = 1;
	sp::operation_queue opQueue(concurrentOperationCount);
	
	// notice that you can add operations even after the operation_queue is started
	opQueue.start();

	for (int i = 0; i < 16; ++i)
	{
		opQueue.add(new demo_runnable(i + 1));
	}

	for (int i = 0; i < 3; ++i)
	{
		Sleep(750);
		concurrentOperationCount *= 2;
		opQueue.set_max_concurrent_operation_count(concurrentOperationCount);
	}

	opQueue.wait_for_all();
}

void demo_parallel_for_each()
{
	std::cout << std::endl;
	std::cout << "demo_parallel_for_each:" << std::endl;

	std::vector<int> ids;
	for (int i = 0; i < 12; ++i)
	{
		ids.push_back(i + 1);
	}

	sp::parallel_for_each(
		5,
		ids,
		[=](int id)
		{
			// the output will probably be interleaved, but we don't care for this demo
			std::cout << "lambda runnable " << id << " starting" << std::endl;
			Sleep(2000);
			std::cout << "lambda runnable " << id << " ending" << std::endl;
		}
	);
}


class ISampleOne
{
public:
	static const std::string InterfaceKey;
	virtual std::string GetName() = 0;
};
const std::string ISampleOne::InterfaceKey = "ISampleOne";


class SampleImplOne : public ISampleOne
{
public:
	virtual std::string GetName() { return "SampleImplOne"; }
};

class SampleImplTwo : public ISampleOne
{
public:
	virtual std::string GetName() { return "SampleImplTwo"; }
};


class ISingletonOne
{
public:
	static const std::string InterfaceKey;
	virtual std::string GetName() = 0;
};
const std::string ISingletonOne::InterfaceKey = "ISingletonOne";

class SingletonImplOne : public ISingletonOne
{
public:
	virtual std::string GetName() { return "SingletonImplOne"; }
};

class SingletonImplTwo : public ISingletonOne
{
public:
	virtual std::string GetName() { return "SingletonImplTwo"; }
};


void demo_dependency_injection()
{
	std::cout << "demo_dependency_injection:" << std::endl;

	di::kernel kernel;

	kernel.bind<ISampleOne, SampleImplOne>(new di::object_factory<SampleImplOne>());
	kernel.bind<ISingletonOne, SingletonImplOne>(new di::singleton_factory<SingletonImplOne>());

	boost::shared_ptr<ISampleOne> sample1 = kernel.get<ISampleOne>();
	boost::shared_ptr<ISingletonOne> singleton1 = kernel.get<ISingletonOne>();
	std::cout << sample1->GetName() << std::endl;
	std::cout << singleton1->GetName() << std::endl;

	kernel.bind<ISampleOne, SampleImplTwo>(new di::object_factory<SampleImplTwo>());

	sample1 = kernel.get<ISampleOne>();
	singleton1 = kernel.get<ISingletonOne>();
	std::cout << sample1->GetName() << std::endl;
	std::cout << singleton1->GetName() << std::endl;

	kernel.bind<ISingletonOne, SingletonImplTwo>(new di::singleton_factory<SingletonImplTwo>());
	sample1 = kernel.get<ISampleOne>();
	singleton1 = kernel.get<ISingletonOne>();
	std::cout << sample1->GetName() << std::endl;
	std::cout << singleton1->GetName() << std::endl;
}

int main(int argc, char **argv)
{
	//demo_exceptions();
	//demo_text();
	//demo_operation_queue();
	//demo_operation_queue_with_concurrent_operation_count_change();
	//demo_parallel_for_each();

	demo_dependency_injection();

	return 0;
}