#include "menu.h"
#include "io.h"

namespace menu
{
	namespace
	{
		void main_mode_begin() { }
		void main_mode_end() { }
		void main_mode_tick() { }

		void options_mode_begin() { }
		void options_mode_end() { }
		void options_mode_tick() { }
	}

	const control::Mode main_mode
	{
		main_mode_begin,
		main_mode_end,
		main_mode_tick,
		nullptr
	};

	const control::Mode options_mode
	{
		options_mode_begin,
		options_mode_end,
		options_mode_tick,
		nullptr
	};
}
