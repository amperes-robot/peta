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
			control::set_mode(&follow_mode);
		}

		uint8_t pet_id = 0;
		uint8_t mark_hold = 0;
		void follow_begin()
		{
			io::log("follow/retrieval");

			if (pet_id == 0)
			{
				controller.gain_p = menu::flw_gain_p.value();
				controller.gain_i = menu::flw_gain_i.value();
				controller.gain_d = menu::flw_gain_d.value();
			}

			if (pet_id == 4)
			{
				control::set_mode(&beacon_homing_mode);
				return;
			}
		}

		void follow_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
			}

			bool qrd = io::Digital::qrd_side.read();

			if (!qrd && mark_hold < menu::flw_mark_lat.value())
			{
				mark_hold++;
			}
			else if (qrd)
			{
				mark_hold = 0;
			}

			if (mark_hold == menu::flw_mark_lat.value())
			{
				control::set_mode(&side_retrieval_mode);
			}

			int16_t in = io::Analog::qrd_tape.read();
			int16_t thresh = menu::flw_thresh.value();

			controller.in(in - thresh);
			int16_t out = controller.out();

			motion::vel(menu::flw_vel.value());
			motion::dir(out);
		}

		void follow_end()
		{
			controller.reset();
		}

		void side_retrieval_begin()
		{
			pet_id++;
		}

		void side_retrieval_tick()
		{
			// TODO
			static int timer = 0;

			if (timer == 0) 
			{
				motion::retrieval.speed(-20);
			}
			else if (timer == 50)
			{
				motion::retrieval.speed(20);
			}
			else if (timer == 100)
			{
				motion::retrieval.speed(0);
				control::set_mode(&follow_mode);
				return;
			}

			timer++;
		}
	}

	const control::Mode begin_mode
	{
		&control::nop,
		&begin_tick,
		&control::nop
	};

	const control::Mode follow_mode
	{
		&follow_begin,
		&follow_tick,
		&control::nop
	};

	const control::Mode side_retrieval_mode
	{
		&side_retrieval_begin,
		&side_retrieval_tick,
		&control::nop
	};

	const control::Mode beacon_homing_mode
	{
		[]() { io::log("BH"); },
		&control::nop,
		&control::nop
	};

	const control::Mode rubble_excavation_mode
	{
		&control::nop,
		&control::nop,
		&control::nop
	};

	const control::Mode return_mode
	{
		&control::nop,
		&control::nop,
		&control::nop
	};
}
