#include "io.h"

#ifdef IO_SIM
	#include <queue>
	#include <string>
#endif

namespace io
{
	namespace
	{
		int pins[N_PINS] = { };
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

	void poll()
	{
		// read digital and analog from board
#ifndef IO_SIM
#endif
	}

#ifdef IO_SIM
	void set_input(int pin, int value)
	{
		if (pin >= N_PINS)
		{
			log<FATAL>("imposible pin");
		}

		pins[pin] = value;
	}
#endif
}
