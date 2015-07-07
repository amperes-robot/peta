#include "motion.h"
#include "math.h"
#include "menu.h"
#include "isr.h"

namespace motion
{
	namespace
	{
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
		_dir = 1;
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
			_dir = 1;
			dirs[_id]->write(true);
			analogWrite(digital_enables[_id], abs);
		}
		else
		{
			_dir = -1;
			dirs[_id]->write(false);
			analogWrite(digital_enables[_id], abs);
		}
	}
	void Motor::halt()
	{
		analogWrite(digital_enables[_id], 0);
	}

	volatile uint8_t enc0_counts;
	volatile uint8_t enc1_counts;
	volatile uint8_t enc2_counts;

	volatile uint32_t x;
	volatile uint32_t y;
	volatile uint16_t theta;

	void init()
	{
		isr::attach_pin(0, RISING);
		isr::attach_pin(1, RISING);
		isr::attach_timer3(100);
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

ISR(INT0_vect)
{
	motion::enc0_counts++;
}
ISR(INT1_vect)
{
	motion::enc1_counts++;
}
ISR(INT2_vect) // 62.4 per rev
{
	motion::enc2_counts++;
}

ISR(TIMER3_COMPA_vect)
{
	int8_t enc0 = motion::enc0_counts * motion::left.dir();
	int8_t enc1 = motion::enc1_counts * motion::right.dir();
	uint16_t vv = (enc0 + enc1) / 2 * menu::dr_vscl.value(); // velocity
	uint16_t dth = (enc1 - enc0) * menu::dr_wheel_d.value(); // angular velocity

	motion::theta += dth;
	motion::x += math::cos(motion::theta) * vv / math::full;
	motion::y += math::sin(motion::theta) * vv / math::full;

	motion::enc0_counts = 0;
	motion::enc1_counts = 0;
}
