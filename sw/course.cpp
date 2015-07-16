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
		enum { ARM_LO_THRESH = -23, ARM_HI_THRESH = -5 };
		enum { ZERO_TURN_THETA = 16, ZERO_BACK_THETA = -50, ZERO_FWD_THETA = 40, ONE_TURN_THETA = 10, ONE_FWD_THETA = 30, TWO_BACKUP_THETA = -20 };
		enum { PICKUP_COOLDOWN = 300 };
		const int16_t SLOW_SPEED = 120;
		const int16_t MEDIUM_SPEED = 180;
		const int16_t FAST_SPEED = 210;
		const int16_t LUDICROUS_SPEED = 255;

		pid::DigitalController controller(0, 0, 0);

		uint8_t pet_id;
		uint8_t retry_count;
		uint8_t state;

		void begin_tick()
		{
			pet_id = 0;
			control::set_mode(&follow_mode);
		}

		void follow_begin()
		{
			io::lcd.home();
			io::lcd.clear();
			io::lcd.print(TO_FSTR(strings::follow));

			io::Timer::start();
			state = 0;
			
			if (pet_id == 0)
			{
				controller.reset();
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
			enum { HOLD_AMT = 1, DELAY_AMT = 2 };

			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			bool qrd = io::Analog::qrd_side.read() > menu::flw_thresh_side.value();

			if (state > DELAY_AMT)
			{
				if (io::Timer::time() > PICKUP_COOLDOWN)
				{
					control::set_mode(&adjust_mode);
					return;
				}
			}
			else if (state > HOLD_AMT || qrd)
			{
				state++;
			}
			else
			{
				state = 0; // debouncer failed
			}

			int8_t in = pid::follow_value_digital();

			controller.in(in);
			int16_t out = controller.out();

			motion::vel(menu::flw_vel.value());
			motion::dir(out);

			io::delay_ms(10);
		}

		void adjust_begin()
		{
			io::lcd.clear();
			io::lcd.home();
			io::lcd.print("adjust");

			state = 0;
			motion::left.halt();
			motion::right.halt();
		}

		enum
		{
			ZERO_BACK_BEGIN = 0,
			ZERO_BACK,
			ZERO_TURN_BEGIN,
			ZERO_TURN,
			ZERO_FWD_BEGIN,
			ZERO_FWD
		};
		enum
		{
			ONE_TURN_BEGIN = 0,
			ONE_TURN,
			ONE_FWD_BEGIN,
			ONE_FWD
		};
		enum
		{
			TWO_BACKUP_BEGIN = 0,
			TWO_BACKUP
		};

		void adjust_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			if (pet_id == 0)
			{
				switch (state)
				{
					case ZERO_BACK_BEGIN:
					{
						io::lcd.setCursor(0, 1);
						io::lcd.print("bck");

						state++;
						motion::left.speed(-SLOW_SPEED);
						motion::right.speed(-SLOW_SPEED);
						motion::left_theta = 0;
					}
					case ZERO_BACK:
					{
						if (motion::left_theta < ZERO_BACK_THETA)
						{
							state = ZERO_TURN_BEGIN;
						}
						break;
					}
					case ZERO_TURN_BEGIN:
					{
						io::lcd.setCursor(0, 1);
						io::lcd.print("trn");

						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::right.speed(-SLOW_SPEED);
						motion::left_theta = 0;
						// fall through
					}
					case ZERO_TURN:
					{
						if (motion::left_theta > ZERO_TURN_THETA)
						{
							state = ZERO_FWD_BEGIN;
						}
						break;
					}
					case ZERO_FWD_BEGIN:
					{
						io::lcd.setCursor(0, 1);
						io::lcd.print("fwd");
						state++;

						motion::left.speed(MEDIUM_SPEED);
						motion::right.speed(MEDIUM_SPEED);
						motion::left_theta = 0;
						// fall through
					}
					case ZERO_FWD:
					{
						if (motion::left_theta > ZERO_FWD_THETA)
						{
							control::set_mode(&side_retrieval_mode);
						}
						break;
					}
				}
			}
			else if (pet_id == 1)
			{
				switch (state)
				{
					case ONE_TURN_BEGIN:
					{
						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::right.halt();
						motion::left_theta = 0;
						// fall through
					}
					case ONE_TURN:
					{
						if (motion::left_theta > ONE_TURN_THETA)
						{
							state = ONE_FWD_BEGIN;
						}
						break;
					}
					case ONE_FWD_BEGIN:
					{
						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::right.speed(MEDIUM_SPEED);
						motion::left_theta = 0;
					}
					case ONE_FWD:
					{
						if (motion::left_theta > ONE_FWD_THETA)
						{
							control::set_mode(&side_retrieval_mode);
							return;
						}
						break;
					}
				}
			}
			else if (pet_id == 2)
			{
				switch (state)
				{
					case TWO_BACKUP_BEGIN:
					{
						state++;
						motion::left.speed(-MEDIUM_SPEED);
						motion::right.speed(-MEDIUM_SPEED);
						motion::left_theta = 0;
						// fall through
					}
					case TWO_BACKUP:
					{
						if (motion::left_theta < TWO_BACKUP_THETA)
						{
							control::set_mode(&side_retrieval_mode);
							return;
						}
						break;
					}
				}
			}
			else
			{
				control::set_mode(&side_retrieval_mode);
				return;
			}

			motion::update_enc();
			io::delay_ms(10);
		}

		void side_retrieval_begin()
		{
			if (pet_id == 0) // we don't use the arm
			{
				control::set_mode(&recover_mode);
			}

			motion::left.halt();
			motion::right.halt();

			io::lcd.clear();
			io::lcd.home();
			io::lcd.print(TO_FSTR(strings::retrieval));

			motion::update_enc(); // clear accumulator
			motion::arm_theta = 0;

			retry_count = 0;
			state = 0;
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
					io::lcd.setCursor(0, 1);
					io::lcd.print("drp");
					motion::arm.speed(-FAST_SPEED);
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
					io::lcd.setCursor(0, 1);
					io::lcd.print("brk");
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
					io::lcd.setCursor(0, 1);
					io::lcd.print("lft");
					motion::arm.speed(MEDIUM_SPEED);
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
					io::lcd.setCursor(0, 1);
					io::lcd.print("rty");
					retry_count++;
					state++;
					motion::arm.speed(-SLOW_SPEED);
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
					io::lcd.setCursor(0, 1);
					io::lcd.print("zro");

					motion::arm.speed(MEDIUM_SPEED);
					io::Timer::start();
					state++;
					// fall through
				}
				case ZERO:
				{
					if (io::Timer::time() > 500)
					{
						state = DONE_BEGIN;
					}
					break;
				}
				case DONE_BEGIN:
				{
					io::lcd.setCursor(0, 1);
					io::lcd.print("dne");

					motion::arm.halt();
					state++;
					// fall through
				}
				case DONE:
				{
					control::set_mode(&recover_mode);
					break;
				}
			}

			motion::update_enc();
			io::delay_ms(10);
		}

		void recover_begin()
		{
			io::lcd.clear();
			io::lcd.home();
			io::lcd.print("recover");
			state = 0;

			motion::left.halt();
			motion::right.halt();
		}

		void recover_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			if (pet_id == 0)
			{
				switch (state)
				{
					case ZERO_BACK_BEGIN:
					{
						state++;
						motion::right.speed(MEDIUM_SPEED);
						// fall through
					}
					case ZERO_BACK:
					{
						if (io::Analog::qrd_tape_left.read() > menu::flw_thresh_left.value())
						{
							control::set_mode(&follow_mode);
							return;
						}
						break;
					}
				}
			}
			else if (pet_id == 1)
			{
				switch (state)
				{
					case ONE_TURN_BEGIN:
					{
						state++;
						motion::left.speed(-MEDIUM_SPEED);
						motion::left_theta = 0;
						// fall through
					}
					case ONE_TURN:
					{
						if (io::Analog::qrd_tape_left.read() > menu::flw_thresh_left.value())
						{
							control::set_mode(&follow_mode);
							return;
						}
						break;
					}
				}
			}
			else
			{
				control::set_mode(&follow_mode);
				return;
			}

			motion::update_enc();
			io::delay_ms(10);
		}

		void recover_end()
		{
			pet_id++;
		}

		void beacon_homing_begin()
		{
			io::lcd.clear();
			io::lcd.home();
			io::lcd.print(TO_FSTR(strings::home));
			
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

			uint16_t left = io::Analog::pd_left.read();
			uint16_t right = io::Analog::pd_right.read();

			if ((left + right) / 2 > menu::beacon_thresh.value()) // close enough to the beacon
			{
				control::set_mode(&parallel_park_mode);
				return;
			}

			int16_t in = left - right;
			int16_t thresh = menu::home_thresh.value();

			controller.in(in - thresh);
			int16_t out = controller.out();

			motion::vel(menu::home_vel.value());
			motion::dir(out);
		}

		void parallel_park_begin()
		{
			io::lcd.clear();
			io::lcd.home();
			io::lcd.print(TO_FSTR(strings::ppark));

			state = 0;
		}

		void parallel_park_tick()
		{
			enum
			{
				ENTRY_BEGIN = 0,
				ENTRY,
				BACK_BEGIN,
				BACK,
				FORWARD_BEGIN,
				FORWARD
			};

			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			switch (state)
			{
				case ENTRY_BEGIN:
				{
					state++;
					motion::left.halt();
					motion::right.speed(MEDIUM_SPEED);
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
					motion::left.speed(-MEDIUM_SPEED);
					motion::right.speed(-SLOW_SPEED);
					motion::right_theta = 0;
					motion::left_theta = 0;
					// fall through
				}
				case BACK:
				{
					if ((motion::right_theta + motion::left_theta) / 2 < -100)
					{
						state = FORWARD_BEGIN;
					}
					break;
				}
				case FORWARD_BEGIN:
				{
					state++;
					motion::left.speed(SLOW_SPEED);
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

			io::delay_ms(10);
			motion::update_enc();
		}

		void rubble_excavation_begin()
		{
			state = 0;
		}

		void rubble_excavation_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}
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

	const control::Mode adjust_mode
	{
		&adjust_begin,
		&adjust_tick,
		&control::nop
	};

	const control::Mode recover_mode
	{
		&recover_begin,
		&recover_tick,
		&recover_end
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
