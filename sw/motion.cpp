#include "motion.h"

namespace motion
{
	namespace
	{
		const uint8_t dirs[] = { 24, 25, 38, 39 };
		const uint8_t digital_enables[] = { 29, 30, 36, 37 }; //digital input/output values
		const uint8_t enables[] = { 5,  4,  1,  0 }; // PWM output values

		const bool reverse_left = true;
		const bool reverse_right = true;

		int16_t direction, velocity;

		void refresh()
		{
			left.speed(reverse_left ? -(velocity - direction) : velocity - direction);
			right.speed(reverse_right ? -(velocity + direction) : velocity + direction);
		}
	}

	Motor left(0);
	Motor right(1);
	Motor retrieval(2);
	Motor zipline(3);

	Motor::Motor(uint8_t id) : _id(id)
	{
		pinMode(dirs[_id], OUTPUT);
		pinMode(digital_enables[_id], OUTPUT);
		digitalWrite(dirs[_id], HIGH);

		analogWrite(digital_enables[_id], 1); //make the motor glitch, force it to be zero.
		analogWrite(enables[_id], 0);
	}
	void Motor::speed(int16_t speed)
	{
		if (speed > 255) speed = 255;
		if (speed < -255) speed = -255;
		uint8_t abs = speed > 0 ? speed : -speed;

		if (speed >= 0)
		{
			digitalWrite(dirs[_id], HIGH);
			analogWrite(digital_enables[_id], abs);
			// digitalWrite(digital_enables[_id], HIGH);
		}
		else
		{
			digitalWrite(dirs[_id], LOW);
			analogWrite(digital_enables[_id], abs);
			// digitalWrite(digital_enables[_id], HIGH);
		}
	}

	void halt()
	{
		velocity = 0;
		direction = 0;
		refresh();
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
