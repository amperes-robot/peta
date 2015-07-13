#include "course.h"
#include "isr.h"
#include "io.h"
#include "menu.h"
#include "motion.h"
#include "pid.h"

namespace course
{
	namespace
	{
		enum { ARM_LO_THRESH = -20, ARM_HI_THRESH = -5 };

		pid::Controller controller(0, 0, 0);

		void begin_tick()
		{
			control::set_mode(&follow_mode);
		}

		uint8_t pet_id = 0;
		uint8_t mark_hold = 0;
		uint8_t retry_count;
		uint8_t state;

		void follow_begin()
		{
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

			if (motion::arm_theta < ARM_HI_THRESH)
			{
				motion::arm.speed(200);
			}
		}

		void follow_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			if (motion::arm_theta > ARM_HI_THRESH)
			{
				motion::arm.halt();
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
				return;
			}

			int16_t in = pid::follow_value();

			controller.in(in);
			int16_t out = controller.out();

			motion::vel(menu::flw_vel.value());
			motion::dir(out);

			motion::update_enc();
			io::delay_ms(10);
		}

		void follow_end()
		{
			controller.reset();
		}

		void side_retrieval_begin()
		{
			motion::update_enc(); // clear queue
			motion::arm_theta = 0;

			retry_count = 0;
			state = 0;
			pet_id++;
		}

		void side_retrieval_tick()
		{
			enum { N_RETRIES = 2 };
			enum
			{
				DROPPING_BEGIN = 0,
				DROPPING,
				BRAKE_BEGIN,
				BRAKE,
				LIFTING_BEGIN,
				LIFTING,
				RETRY_BEGIN,
				RETRY,
				ZERO_BEGIN,
				ZERO,
				DONE_BEGIN,
				DONE
			};

			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			switch (state) // dropping arm
			{
				case DROPPING_BEGIN:
				{
					motion::arm.speed(-200);
					state++;
					// fall through
				}
				case DROPPING:
				{
					if (motion::arm_theta < ARM_LO_THRESH /*|| io::Digital::switch_upper.read()*/) // dropped
					{
						state = BRAKE_BEGIN;
					}
					else if (motion::arm_theta < ARM_LO_THRESH)
					{
						state = DONE_BEGIN; // not detected, abort
					}
					break;
				}
				case BRAKE_BEGIN:
				{
					motion::arm.halt();
					state++;
					io::Timer::start();
					// fall through
				}
				case BRAKE:
				{
					if (io::Timer::time() > 750)
					{
						state = LIFTING_BEGIN;
					}
					break;
				}
				case LIFTING_BEGIN:
				{
					motion::arm.speed(255);
					state++;
					io::Timer::start();
					// fall through
				}
				case LIFTING:
				{
					if (motion::arm_theta > ARM_HI_THRESH /* || !io::Digital::switch_upper.read()*/) // detached or lost or up
					{
						state = ZERO_BEGIN;
					}
					else if (io::Timer::time() > 2000) // timeout
					{
						if (retry_count < N_RETRIES)
						{
							state = RETRY_BEGIN;
						}
						else
						{
							state = ZERO_BEGIN;
						}
					}
					break;
				}
				case RETRY_BEGIN:
				{
					retry_count++;
					state++;
					motion::arm.speed(-128);
					// fall through
				}
				case RETRY:
				{
					if (motion::arm_theta < ARM_LO_THRESH)
					{
						state = BRAKE_BEGIN;
					}
					break;
				}
				case ZERO_BEGIN:
				{
					motion::arm.speed(255);
					io::Timer::start();
					state++;
					// fall through
				}
				case ZERO:
				{
					if (io::Timer::time() > 1000)
					{
						state = DONE_BEGIN;
					}
					break;
				}
				case DONE_BEGIN:
				{
					motion::arm.halt();
					state++;
					// fall through
				}
				case DONE:
				{
					control::set_mode(&menu::main_mode);
					// control::set_mode(&parallel_park_mode);
					break;
				}
			}

			motion::update_enc();
			io::delay_ms(20);
		}

		void parallel_park_begin()
		{
			state = 0;
		}

		void parallel_park_tick()
		{
			enum
			{
				ENTRY_BEGIN,
				ENTRY,
				BACK_BEGIN,
				BACK,
				FORWARD_BEGIN,
				FORWARD
			};

			switch (state)
			{
				case ENTRY_BEGIN:
				{
					state++;
					motion::left.halt();
					motion::right.speed(50);
					motion::right_theta = 0;
					// fall through
				}
				case ENTRY: // go in
				{
					if (motion::right_theta > 100)
					{
						state = BACK_BEGIN;
					}
					break;
				}
				case BACK_BEGIN:
				{
					state++;
					motion::left.speed(-50);
					motion::right.speed(-70);
					motion::right_theta = 0;
					motion::left_theta = 0;
					// fall through
				}
				case BACK:
				{
					if ((motion::right_theta + motion::left_theta) / 2 > 100)
					{
						state = FORWARD_BEGIN;
					}
					break;
				}
				case FORWARD_BEGIN:
				{
					state++;
					motion::left.speed(50);
					motion::right.halt();
					motion::left_theta = 0;
					// fall through
				}
				case FORWARD:
				{
					if (motion::left_theta > 50)
					{
						control::set_mode(&rubble_excavation_mode);
					}
					break;
				}
			}

			io::delay_ms(20);
			motion::update_enc();
		}

		void beacon_homing_begin()
		{
			controller.reset();
			controller.gain_p = menu::home_gain_p.value();
			controller.gain_i = menu::home_gain_i.value();
			controller.gain_d = menu::home_gain_d.value();
		}

		void beacon_homing_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			int16_t in = io::Analog::pd_left.read() - io::Analog::pd_right.read();
			int16_t thresh = menu::home_thresh.value();

			controller.in(in - thresh);
			int16_t out = controller.out();

			motion::vel(menu::home_vel.value());
			motion::dir(out);
		}

		void rubble_excavation_begin()
		{
			state = 0;
		}

		void rubble_excavation_tick()
		{
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
		&beacon_homing_begin,
		&beacon_homing_tick,
		&control::nop
	};

	const control::Mode parallel_park_mode
	{
		&parallel_park_begin,
		&parallel_park_tick,
		&control::nop
	};

	const control::Mode rubble_excavation_mode
	{
		&rubble_excavation_begin,
		&rubble_excavation_tick,
		&control::nop
	};

	const control::Mode return_mode
	{
		&control::nop,
		&control::nop,
		&control::nop
	};
}
