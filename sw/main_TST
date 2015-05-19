#include <iostream>
#include "test.h"
#include "io.h"
#include "loop.cpp"

const TestCallback* suites[] =
{
	io::test_suite,
	nullptr
};

int main()
{
	for (const TestCallback** i = suites; *i != nullptr; i++)
	{
		for (const TestCallback* o = *i; *o != nullptr; o++)
		{
			(*o)();
		}
	}
	return 0;
}
