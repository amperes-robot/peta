#include "menu.h"
#include "course.h"
#include "motion.h"
#include "pid.h"

namespace menu
{
	namespace
	{
		uint8_t get_index(uint8_t n)
		{
			return (uint16_t) io::analog_in(io::Analog::SELECT) * n / 1024;
		}

		io::string main_names[] =
		{
			"course",
			"select",
			"opt",
			"opt-restore",
			"follow"
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
				LCD.write((uint8_t) 0);
				LCD.setCursor(0, 1);
				LCD.print(main_names[index]);
			}
			prev_index = index;

			if (start_falling())
			{
				switch (index)
				{
					case 0:
						control::set_mode(&course::begin_mode);
						break;
					case 1:
						break;
					case 2:
						control::set_mode(&menu::opt_mode);
						break;
					case 3:
						control::set_mode(&menu::opt_restore_mode);
						break;
					case 4:
						control::set_mode(&pid::follow_mode);
						break;
				}
			}

			delay(100);
		}
		void main_mode_end()
		{
			LCD.clear();
		}

		uint8_t restore_ticker;
		uint8_t restore_bar;
		Opt* opts[N_OPTS];
		int8_t opt_editing = 0;

		/**
		 * OPT_RESTORE_MODE
		 */

		void opt_restore_mode_begin()
		{
			LCD.clear();
			LCD.setCursor(0, 1);
			LCD.print("opt-restore");
			LCD.home();
			restore_ticker = 0;
			restore_bar = 0;
		}
		void opt_restore_mode_tick()
		{
			if (restore_ticker == restore_bar)
			{
				LCD.write((uint8_t) 0);

				if (restore_ticker < 0x80)
				{
					restore_bar += 8;
				}
			}

			if (restore_ticker < 0x80 && io::Digital::start.read())
			{
				control::set_mode(&main_mode);
			}
			else if (restore_ticker == 0x80)
			{
				for (uint8_t i = 0; i < Opt::opt_count; i++)
				{
					opts[i]->restore();
				}
				LCD.setCursor(12, 1);
				LCD.print("done");
			}
			else if (restore_ticker == 0xFF)
			{
				control::set_mode(&main_mode);
			}

			restore_ticker++;
			delay(6);
		}
		void opt_restore_mode_end()
		{
		}

		/**
		 * OPT_MODE
		 */

		void opt_mode_begin()
		{
			LCD.clear();
			opt_editing = -1;
			prev_index = -1;
		}
		void opt_mode_tick()
		{
			uint8_t index = get_index(Opt::opt_count);
			uint16_t tweak = io::analog_in(io::Analog::TWEAK);

			if (prev_index != index || opt_editing >= 0)
			{
				LCD.clear();
				LCD.home();
				LCD.setCursor(0, 0);
				LCD.print("opt ");

				if (opt_editing >= 0)
				{
					LCD.print(opts[opt_editing]->name());
					LCD.setCursor(0, 1);
					LCD.print("*");
					//LCD.print(tweak * opts[opt_editing]->scale());
					LCD.print(tweak);
				}
				else
				{
					LCD.print(opts[index]->name());
					LCD.setCursor(0, 1);
					//LCD.print(opts[index].value() * opts[index]->scale());
					LCD.print(opts[index]->value());
				}
			}
			prev_index = index;

			if (start_falling())
			{
				if (opt_editing >= 0)
				{
					opts[opt_editing]->write(tweak);
					opt_editing = -1;
				}
				else
				{
					opt_editing = index;
				}
				prev_index = -1;
			}
			if (stop_falling())
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

			delay(120);
		}
		void opt_mode_end()
		{
			LCD.clear();
		}
	}

	byte cross[8] =
	{
		B00100,
		B10110,
		B01101,
		B00110,
		B01100,
		B10110,
		B01101,
		B00100,
	};

	void init()
	{
		LCD.createChar(0, cross);
	}
	
	bool stop_falling()
	{
		static bool prev_state = false;
		bool state = !io::Digital::stop.read();
		bool ret = false;

		if (state && !prev_state)
		{
			ret = true;
		}

		prev_state = state;
		return ret;
	}

	bool start_falling()
	{
		static bool prev_state = false;
		bool state = !io::Digital::start.read();
		bool ret = false;

		if (state && !prev_state)
		{
			ret = true;
		}

		prev_state = state;
		return ret;
	}

	uint8_t Opt::opt_count = 0;

	Opt::Opt(io::string name, uint16_t def) : _addr_eep((uint16_t*) (2 * opt_count)), _name(name), _default(def)
	{
		opts[opt_count++] = this;
		_value = eeprom_read_word(_addr_eep);
	}

	Opt flw_gain_p("flw.p", 70);
	Opt flw_gain_i("flw.i", 1);
	Opt flw_gain_d("flw.d", 60);
	Opt flw_vel("flw.vel", 100);
	Opt flw_thresh("flw.thresh", 360);

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

	const control::Mode opt_restore_mode
	{
		opt_restore_mode_begin,
		opt_restore_mode_tick,
		opt_restore_mode_end
	};
}
