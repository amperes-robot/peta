#include "motion.h"

extern motorClass motor;

namespace motion
{
	namespace
	{
		const uint8_t motor_left = 0;
		const uint8_t motor_right = 1;
		const bool reverse_left = true;
		const bool reverse_right = true;

		int16_t direction, velocity;

		void refresh()
		{
			motor.speed(motor_left, reverse_left ? -(velocity - direction) : velocity - direction);
			motor.speed(motor_right, reverse_right ? -(velocity + direction) : velocity + direction);
		}
	}

	void dir(int16_t x)
	{
		direction = x;
		refresh();
	}

	void vel(int16_t v)
	{
		velocity = v;
		refresh();
	}
}
