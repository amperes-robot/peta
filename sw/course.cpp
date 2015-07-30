#include "course.h"
#include "isr.h"
#include "io.h"
#include "menu.h"
#include "motion.h"
#include "pid.h"
#include "async.h"

namespace course
{
	namespace
	{
		uint8_t state;
		using namespace async;

		pid::DigitalController dcontroller(0, 0, 0);
		pid::Controller acontroller(0, 0, 0);

		enum Constants
		{
			ARM_LO_THRESH = -28,
			ARM_HI_THRESH = -5,
			ARM_MID_THRESH = -15,
			SQUARE_FWD_MAX = 50,
			SQUARE_BACK_MAX = -50,
			RETRY_SHIFT_THETA = 8
		};

		enum Speeds
		{
			SLOW_SPEED = 120,
			MILD_SPEED = 150,
			MEDIUM_SPEED = 180,
			FAST_SPEED = 210,
			LUDICROUS_SPEED = 255
		};

		enum MotorMasks : uint16_t
		{
			MOTOR_LEFT_BIT = 1U << 12,
			MOTOR_RIGHT_BIT = 1U << 13,
			MOTOR_ARM_BIT = 1U << 14,
			MOTOR_EXCAVATOR_BIT = 1U << 15,

			MOTOR_LEFT = 0x000,
			MOTOR_RIGHT = 0x100,
			MOTOR_ARM = 0x200,
			MOTOR_EXCAVATOR = 0x300,
			MOTOR_MASK = 0x300,

			MOTOR_REVERSE = 0x800,

			SPEED_MASK = 0xFF,
		};
		
		enum MiscMasks : uint16_t
		{
			DELAY_MASK = 0x0FFF,
			FOLLOW_IGNORE_SIDES = 1U << 15,
			BEACON_REVERSE = 1U << 15,
			ELEVATED_PET = 1U << 0,
			NONE = 0U
		};

		uint8_t increment_pet(uint8_t, uint16_t);
		uint8_t follow(uint8_t, uint16_t);
		uint8_t square(uint8_t, uint16_t);
		uint8_t halt(uint8_t, uint16_t);
		uint8_t motor(uint8_t, uint16_t);
		uint8_t retrieve(uint8_t, uint16_t);
		uint8_t beacon(uint8_t, uint16_t);

