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

		enum Constants
		{
			ARM_LO_THRESH = -28,
			ARM_HI_THRESH = -5,
			ARM_MID_THRESH = -15,
			SQUARE_FWD_MAX = 50,
			SQUARE_BACK_MAX = -50
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
			DELAY_MASK = 0x0FFF
		};

		uint8_t increment_pet(uint8_t, uint16_t);
		uint8_t follow(uint8_t, uint16_t);
		uint8_t square(uint8_t, uint16_t);
		uint8_t halt(uint8_t, uint16_t);
		uint8_t motor(uint8_t, uint16_t);
		uint8_t retrieve(uint8_t, uint16_t);

		void begin_tick()
		{
			uint16_t side_thresh = menu::flw_thresh_side.value();
			uint16_t left_thresh = menu::flw_thresh_left.value();
			uint16_t right_thresh = menu::flw_thresh_right.value();

			begin();

			// PET 0
			exec(&follow, Until(EITHER_SIDE_QRD_GREATER_THAN, side_thresh));
			exec(&square, Until(FALSE));

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 300U);

			fork(&motor, Until(TRUE),                    MOTOR_REVERSE | MOTOR_RIGHT | 120U);
			exec(&motor, Until(LEFT_ENC_LESS_THAN, -45), MOTOR_REVERSE | MOTOR_LEFT | 120U);

			fork(&motor, Until(TRUE),                      MOTOR_REVERSE | MOTOR_RIGHT | 160U);
			exec(&motor, Until(LEFT_ENC_GREATER_THAN, 27), MOTOR_LEFT | 160U);

			fork(&motor, Until(TRUE),                      MOTOR_RIGHT | 180U);
			exec(&motor, Until(LEFT_ENC_GREATER_THAN, 70), MOTOR_LEFT | 180U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 300U);
			exec(&increment_pet, Until(TRUE));

			exec(&motor, Until(FRONT_LEFT_QRD_GREATER_THAN, left_thresh), MOTOR_LEFT | 100U);

			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 1000U);

			// PET 1

			exec(&follow, Until(EITHER_SIDE_QRD_GREATER_THAN, side_thresh));
			exec(&halt, Until(FALSE), MOTOR_LEFT_BIT | MOTOR_RIGHT_BIT | 1000U);

			exec(&square, Until(FALSE));

			end();

			control::set_mode(&async::async_mode);
		}
		
		uint8_t increment_pet(uint8_t first, uint16_t)
		{
			pet_id++;
			return 0;
		}

		uint8_t follow(uint8_t first, uint16_t)
		{
			if (first)
			{
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

			return 1;
		}

		uint8_t retrieve(uint8_t first, uint16_t)
		{
			static int retry_count;

			if (first)
			{
				motion::left.halt();
				motion::right.halt();
				motion::update_enc();
				motion::arm_theta = 0;
				state = 0;
				retry_count = 0;
			}

			int8_t drop_thresh;

			drop_thresh = pet_id < 4 ? ARM_LO_THRESH : ARM_MID_THRESH;

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

			switch (state) // dropping arm
			{
				case DROPPING_BEGIN: // drop the arm
				{
					motion::arm.speed(-MEDIUM_SPEED);
					io::Timer::start(); 
					state++;
					// fall through
				}
				case DROPPING:
				{
					if (motion::arm_theta < drop_thresh || io::Timer::time() > 1000 /*|| io::Digital::switch_upper.read()*/)
						// the arm is down or the microswitch has been activated or timeout
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
					if (io::Timer::time() > 400) state = LIFTING_BEGIN;
					break;
				}
				case LIFTING_BEGIN: // lift
				{
					motion::arm.speed(MEDIUM_SPEED - 15);
					state++;
					io::Timer::start();
					// fall through
				}
				case LIFTING:
				{
					if (motion::arm_theta > ARM_HI_THRESH /* || !io::Digital::switch_upper.read()*/)
						// wait until pet is detached or the arm is up
					{
						state = ZERO_BEGIN;
					}
					else if (io::Timer::time() > 2000) // timeout
					{
						state = retry_count < 2 ? RETRY_BEGIN : ZERO_BEGIN;
					}
					break;
				}
				case RETRY_BEGIN: // move down and try again
				{
					retry_count++;
					state++;
					motion::arm.speed(-SLOW_SPEED);
					// fall through
				}
				case RETRY:
				{
					if (motion::arm_theta < ARM_LO_THRESH) state = BRAKE_BEGIN;
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

				io::lcd.clear();
				io::lcd.home();
				io::lcd.print("square");

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
