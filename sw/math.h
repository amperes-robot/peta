#pragma once
#include "io.h"

namespace math
{
	const uint16_t pi = 1 << 15;
	const uint16_t full = (uint16_t) -1;

	int16_t sin(uint16_t theta);
	int16_t cos(uint16_t theta);
}
