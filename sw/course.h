#pragma once

#include "control.h"
#include "io.h"

namespace course
{
	extern uint8_t pet_id;
	/**
	 * Control modes for the course.
	 */

	extern const control::Mode begin_mode;
	extern const control::Mode follow_mode;
	extern const control::Mode adjust_mode;
	extern const control::Mode retrieve_mode;
	extern const control::Mode recover_mode;
	extern const control::Mode reverse_follow_mode;
	extern const control::Mode beacon_homing_mode;
	extern const control::Mode parallel_park_mode;
	extern const control::Mode rubble_excavation_mode;
	extern const control::Mode return_mode;
}
