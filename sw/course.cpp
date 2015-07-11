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
				LIFTING_BEGIN,
				LIFTING,
				RETRY_BEGIN,
				RETRY,
				DONE_BEGIN,
				DONE
			};

			io::lcd.clear();
			io::lcd.home();
			io::lcd.setCursor(0, 1);
			io::lcd.print(motion::arm_theta);

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
						state = LIFTING_BEGIN;
					}
					else if (motion::arm_theta < ARM_LO_THRESH)
					{
						state = DONE_BEGIN; // not detected, abort
					}
					break;
				}
				case LIFTING_BEGIN:
				{
					motion::arm.halt();
					motion::update_enc();

					io::delay_ms(1000);

					io::lcd.print(' ');
					io::lcd.print(motion::enc2_counts);

					motion::update_enc();
					io::lcd.print(' ');
					io::lcd.print(motion::arm_theta);

					io::delay_ms(500);

					motion::arm.speed(255);
					state++;
					io::Timer::start();
					// fall through
				}
				case LIFTING:
				{
					if (motion::arm_theta > ARM_HI_THRESH /* || !io::Digital::switch_upper.read()*/) // detached or lost or up
					{
						state = DONE_BEGIN;
					}
					else if (io::Timer::time() > 5000) // timeout
					{
						if (retry_count < N_RETRIES)
						{
							state = RETRY_BEGIN;
						}
						else
						{
							state = DONE_BEGIN;
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
						state = LIFTING_BEGIN;
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
					io::delay_ms(1000);
					motion::update_enc();

					state = DROPPING_BEGIN;
					break;
				}
			}

			motion::update_enc();
			io::delay_ms(10);
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
