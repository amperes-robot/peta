#pragma once

#ifdef IO_SIM
	#include <string>
	#include <sstream>
	#include <iostream>
	#include <stdexcept>
	#define LOG_THRESHOLD TRACE
#else
	#include <Arduino.h>
	#include <motor.h>
	#include <phys253pins.h>
	#include <ServoTimer2.h>
	#include <LiquidCrystal.h>
	#define LOG_THRESHOLD INFO
#endif

#ifdef IO_TST
	#include "test.h"
#endif

#define N_ANALOG 8

extern LiquidCrystal LCD;
extern motorClass motor;

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
#else
			/*
			LCD.clear();
			LCD.home();
			LCD.print(msg);
			*/
#endif
		}

#ifdef IO_SIM
		if (Level >= FATAL)
		{
			throw std::logic_error("Fatal message logged");
		}
#endif
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
#endif

#ifdef IO_TST
	extern TestCallback test_suite[];
#endif
}
