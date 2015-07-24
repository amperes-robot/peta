#include "course.h"
#include "isr.h"
#include "io.h"
#include "menu.h"
#include "motion.h"

namespace course
{
	namespace
	{
		uint8_t state;

		enum { ZERO_TURN_THETA = 30, ZERO_BACK_THETA = -70, ZERO_FWD_THETA = 70, ZERO_TURN2_THETA = 30,
			ONE_TURN_THETA = 5, ONE_FWD_THETA = 6, TWO_TURN_THETA = 10, TWO_BACKUP_THETA = -10 };
		enum { THREE_FWD_THETA = 20, THREE_TURN_THETA = 45 };
		enum { FIVE_REPOS_THETA = -40, FIVE_BACK_THETA = -100, FIVE_TURN_THETA = 200 };

		/**
		 * ADJUST (before dropping arm)
		 */
		enum
		{
			ZERO_BACK_BEGIN = 0,
			ZERO_BACK,
			ZERO_TURN_BEGIN,
			ZERO_TURN,
			ZERO_FWD_BEGIN,
			ZERO_FWD,
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
			TWO_BACKUP,
			TWO_TURN_BEGIN, // unused
			TWO_TURN,
		};
		enum
		{
			THREE_FWD_BEGIN = 0,
			THREE_FWD
		};
		enum
		{
			FIVE_REPOS_BEGIN = 0,
			FIVE_REPOS
		};
		enum
		{
			FIVE_BACK_BEGIN = 0,
			FIVE_BACK,
			FIVE_TURN_BEGIN,
			FIVE_TURN
		};

		void adjust_begin()
		{
			io::lcd.clear();
			io::lcd.home();
			io::lcd.print("adjust");

			state = 0;
			motion::left.halt();
			motion::right.halt();
		}
		void adjust_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			if (pet_id == 0) // this should really be a switch but whatever
			{
				switch (state)
				{
					case ZERO_BACK_BEGIN: // drive back
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
					case ZERO_TURN_BEGIN: // turn CW a bit
					{
						io::lcd.setCursor(0, 1);
						io::lcd.print("trn");

						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::right.speed(-MEDIUM_SPEED);
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
					case ZERO_FWD_BEGIN: // drive forward
					{
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
							control::set_mode(&recover_mode); // skip arm dropping and go to recovery directly
						}
						break;
					}
				}
			}
			else if (pet_id == 1)
			{
				switch (state)
				{
					case ONE_TURN_BEGIN: // turn to the right
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
					case ONE_FWD_BEGIN: // forward a bit to pick it up
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
							control::set_mode(&retrieve_mode);
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
					case TWO_TURN_BEGIN: // turn CW
					{
						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::right.halt();
						motion::left_theta = 0;
					}
					case TWO_TURN:
					{
						if (motion::left_theta > TWO_TURN_THETA)
						{
							state = TWO_BACKUP_BEGIN;
						}
						break;
					}
					case TWO_BACKUP_BEGIN: // back up
					{
						state++;
						motion::left.speed(-SLOW_SPEED);
						motion::right.speed(-SLOW_SPEED);
						motion::left_theta = 0;
						// fall through
					}
					case TWO_BACKUP:
					{
						if (motion::left_theta < TWO_BACKUP_THETA)
						{
							control::set_mode(&retrieve_mode);
							return;
						}
						break;
					}
				}
			}
			else if (pet_id == 3) // go forward a bit to catch the pet standing in the middle
			{
				switch (state)
				{
					case THREE_FWD_BEGIN:
					{
						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::right.speed(MEDIUM_SPEED);
						motion::left_theta = 0;
						// fall through
					}
					case THREE_FWD:
					{
						if (motion::left_theta > THREE_FWD_THETA)
						{
							// control::set_mode(&reverse_follow_mode);
							control::set_mode(&recover_mode);
							return;
						}
						break;
					}
				}
			}
			else if (pet_id == 5)
			{
				switch (state)
				{
					case FIVE_REPOS_BEGIN:
					{
						state++;
						motion::right.speed(-SLOW_SPEED);
						motion::left.halt();
						motion::right_theta = 0;
						// fall through
					}
					case FIVE_REPOS:
					{
						if (motion::right_theta < FIVE_REPOS_THETA)
						{
							control::set_mode(&rubble_excavation_mode);
						}
						break;
					}
				}
			}
			else // should not reach here
			{
				control::set_mode(&retrieve_mode);
				return;
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

		void recover_tick() // get back to the tape
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
					case ZERO_BACK_BEGIN: // turn CCW until the tape is found
					{
						state++;
						motion::left.speed(-MEDIUM_SPEED);
						motion::right.speed(MEDIUM_SPEED);
						motion::right_theta = 0;
						// fall through
					}
					case ZERO_BACK:
					{
						if (motion::right_theta > ZERO_TURN2_THETA)
						{
							state = ZERO_TURN_BEGIN;
						}
						break;
					}
					case ZERO_TURN_BEGIN:
					{
						state++;
						motion::left.speed(-MEDIUM_SPEED);
						motion::right.speed(MEDIUM_SPEED);
						motion::right_theta = 0;
					}
					case ZERO_TURN:
					{
						if (motion::right_theta > 40 && io::Analog::qrd_tape_left.read() > menu::flw_thresh_left.value())
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
				switch (state) // CCW until tape is found
				{
					case ONE_TURN_BEGIN:
					{
						state++;
						motion::left.speed(-MEDIUM_SPEED);
						motion::right.halt();
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
			else if (pet_id == 3)
			{
				switch (state)
				{
					case THREE_FWD_BEGIN:
					{
						motion::left.speed(MEDIUM_SPEED);
						motion::right.halt();
						motion::left_theta = 0;
						state++;
						// fall through
					}
					case THREE_FWD: // turn a bit until somewhat in line with beacon
					{
						io::lcd.clear();
						io::lcd.home();
						io::lcd.print(motion::left_theta);
						if (motion::left_theta > THREE_TURN_THETA)
						{
							control::set_mode(&beacon_homing_mode);
							return;
						}
						break;
					}
				}
			}
			else if (pet_id == 4) // change mode here instead of doing a real recovery
			{
				control::set_mode(menu::rev_enable.value() ? &reverse_follow_mode : &beacon_homing_mode);
				return;
			}
			else if (pet_id == 5)
			{
				switch (state)
				{
					case FIVE_BACK_BEGIN:
					{
						state++;
						motion::left.speed(-MEDIUM_SPEED);
						motion::right.speed(-MEDIUM_SPEED);
						motion::left_theta = 0;
						// fall through
					}
					case FIVE_BACK:
					{
						if (motion::left_theta < FIVE_BACK_THETA)
						{
							state = FIVE_TURN_BEGIN;
						}
						break;
					}
					case FIVE_TURN_BEGIN:
					{
						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::right.speed(-MEDIUM_SPEED);
						motion::left_theta = 0;
					}
					case FIVE_TURN:
					{
						if (motion::left_theta > FIVE_TURN_THETA)
						{
							control::set_mode(&beacon_homing_mode);
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
	}

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

}
