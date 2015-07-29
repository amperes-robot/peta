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
			TO_FSTR(strings::view)
		};
		const size_t main_count = sizeof(main_names) / sizeof(*main_names);
		int8_t prev_index;

		void main_mode_begin()
		{
			prev_index = -1;
			io::lcd.clear();
			motion::dir(0);
			motion::vel(0);
			motion::left.halt();
			motion::right.halt();
			motion::excavator.halt();
			motion::arm.halt();
		}

		void main_mode_tick()
		{
			static uint8_t prev = 1, now = 0;
			uint8_t temp;

			temp = prev;
			prev = now;

			if (now == 0 && temp == 1) now = 1;
			else if (now == 15 && temp == 14) now = 14;
			else now = now + now - temp;

			io::lcd.setCursor(prev, 0);
			io::lcd.write(' ');
			io::lcd.setCursor(now, 0);
			io::lcd.write('x');

			uint8_t index = get_index(main_count);
			if (prev_index != index)
			{
				io::lcd.setCursor(0, 1);
				io::lcd.print(TO_FSTR(main_names[index]));
				io::lcd.print("                ");
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
						break;
					case 6:
						control::set_mode(&view_mode);
						break;
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
				return;
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
				return;
			}

			restore_ticker++;
			io::delay_ms(6);
		}

		/**
		 * VIEW_MODE
		 */

		void view_mode_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			uint8_t index = get_index(24);
			io::lcd.home();
			io::lcd.clear();
			io::lcd.print(TO_FSTR(strings::view));
			io::lcd.setCursor(0, 1);

			if (index < 8)
			{
				io::lcd.print('D');
				io::lcd.print(index);
				io::lcd.print(' ');
				io::lcd.print(PIND & (1 << index));
			}
			else if (index < 16)
			{
				io::lcd.print('C');
				io::lcd.print(index - 8);
				io::lcd.print(' ');
				io::lcd.print(PINC & (1 << (index - 8)));
			}
			else
			{
				io::lcd.print('F');
				io::lcd.print(index - 16);
				io::lcd.print(' ');
				io::lcd.print(io::analog_pins[index - 16]);
			}

			io::delay_ms(100);
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
					io::lcd.print('*');
					io::lcd.print(tweak / opts[opt_editing]->scale());
				}
				else
				{
					io::lcd.print(opts[index]->name());
					io::lcd.setCursor(0, 1);
					io::lcd.print(opts[index]->value());
				}
			}
			prev_index = index;

			if (start_falling())
			{
				if (opt_editing >= 0)
				{
					opts[opt_editing]->write(tweak / opts[opt_editing]->scale());
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
					return;
				}
			}

			io::delay_ms(120);
		}
		void opt_mode_end()
		{
			io::lcd.clear();
		}
		
		void dbg_mode_begin()
		{
		}

		void dbg_mode_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}
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

	Opt::Opt(FSTR name, uint16_t def, uint16_t scale) : _scale(scale), _addr_eep((uint16_t*) (2 * opt_count)), _name(name), _default(def)
	{
		opts[opt_count++] = this;
		_value = eeprom_read_word(_addr_eep);
	}

	/*
	Opt dr_wheel_d(TO_FSTR(strings::dr_d), 150 / (56 wheel diam (mm) * 3.14159 / 24));
	Opt dr_vscl(TO_FSTR(strings::dr_vscl), 3, 10); // velocity scale factor
	*/

	Opt flw_gain_p(TO_FSTR(strings::flw_p), 900, 1);
	Opt flw_gain_i(TO_FSTR(strings::flw_i), 0, 5);
	Opt flw_gain_d(TO_FSTR(strings::flw_d), 20, 2);
	Opt flw_vel(TO_FSTR(strings::flw_vel), 180, 4);
	Opt flw_thresh_left(TO_FSTR(strings::flw_thresh_left), 165);
	Opt flw_thresh_right(TO_FSTR(strings::flw_thresh_right), 165);
	Opt flw_thresh_side(TO_FSTR(strings::flw_thresh_side), 280);
	Opt flw_recover(TO_FSTR(strings::flw_recover), 130);
	Opt flw_drecover(TO_FSTR(strings::flw_drecover), 5, 10);

	Opt beacon_theta(TO_FSTR(strings::beacon_theta), 125, 3);

	Opt home_gain_p(TO_FSTR(strings::home_p), 400);
	Opt home_gain_i(TO_FSTR(strings::home_i), 0);
	Opt home_gain_d(TO_FSTR(strings::home_d), 10);
	Opt home_vel(TO_FSTR(strings::home_vel), 100);

	Opt rev_dead_begin(TO_FSTR(strings::rev_dbegin), 110);
	Opt rev_dead_end(TO_FSTR(strings::rev_dend), 130);

	Opt rev_enable(TO_FSTR(strings::rev_enable), 0, 512);

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
		control::nop
	};

	const control::Mode dbg_mode
	{
		dbg_mode_begin,
		dbg_mode_tick,
		control::nop
	};

	const control::Mode view_mode
	{
		control::nop,
		view_mode_tick,
		control::nop
	};
}
