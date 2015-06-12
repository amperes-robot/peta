#pragma once

#include <Arduino.h>
#include <motor.h>
#include <phys253pins.h>
#include <ServoTimer2.h>
#include <LiquidCrystal.h>
#define LOG_THRESHOLD INFO

extern LiquidCrystal LCD;
extern motorClass motor;

#define N_ANALOG 8

namespace io
{
#ifdef IO_SIM
	typedef std::string string;
#else
	typedef String string;
#endif

	enum LogLevel
	{
		TRACE,
		INFO,
		CRITICAL,
		FATAL
	};

	template<LogLevel Level>
	inline void log(string msg)
	{
		if (Level >= LOG_THRESHOLD)
		{
#ifdef IO_SIM
			std::string level = 
				(Level == TRACE ? "TRACE" :
				(Level == INFO ? "INFO" :
				(Level == CRITICAL ? "CRITICAL" :
				(Level == FATAL ? "FATAL" : ""))));
			std::cout << "LCD " << level << " " << msg << std::endl;

			if (Level >= FATAL)
			{
				throw std::logic_error("Fatal message logged");
			}
#else
			LCD.clear();
			LCD.home();
			LCD.print(msg);
#endif
		}
	}

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

	void start_adc(uint8_t pin);

#ifdef IO_SIM
	void set_input(uint8_t pin, bool value);

	extern TestCallback test_suite[];
#endif
}
