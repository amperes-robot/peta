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
		using namespace async;

		pid::DigitalController dcontroller(0, 0, 0);

		enum MotorMasks : uint16_t
		{
			MOTOR_LEFT = 0x00,
			MOTOR_RIGHT = 0x10,
			MOTOR_ARM = 0x20,
			MOTOR_EXCAVATOR = 0x30,
			MOTOR_REVERSE = 0x80,


			MOTOR_MASK = 0x40,
			REVERSE_MASK = 0x80,
			SPEED_MASK = 0x0F,
		};

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

		uint8_t motor(uint8_t first, uint16_t meta)
		{
			int16_t speed = meta & SPEED_MASK;

			if (meta & REVERSE_MASK)
			{
				speed = -speed;
			}

			motion::motors[(meta & MOTOR_MASK) >> 4]->speed(speed);

			return 1;
		}

		void begin_tick()
		{
			begin();

			fork(&motor, Until(QRD_SIDE_GREATER_THAN, menu::flw_thresh_side.value()), MOTOR_REVERSE | MOTOR_ARM | 60);
			exec(&follow, Until(QRD_SIDE_GREATER_THAN, menu::flw_thresh_side.value()));

			end();

			control::set_mode(&async::async_mode);
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
