#pragma once

#include "control.h"

namespace course
{
	/**
	 * Control modes for the course.
	 */

	extern const control::Mode begin_mode;
	extern const control::Mode follow_mode;
	extern const control::Mode adjust_mode;
	extern const control::Mode side_retrieval_mode;
	extern const control::Mode recover_mode;
	extern const control::Mode beacon_homing_mode;
	extern const control::Mode parallel_park_mode;
	extern const control::Mode rubble_excavation_mode;
	extern const control::Mode return_mode;
}
