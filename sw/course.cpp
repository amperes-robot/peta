#include "course.h"
#include "io.h"

namespace course
{
	namespace
	{
		void nop() { }

		void begin_tick()
		{
			control::set_mode(&follow_retrieval_mode);
		}

		void follow_retrieval_tick()
		{
			io::log("follow/retrieval");
		}
	}

	const control::Mode begin_mode
	{
		&nop,
		&begin_tick,
		&nop
	};

	const control::Mode follow_retrieval_mode
	{
		&nop,
		&follow_retrieval_tick,
		&nop
	};

	const control::Mode beacon_homing_mode
	{
		&nop,
		&nop,
		&nop
	};

	const control::Mode rubble_excavation_mode
	{
		&nop,
		&nop,
		&nop
	};

	const control::Mode return_mode
	{
		&nop,
		&nop,
		&nop
	};
}
