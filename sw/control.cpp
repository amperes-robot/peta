#include "control.h"
#include "io.h"
#include "menu.h"
#include "isr.h"

namespace control
{
	namespace
	{
		const Mode* current_mode;

		void idle_mode_begin() { }
		void idle_mode_end() { }
		void idle_mode_tick() { }
	}

	const Mode idle_mode
	{
		idle_mode_begin,
		idle_mode_end,
		idle_mode_tick,
		nullptr
	};

	void set_mode(const Mode* mode)
	{
		if (!mode)
		{
			io::log<io::CRITICAL>("Null mode set");
			mode = &idle_mode;
		}

		current_mode->end();
		current_mode = mode;
		current_mode->begin();
	}

	void init()
	{
		current_mode = &idle_mode;
		set_mode(&menu::main_mode);
		isr::attach_adc();
	}

	void loop()
	{
		current_mode->tick();
		analogRead(0);
	}
}