		void begin_tick()
		{
			pet_id = 0;

			uint16_t side_thresh = menu::flw_thresh_side.value();
			uint16_t left_thresh = menu::flw_thresh_left.value();
			uint16_t right_thresh = menu::flw_thresh_right.value();

			begin();

			// PET 0
			exec(&follow, Until(FALSE), 0U);
			exec(&square, Until(FALSE));
			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			fork(&motor, Until(TRUE),          MOTOR_REVERSE | MOTOR_RIGHT | 140U);
			exec(&motor, Until(L_ENC_LT, -45), MOTOR_REVERSE | MOTOR_LEFT | 140U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			fork(&motor, Until(TRUE),         MOTOR_REVERSE | MOTOR_RIGHT | 160U);
			exec(&motor, Until(L_ENC_GT, 27), MOTOR_LEFT | 160U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			fork(&motor, Until(TRUE),         MOTOR_RIGHT | 180U);
			exec(&motor, Until(L_ENC_GT, 70), MOTOR_LEFT | 180U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);
			exec(&increment_pet, Until(TRUE));

			exec(&motor, Until(FRONT_LEFT_QRD_GT, left_thresh), MOTOR_RIGHT | 180U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			// PET 1

			exec(&follow, Until(FALSE), 500U);
			exec(&square, Until(FALSE));
			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			fork(&motor, Until(TRUE),         MOTOR_RIGHT | 100U);
			exec(&motor, Until(L_ENC_GT, 10), MOTOR_LEFT | 160U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			exec(&retrieve, Until(FALSE));
			exec(&increment_pet, Until(TRUE));

			exec(&motor, Until(FRONT_LEFT_QRD_GT, left_thresh), MOTOR_REVERSE | MOTOR_LEFT | 180U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			// PET 2

			exec(&follow, Until(FALSE), 2000U);
			exec(&square, Until(FALSE));
			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			fork(&motor, Until(TRUE),         MOTOR_REVERSE | MOTOR_RIGHT | 160U);
			exec(&motor, Until(L_ENC_LT, -3), MOTOR_REVERSE | MOTOR_LEFT | 160U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			exec(&retrieve, Until(FALSE));
			exec(&increment_pet, Until(TRUE));

			exec(&motor, Until(FRONT_LEFT_QRD_GT, left_thresh), MOTOR_LEFT | 180U);
			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			// PET 3

			exec(&follow, Until(L_MINUS_R_ENC_GT, 75), FOLLOW_IGNORE_SIDES | 0U);
			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			fork(&motor, Until(TRUE),         MOTOR_RIGHT | 120U);
			exec(&motor, Until(L_ENC_GT, 80), MOTOR_LEFT | 180U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			exec(&motor, Until(FRONT_RIGHT_QRD_GT, right_thresh), MOTOR_LEFT | 180U);

			exec(&follow, Until(FALSE), 500U);
			//exec(&square, Until(FALSE));
			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			fork(&motor, Until(TRUE),         MOTOR_RIGHT | 150U);
			exec(&motor, Until(L_ENC_GT, 20), MOTOR_LEFT | 150U);

			// PET 4

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 150U);

			exec(&motor, Until(L_ENC_GT, 20), MOTOR_LEFT | 120U);

			exec(&beacon, Until(L_PLUS_R_ENC_GT, 300));
			exec(&retrieve, Until(FALSE), ELEVATED_PET);

			end();

			control::set_mode(&async::async_mode);
		}

		uint8_t increment_pet(uint8_t first, uint16_t)
		{
			pet_id++;
			return 0;
		}

		uint8_t follow(uint8_t first, uint16_t meta)
		{
			if (first)
			{
				io::Timer::start();
				dcontroller.reset();
				dcontroller.gain_p = menu::flw_gain_p.value();
				dcontroller.gain_i = menu::flw_gain_i.value();
				dcontroller.gain_d = menu::flw_gain_d.value();
			}

			int8_t in = pid::follow_value_digital();

			dcontroller.in(in);
			int16_t out = dcontroller.out();

			motion::vel(menu::flw_vel.value());
			motion::dir(out);

			uint16_t thresh = menu::flw_thresh_side.value();

			if (meta & FOLLOW_IGNORE_SIDES)
			{
				return 1;
			}
			else
			{
				uint8_t timer_elapsed = io::Timer::time() > (meta & DELAY_MASK);
				uint8_t side_detected = io::Analog::qrd_side_left.read() > thresh ||
				                        io::Analog::qrd_side_right.read() > thresh;

				return !(side_detected && timer_elapsed);
			}
		}

		uint8_t beacon(uint8_t first, uint16_t meta)
		{
			if (first)
			{
				acontroller.reset();
				acontroller.gain_p = menu::home_gain_p.value();
				acontroller.gain_i = menu::home_gain_i.value();
				acontroller.gain_d = menu::home_gain_d.value();

				motion::update_enc();
				motion::left_theta = 0;
				motion::right_theta = 0;
			}

			uint16_t left = io::Analog::pd_left.read();
			uint16_t right = io::Analog::pd_right.read();

			int16_t in = ((int32_t) right - left) * 80 / (left + right);

			acontroller.in(in);
			int16_t out = acontroller.out();

			if (meta & BEACON_REVERSE)
			{
				motion::vel(-((int16_t) menu::home_vel.value()));
				motion::dir(out);
			}
			else
			{
				motion::vel(menu::home_vel.value());
				motion::dir(out);
			}
		}

		uint8_t retrieve(uint8_t first, uint16_t meta)
		{
			static uint8_t retry_count;

			if (first)
			{
				motion::left.halt();
				motion::right.halt();

				motion::update_enc(); // clear accumulator
				motion::arm_theta = 0;

				retry_count = 0;
				state = 0;
			}

			int8_t drop_thresh = (meta & ELEVATED_PET) ? ARM_MID_THRESH : ARM_LO_THRESH;

			enum { N_RETRIES = 1 };
			enum
			{
				DROPPING_BEGIN = 0,
				DROPPING,
				BRAKE_BEGIN,
				BRAKE,
				LIFTING_BEGIN,
				LIFTING,
				RETRY_SHIFT_BEGIN,
				RETRY_SHIFT,
				ZERO_BEGIN,
				ZERO,
				SHIFT_BACK_BEGIN,
				SHIFT_BACK,
				DONE_BEGIN,
				DONE
			};

			switch (state) // dropping arm
			{
				case DROPPING_BEGIN: // drop the arm
				{
					motion::arm.speed(-MEDIUM_SPEED);
					motion::left.halt();
					motion::right.halt();
					io::Timer::start();
					state++;
					// fall through
				}
				case DROPPING:
				{
					if (motion::arm_theta < drop_thresh || io::Timer::time() > 1000)
						// the arm is down or timeout
					{
						state = BRAKE_BEGIN;
					}
					break;
				}
				case BRAKE_BEGIN: // brake
					{
						motion::arm.halt();
						state++;
						io::Timer::start();
						// fall through
					}
				case BRAKE: // wait for arm to slow to halt
				{
					if (io::Timer::time() > 400)
					{
						state = LIFTING_BEGIN;
					}
					break;
				}
				case LIFTING_BEGIN: // lift
				{
					motion::arm.speed(MEDIUM_SPEED);
					state++;
					io::Timer::start();
					// fall through
				}
				case LIFTING:
				{
					if (motion::arm_theta >= ARM_HI_THRESH )
						// wait until the arm is up
					{
						if (!io::Digital::switch_arm.read() && retry_count < N_RETRIES) // pet is off
						{
							state = RETRY_SHIFT_BEGIN;
						}
						else
						{
							state = ZERO_BEGIN;
						}
					}
					break;
				}
				case RETRY_SHIFT_BEGIN: // move down and try again
				{
					retry_count++;

					state = RETRY_SHIFT;
					if (meta & ELEVATED_PET)
					{
						motion::right.speed(-MEDIUM_SPEED);
						motion::left.speed(-MEDIUM_SPEED);
					}
					else
					{
						motion::left.speed(-MEDIUM_SPEED);
					}
					motion::left_theta = 0;
					// fall through
				}

				case RETRY_SHIFT:
				{
					if (motion::left_theta <= -RETRY_SHIFT_THETA)
					{
						state = DROPPING_BEGIN;
					}
					break;
				}

				case ZERO_BEGIN: // zero the arm by ramming it into the hardstop
				{
					motion::arm.speed(MEDIUM_SPEED);
					io::Timer::start();
					state++;
					// fall through
				}

				case ZERO:
				{
					if (io::Timer::time() > 500)
					{
						if (retry_count > 0)
						{
							state = SHIFT_BACK_BEGIN;
						}
						else
						{
							state = DONE_BEGIN;
						}
					}
					break;
				}

				case SHIFT_BACK_BEGIN: // move down and try again
				{
					retry_count++;

					if (meta & ELEVATED_PET)
					{
						state = DONE_BEGIN;
					}
					else
					{
						state++;
						motion::left.speed(MEDIUM_SPEED);
						motion::left_theta = 0;
					}

					// fall through
				}
				case SHIFT_BACK:
				{
					if (motion::left_theta >= retry_count * RETRY_SHIFT_THETA)
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
					return 0;
				}
			}
			
			return 1;
		}

		uint8_t motor(uint8_t, uint16_t meta)
		{
			int16_t speed = meta & SPEED_MASK;

			if (meta & MOTOR_REVERSE)
			{
				speed = -speed;
			}

			motion::motors[(meta & MOTOR_MASK) >> 8]->speed(speed);

			return 1;
		}

		// should only be called from main thread
		uint8_t halt(uint8_t first, uint16_t meta)
		{
			if (first)
			{
				io::Timer::start();
			}

			for (uint8_t i = 12; i < 16; i++)
			{
				if (meta & (1U << i))
				{
					motion::motors[i - 12]->halt();
				}
			}

			return io::Timer::time() < (meta & DELAY_MASK);
		}

		uint8_t square(uint8_t first, uint16_t)
		{
			if (first)
			{
				motion::left.halt();
				motion::right.halt();

				state = 0;
			}

			enum
			{
				BEGIN = 0,
				LEFT_FORWARD_BEGIN,
				LEFT_FORWARD,
				LEFT_BACKWARD_BEGIN,
				LEFT_BACKWARD,
				RIGHT_FORWARD_BEGIN,
				RIGHT_FORWARD,
				RIGHT_BACKWARD_BEGIN,
				RIGHT_BACKWARD
			};

			bool qrd_left = io::Analog::qrd_side_left.read() > menu::flw_thresh_side.value();
			bool qrd_right = io::Analog::qrd_side_right.read() > menu::flw_thresh_side.value();

			switch (state)
			{
				case BEGIN:
				{
					if (!qrd_left) state = LEFT_FORWARD_BEGIN;
					else if (!qrd_right) state = RIGHT_FORWARD_BEGIN;
					else return 0;

					break;
				}
				case LEFT_FORWARD_BEGIN:
				{
					state++;
					motion::left.speed(MEDIUM_SPEED);
					motion::left_theta = 0;
				}
				case LEFT_FORWARD:
				{
					if (qrd_left) return 0;
					if (motion::left_theta > SQUARE_FWD_MAX) state++;

					break;
				}
				case LEFT_BACKWARD_BEGIN: 
				{
					state++;
					motion::left.speed(-MEDIUM_SPEED);
					motion::left_theta = 0;
				}
				case LEFT_BACKWARD:
				{
					if (qrd_left || motion::left_theta < SQUARE_BACK_MAX) return 0;

					break;
				}
				case RIGHT_FORWARD_BEGIN: 
				{
					state++;
					motion::right.speed(MEDIUM_SPEED);
					motion::right_theta = 0;
				}
				case RIGHT_FORWARD:
				{
					if (qrd_right) return 0;
					if (motion::right_theta > SQUARE_FWD_MAX) state++;

					break;
				}
				case RIGHT_BACKWARD_BEGIN: 
				{
					state++;
					motion::right.speed(-MEDIUM_SPEED);
					motion::right_theta = 0;
				}
				case RIGHT_BACKWARD:
				{
					if (qrd_right || motion::right_theta < SQUARE_BACK_MAX) return 0;
					break;
				}
			}
			return 1;
		}
	}

	uint8_t pet_id;

	const control::Mode begin_mode
	{
		&control::nop,
		&begin_tick,
		&control::nop
	};
}
