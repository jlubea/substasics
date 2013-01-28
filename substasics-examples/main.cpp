#include "substasics/platform/exceptions.h"
#include "substasics/platform/thread.h"
#include "substasics/platform/mutex.h"
#include "substasics/platform/signal.h"
#include "substasics/platform/text.h"
#include "substasics/log/trace.h"

#include <iostream>
#include <string>
#include <vector>

namespace sp = substasics::platform;

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

void demo_parallel_for_each()
{
	std::cout << std::endl;
	std::cout << "demo_parallel_for_each:" << std::endl;

	std::vector<int> ids;
	for (int i = 0; i < 12; ++i)
	{
		ids.push_back(i + 1);
	}

	sp::parallel_for_each<int>(
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

int main(int argc, char **argv)
{
	demo_exceptions();
	demo_text();
	demo_operation_queue();
	demo_parallel_for_each();

	return 0;
}