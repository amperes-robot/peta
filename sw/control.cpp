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

		void idle_mode_begin() { }
		void idle_mode_tick() { }
		void idle_mode_end() { }
	}

	const Mode idle_mode
	{
		idle_mode_begin,
		idle_mode_tick,
		idle_mode_end,
	};

	void set_mode(const Mode* mode)
	{
		if (!mode)
		{
			io::log("Null mode set!");
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

		// isr::attach_adc();

		set_mode(&menu::main_mode);
	}

	void loop()
	{
		current_mode->tick();
	}
}
