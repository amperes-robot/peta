#include "pid.h"
#include "menu.h"
#include "motion.h"

namespace pid
{
	namespace
	{
		Controller controller(0, 0, 0);

		void follow_mode_begin()
		{
			controller.reset();
			controller.gain_p = menu::flw_gain_p.value();
			controller.gain_i = menu::flw_gain_i.value();
			controller.gain_d = menu::flw_gain_d.value();
		}
		void follow_mode_tick()
		{
			int16_t in = follow_value();

			controller.in(in);
			int16_t out = controller.out();

			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
			}

			motion::vel(menu::flw_vel.value());
			motion::dir(out);

			io::delay_ms(10);
		}
	}

	int16_t follow_value()
	{
		static int16_t previous = 0;
		int16_t left = io::Analog::qrd_tape_left.read() - menu::flw_thresh_left.value();
		int16_t right = io::Analog::qrd_tape_right.read() - menu::flw_thresh_right.value();

		if (left < 0)
		{
			previous = -menu::flw_recover.value();
		}
		else if (right < 0)
		{
			previous = menu::flw_recover.value();
		}

		if (left > 0 && right > 0) // both lost
		{
			return right - left + previous;
		}
		else
		{
			return right - left;
		}
	}

	Controller::Controller(uint16_t gain_p, uint16_t gain_i, uint16_t gain_d)
		: gain_p(gain_p), gain_i(gain_i), gain_d(gain_d), _prev(0), _now(0) { }

	void Controller::in(int16_t entry)
	{
		_int += entry;
		if (_int > _int_lim) _int = _int_lim;
		if (_int < -_int_lim) _int = -_int_lim;

		_prev = _now;
		_now = entry;
	}

	int16_t Controller::out() const
	{
		int16_t fp = (int32_t) _now * gain_p / 255;
		int16_t fi = (int32_t) _int * gain_i / 255;
		int16_t fd = (int32_t) (_now - _prev) * gain_d / 255;

		return -(fp + fi + fd);
	}

	void Controller::reset()
	{
		_prev = 0;
		_now = 0;
	}

	const control::Mode follow_mode
	{
		&follow_mode_begin,
		&follow_mode_tick,
		&control::nop,
	};
}
