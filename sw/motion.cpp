#include "motion.h"
#include "isr.h"

namespace motion
{
	namespace
	{
		volatile uint8_t enc0_counts;
		volatile uint8_t enc1_counts;

		DIGITAL_OUTPUT(dir_0, B, 0);
		DIGITAL_OUTPUT(dir_1, B, 1);
		DIGITAL_OUTPUT(dir_2, E, 6);
		DIGITAL_OUTPUT(dir_3, E, 7);
		const io::Digital::Out* dirs[] = { &dir_0, &dir_1, &dir_2, &dir_3 };

		/*
		DIGITAL_OUTPUT(den_0, B, 3);
		DIGITAL_OUTPUT(den_1, B, 4);
		DIGITAL_OUTPUT(den_2, E, 4);
		DIGITAL_OUTPUT(den_3, E, 5);
		const io::Digital::Out* dens[] = { &den_0, &den_1, &den_2, &den_3 };
		*/

		const uint8_t digital_enables[] = { 29, 30, 36, 37 }; //digital input/output values

		const uint8_t enables[] = { 5, 4, 1, 0 }; // PWM output values

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
		dirs[_id]->write(true);
		pinMode(digital_enables[_id], OUTPUT);

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
			dirs[_id]->write(true);
			analogWrite(digital_enables[_id], abs);
		}
		else
		{
			dirs[_id]->write(false);
			analogWrite(digital_enables[_id], abs);
		}
	}
	void Motor::halt()
	{
		analogWrite(digital_enables[_id], 0);
	}

	uint32_t x;
	uint32_t y;
	uint16_t theta;

	void init()
	{
		isr::attach_pin(0, RISING);
		isr::attach_pin(1, RISING);
		isr::attach_timer3(100);
	}

	void enc0()
	{
		enc0_counts++;
	}
	void enc1()
	{
		enc1_counts++;
	}
	void update_100hz()
	{
		

		enc0_counts = 0;
		enc1_counts = 0;
	}

	void halt()
	{
		velocity = 0;
		direction = 0;
		left.halt();
		right.halt();
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
