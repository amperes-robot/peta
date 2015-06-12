#pragma once
#include "io.h"

namespace motion
{
	/*
	 * Move in a direction, with -255 being right, 255 being left.
	 */
	void dir(int16_t x);

	/*
	 * Speed.
	 */
	void vel(int16_t x);
}
