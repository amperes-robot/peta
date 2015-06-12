#pragma once
#include <Arduino.h>
#include <motor.h>
#include <phys253pins.h>
#include <ServoTimer2.h>
#include <LiquidCrystal.h>

extern LiquidCrystal LCD;

#define N_ANALOG 8

namespace io
{
#ifdef IO_SIM
	typedef std::string string;
#else
	typedef String string;
#endif

	template<typename T>
	string to_string(T t)
	{
#ifdef IO_SIM
		std::ostringstream os;
		os << t;
		return os.str();
#else
		return string(t);
#endif
	}

	template<typename T>
	inline void log(T msg)
	{
#ifdef IO_SIM
		std::cout << msg << std::endl;
#else
		LCD.clear();
		LCD.home();
		LCD.print(to_string(msg));
#endif
	}

	namespace Analog
	{
		enum Analog_t : uint8_t
		{
			FOLLOW = 0,
			TWEAK = 6,
			SELECT = 7
		};
	}

	namespace Digital
	{
		enum Digital_t : uint8_t
		{
			STOP = 49,
			START = 50
		};
	}

	bool digital_in(uint8_t pin);
	void digital_out(uint8_t pin, bool value);

	uint16_t analog_in(uint8_t pin);
	void start_adc(uint8_t pin);

#ifdef IO_SIM
	void set_input(uint8_t pin, bool value);

	extern TestCallback test_suite[];
#endif
}
