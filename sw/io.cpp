#include "io.h"

#ifdef IO_SIM
	#include <string>
#endif

namespace io
{
	namespace
	{
		uint16_t _digital_pins = { };
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

#ifdef IO_TST
	static void test_test()
	{
		ASSERT(0 == 0);
	}

	TestCallback test_suite[] =
	{
		SUITE_ENTRY(test_test),
		SUITE_END()
	};
#endif
}
