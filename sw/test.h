#pragma once
#include <iostream>
#include <string>

typedef void (*TestCallback)(void);

#define ASSERT(x) \
	do \
	{ \
		if (!x) \
		{ \
			std::cerr << "Assertion failed: " #x " at " __FILE__ ":" << __LINE__ << std::endl; \
			return; \
		} \
	} while (0)
