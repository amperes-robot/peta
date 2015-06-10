#include "control.h"
#include "io.h"
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
			io::log<io::FATAL>("Null mode set");
		}

		current_mode->end();
		current_mode = mode;
		current_mode->begin();
	}

	void init()
	{
		current_mode = &idle_mode;
	}

	void loop()
	{
		current_mode->tick();
	}
}
