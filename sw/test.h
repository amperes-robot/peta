#pragma once
#include <cmath>
#include <iostream>
#include <string>

typedef void (*TestHandle)(void);

struct TestCallback
{
	TestHandle handle;
	std::string name;
};

extern std::string assert_message;

#define STR_X(x) #x
#define STR(x) STR_X(x)

#define ASSERT(x) \
	do \
	{ \
		if (!x) \
		{ \
			assert_message = "assertion failed: " STR(x) " at " __FILE__ ":" STR(__LINE__); \
			return; \
		} \
	} while (0)

#define ASSERT_EQ(exp, act) \
	do \
	{ \
		if (exp != act) \
		{ \
			assert_message = "expected " STR(exp) " but found " + std::to_string(act) " at " __FILE__ ":" STR(__LINE__); \
			return; \
		} \
	} while (0)

#define ASSERT_EQF(exp, act, tolerance) \
	do \
	{ \
		if (fabs(exp - act) > tolerance) \
		{ \
			assert_message = "expected " STR(exp) " with tolerance " STR(tol) " but found " + std::to_string(act) " at " __FILE__ ":" STR(__LINE__); \
			return; \
		} \
	} while (0)

#define SUITE_ENTRY(x) { &x, STR(x) }
#define SUITE_END() { nullptr, "" }
