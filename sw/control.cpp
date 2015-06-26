#include "control.h"
#include "io.h"
#include "menu.h"
#include "pid.h"
#include "motion.h"
#include "isr.h"

namespace control
{
	namespace
	{
		const volatile Mode* current_mode;
	}

	const Mode idle_mode
	{
		&nop,
		&nop,
		&nop,
	};

	void set_mode(const Mode* mode)
	{
		if (!mode)
		{
			mode = &idle_mode;
		}

		current_mode->end();
		current_mode = mode;
		current_mode->begin();
	}

	void init()
	{
		Serial.begin(9600);
		current_mode = &idle_mode;

		menu::init();
		io::init();
		motion::init();

		set_mode(&menu::main_mode);
	}

	void loop()
	{
		current_mode->tick();
	}
}
