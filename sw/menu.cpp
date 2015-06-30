#include "menu.h"
#include "math.h"
#include "course.h"
#include "strings.h"
#include "motion.h"
#include "pid.h"

namespace menu
{
	namespace
	{
		uint8_t get_index(uint8_t n)
		{
			return io::Analog::select.read() * n / 1024;
		}

		FSTR main_names[] =
		{
			TO_FSTR(strings::course),
			TO_FSTR(strings::select),
			TO_FSTR(strings::opt),
			TO_FSTR(strings::opt_restore),
			TO_FSTR(strings::follow),
			TO_FSTR(strings::dbg),
		};
		const size_t main_count = sizeof(main_names) / sizeof(*main_names);
		int8_t prev_index;

		void main_mode_begin()
		{
			prev_index = -1;
			io::lcd.clear();
			motion::left.halt();
			motion::right.halt();
			motion::zipline.halt();
			motion::retrieval.halt();
		}
		void main_mode_tick()
		{
			uint8_t index = get_index(main_count);
			if (prev_index != index)
			{
				io::lcd.clear();
				io::lcd.home();
				io::lcd.setCursor(0, 0);
				io::lcd.write('x');
				io::lcd.setCursor(0, 1);
				io::lcd.print(TO_FSTR(main_names[index]));
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
						control::set_mode(&opt_mode);
						break;
					case 3:
						control::set_mode(&opt_restore_mode);
						break;
					case 4:
						control::set_mode(&pid::follow_mode);
						break;
					case 5:
						control::set_mode(&dbg_mode);
				}
			}

			io::delay_ms(100);
		}
		void main_mode_end()
		{
			io::lcd.clear();
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
			io::lcd.clear();
			io::lcd.setCursor(0, 1);
			io::lcd.print(TO_FSTR(strings::opt_restore));
			io::lcd.home();
			restore_ticker = 0;
			restore_bar = 0;
		}
		void opt_restore_mode_tick()
		{
			if (restore_ticker == restore_bar)
			{
				io::lcd.write('X');

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
				io::lcd.setCursor(12, 1);
				io::lcd.print("done");
			}
			else if (restore_ticker == 0xFF)
			{
				control::set_mode(&main_mode);
			}

			restore_ticker++;
			io::delay_ms(6);
		}
		void opt_restore_mode_end()
		{
		}

		/**
		 * OPT_MODE
		 */

		void opt_mode_begin()
		{
			io::lcd.clear();
			opt_editing = -1;
			prev_index = -1;
		}
		void opt_mode_tick()
		{
			uint8_t index = get_index(Opt::opt_count);
			uint16_t tweak = io::Analog::tweak.read();

			if (prev_index != index || opt_editing >= 0)
			{
				io::lcd.clear();
				io::lcd.home();
				io::lcd.setCursor(0, 0);
				io::lcd.print("opt ");

				if (opt_editing >= 0)
				{
					io::lcd.print(opts[opt_editing]->name());
					io::lcd.setCursor(0, 1);
					io::lcd.print("*");
					//io::lcd.print(tweak * opts[opt_editing]->scale());
					io::lcd.print(tweak);
				}
				else
				{
					io::lcd.print(opts[index]->name());
					io::lcd.setCursor(0, 1);
					//io::lcd.print(opts[index].value() * opts[index]->scale());
					io::lcd.print(opts[index]->value());
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

			io::delay_ms(120);
		}
		void opt_mode_end()
		{
			io::lcd.clear();
		}

		void dbg_mode_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
			}
			static uint16_t theta = 0;

			io::lcd.home();
			io::lcd.clear();
			io::lcd.print(theta);
			io::lcd.setCursor(0, 1);
			io::lcd.print(math::sin(theta));
			theta += 100;
			delay(1);
		}
	}

	void init()
	{
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

	Opt::Opt(FSTR name, uint16_t def) : _addr_eep((uint16_t*) (2 * opt_count)), _name(name), _default(def)
	{
		opts[opt_count++] = this;
		_value = eeprom_read_word(_addr_eep);
	}

	Opt dr_wheel_d(TO_FSTR(strings::dr_d), 150 /* wheel dist (mm) */ / (56 /* wheel diam (mm) */ * 3.14159 / 24));
	Opt dr_vscl(TO_FSTR(strings::dr_vscl), 3); // velocity scale factor

	Opt flw_gain_p(TO_FSTR(strings::flw_p), 70);
	Opt flw_gain_i(TO_FSTR(strings::flw_i), 1);
	Opt flw_gain_d(TO_FSTR(strings::flw_d), 60);
	Opt flw_vel(TO_FSTR(strings::flw_vel), 100);
	Opt flw_thresh(TO_FSTR(strings::flw_thresh), 360);
	Opt flw_mark_lat(TO_FSTR(strings::flw_mark_lat), 10);
	Opt home_gain_p(TO_FSTR(strings::home_p), 70);
	Opt home_gain_i(TO_FSTR(strings::home_i), 1);
	Opt home_gain_d(TO_FSTR(strings::home_d), 60);
	Opt home_thresh(TO_FSTR(strings::home_thresh), 60);
	Opt home_vel(TO_FSTR(strings::home_vel), 100);

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

	const control::Mode dbg_mode
	{
		control::nop,
		dbg_mode_tick,
		control::nop
	};
}
