#include "course.h"
#include "io.h"
#include "menu.h"
#include "motion.h"
#include "pid.h"

namespace course
{
	namespace
	{
		pid::Controller controller(0, 0, 0);

		void begin_tick()
		{
			control::set_mode(&follow_retrieval_mode);
		}

		void follow_retrieval_begin(void*)
		{
			io::log("follow/retrieval");
			controller.gain_p = menu::flw_gain_p.value();
			controller.gain_i = menu::flw_gain_i.value();
			controller.gain_d = menu::flw_gain_d.value();
		}

		void follow_retrieval_tick()
		{
			int16_t in = io::Analog::qrd_tape.read();
			int16_t thresh = menu::flw_thresh.value();

			controller.in((in - thresh));
			int16_t out = controller.out();

			motion::vel(menu::flw_vel.value());
			motion::dir(out);

			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
			}
			io::delay_ms(10);
		}
	}

	const control::Mode begin_mode
	{
		&control::nop_void,
		&begin_tick,
		&control::nop
	};

	const control::Mode follow_retrieval_mode
	{
		&follow_retrieval_begin,
		&follow_retrieval_tick,
		&control::nop
	};

	const control::Mode beacon_homing_mode
	{
		[](void*) { io::log("beacon/homing"); },
		&control::nop,
		&control::nop
	};

	const control::Mode rubble_excavation_mode
	{
		&control::nop_void,
		&control::nop,
		&control::nop
	};

	const control::Mode return_mode
	{
		&control::nop_void,
		&control::nop,
		&control::nop
	};
}
