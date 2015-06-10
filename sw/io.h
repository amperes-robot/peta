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

#define N_ANALOG_PINS 8

extern LiquidCrystal LCD;
extern motorClass motor;

namespace io
{
	struct Digital final
	{
		Digital() = delete;
		Digital(int id);
		bool value() const;
	};

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

#ifdef IO_SIM
	void set_input(int pin, int value);
#endif

#ifdef IO_TST
	extern TestCallback test_suite[];
#endif
}
