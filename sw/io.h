#pragma once

#ifdef IO_SIM
	#include <string>
	#include <sstream>
	#include <iostream>
	#include <stdexcept>
	#define LOG_THRESHOLD TRACE
#else
	#include <LiquidCrystal.h>
	#define LOG_THRESHOLD INFO
	typedef String string;
#endif

#define N_PINS 16

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
			//LCD.clear();
			//LCD.home();
			//LCD.print(msg);
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
	string to_string(T t);

#ifdef IO_SIM
	void set_input(int pin, int value);
#endif
}
