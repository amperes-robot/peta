#pragma once

#include "control.h"
#include "io.h"

namespace course
{
	enum BeginFlags : uint8_t
	{
		BEGIN_SHORTENED = 1U << 0
	};

	extern uint8_t begin_flags;

	extern const control::Mode begin_mode;
}
