#include "menu.h"
#include "pid.h"

namespace menu
{
	namespace
	{
		uint8_t get_index(uint8_t n)
		{
			return (uint16_t) io::analog_in(io::Analog::SELECT) * n / 1024;
		}

		String main_names[] =
		{
			"run",
			"pid",
			"opt"
		};
		const size_t main_count = sizeof(main_names) / sizeof(*main_names);
		int8_t prev_index;

		void main_mode_begin()
		{
			prev_index = -1;
			LCD.clear();
		}
		void main_mode_tick()
		{
			uint8_t index = get_index(main_count);
			if (prev_index != index)
			{
				LCD.clear();
				LCD.home();
				LCD.setCursor(0, 0);
				LCD.print("MENU");
				LCD.setCursor(0, 1);
				LCD.print(main_names[index]);
			}
			prev_index = index;

			if (!io::digital_in(io::Digital::START))
			{
				switch (index)
				{
					case 0:
						break;
					case 1:
						control::set_mode(&pid::follow_mode);
						break;
					case 2:
						control::set_mode(&menu::opt_mode);
						break;
				}
			}

			delay(80);
		}
		void main_mode_end()
		{
			LCD.clear();
		}

		Opt pid_gain_p("pid.p");
		Opt pid_gain_i("pid.i");
		Opt pid_gain_d("pid.d");

		Opt opts[] =
		{
			pid_gain_p,
			pid_gain_i,
			pid_gain_d
		};
		const size_t opts_count = sizeof(opts) / sizeof(*opts);

		int8_t opt_editing = 0;

		void opt_mode_begin()
		{
			LCD.clear();
			opt_editing = -1;
			prev_index = -1;
		}
		void opt_mode_tick()
		{
			uint8_t index = get_index(opts_count);
			uint16_t tweak = io::analog_in(io::Analog::TWEAK);

			if (prev_index != index)
			{
				LCD.clear();
				LCD.home();
				LCD.setCursor(0, 0);
				LCD.print("opt ");
				LCD.print(opts[index].name());
				LCD.setCursor(0, 1);

				if (opt_editing >= 0)
				{
					LCD.print("*");
					LCD.print(tweak);
				}
				else
				{
					LCD.print(opts[index].value());
				}
			}
			prev_index = index;

			if (!io::digital_in(io::Digital::START))
			{
				if (opt_editing >= 0)
				{
					opts[opt_editing].write(tweak);
					prev_index = -1;
					opt_editing = -1;
				}
				else
				{
					opt_editing = index;
				}
			}
			if (!io::digital_in(io::Digital::STOP))
			{
				if (opt_editing >= 0)
				{
					opt_editing = -1;
					prev_index = -1;
				}
				else
				{
					control::set_mode(&main_mode);
				}
			}

			delay(80);
		}
		void opt_mode_end()
		{
			LCD.clear();
		}
	}

	Opt::Opt(String name) : _name(name)
	{
		_addr_eep = (uint16_t*)(2 * opt_count++);
		_value = eeprom_read_word(_addr_eep);
	}

	uint16_t Opt::opt_count = 0;


	const control::Mode main_mode
	{
		main_mode_begin,
		main_mode_tick,
		main_mode_end
	};

	const control::Mode opt_mode
	{
		opt_mode_begin,
		opt_mode_tick,
		opt_mode_end
	};
}
