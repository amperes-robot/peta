#include "motion.h"
#include "math.h"
#include "menu.h"
#include "isr.h"

namespace motion
{
	volatile uint8_t enc0_counts;
	volatile uint8_t enc1_counts;
	volatile uint8_t enc2_counts;
		
	namespace
	{
		DIGITAL_OUTPUT(dir_0, B, 0);
		DIGITAL_OUTPUT(dir_1, B, 1);
		DIGITAL_OUTPUT(dir_2, E, 6);
		DIGITAL_OUTPUT(dir_3, E, 7);
		const io::Digital::Out* dirs[] = { &dir_0, &dir_1, &dir_2, &dir_3 };

		const uint8_t digital_enables[] = { 29, 30, 36, 37 }; //digital input/output values

		const uint8_t enables[] = { 5, 4, 1, 0 }; // PWM output values

		int16_t direction, velocity;

		void refresh()
		{
			left.speed(velocity - direction);
			right.speed(velocity + direction);
		}
	}

	volatile int16_t left_theta;
	volatile int16_t right_theta;
	volatile int16_t arm_theta;

	Motor left(0, 0);
	Motor right(1, 1);
	Motor arm(2, 0);
	Motor excavator(3, 0);

	Motor* const motors[4] = { &left, &right, &arm, &excavator };

	Motor::Motor(uint8_t id, uint8_t reverse) : _id(id), _rev(reverse)
	{
		_speed = 0;
		dirs[_id]->write(true);
		pinMode(digital_enables[_id], OUTPUT);

		analogWrite(digital_enables[_id], 1); //make the motor glitch, force it to be zero.
		analogWrite(enables[_id], 0);
	}
	int16_t Motor::speed(int16_t speed)
	{
		if (speed == 0)
		{
			halt();
			return 0;
		}

		if (speed > 255) speed = 255;
		if (speed < -255) speed = -255;
		uint8_t abs = speed > 0 ? speed : -speed;

		if (speed >= 0)
		{
			dirs[_id]->write(!_rev);
			analogWrite(digital_enables[_id], abs);
		}
		else
		{
			dirs[_id]->write(_rev);
			analogWrite(digital_enables[_id], abs);
		}

		update_enc();
		return _speed = speed;
	}
	void Motor::halt()
	{
		analogWrite(digital_enables[_id], 0);
	}

	volatile uint32_t x;
	volatile uint32_t y;
	volatile uint16_t theta;
	volatile int32_t raw_theta;

	void init()
	{
		isr::attach_pin(0, RISING);
		isr::attach_pin(1, RISING);
		isr::attach_pin(2, RISING);
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

	void update_enc()
	{
		int16_t left_ticks = enc0_counts * left.dir();
		int16_t right_ticks = enc1_counts * right.dir();
		left_theta += left_ticks;
		right_theta += right_ticks;
		arm_theta += enc2_counts * arm.dir();

		// int8_t enc0 = enc0_counts * left.dir();
		// int8_t enc1 = enc1_counts * right.dir();

		// uint16_t vv = (enc0 + enc1) / 2 * menu::dr_vscl.value(); // velocity
		// uint16_t dth = (enc1 - enc0) * menu::dr_wheel_d.value(); // angular velocity

		// theta += dth;
		// x += math::cos(theta) * vv / math::full;
		// y += math::sin(theta) * vv / math::full;

		raw_theta += right_ticks - left_ticks;

		enc0_counts = 0;
		enc1_counts = 0;
		enc2_counts = 0;
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
ISR(INT2_vect)
{
	/*
	io::lcd.clear();
	io::lcd.home();
	io::lcd.print(motion::enc2_counts);
	*/
	motion::enc2_counts++;
}
