#include "control.h"
#include "io.h"
#include "menu.h"
#include "pid.h"
#include "isr.h"

namespace control
{
	namespace
	{
		const Mode* current_mode;
	}

	const Mode idle_mode
	{
		&nop_void,
		&nop,
		&nop,
	};

	void set_mode(const Mode* mode, void* args)
	{
		if (!mode)
		{
			io::log("Null mode set!");
			mode = &idle_mode;
		}

		current_mode->end();
		current_mode = mode;
		current_mode->begin(args);
	}

	void init()
	{
		Serial.begin(9600);
		current_mode = &idle_mode;
		menu::init();
		io::init();

		set_mode(&menu::main_mode);
	}

	void loop()
	{
		current_mode->tick();
	}
}
