#include "control.h"

namespace control
{
	namespace
	{
		Mode* current_mode;
	}

	void IdleMode::begin() { }
	void IdleMode::tick() { }
	void IdleMode::end() { }

	void init()
	{
		current_mode = new IdleMode();
	}

	void loop()
	{
		current_mode->tick();
	}
}
