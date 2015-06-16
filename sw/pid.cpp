#include "pid.h"
#include "menu.h"
#include "motion.h"

namespace pid
{
	namespace
	{
		Controller controller(70, 1, 80); // default value

		void follow_mode_begin()
		{
			controller.gain_p = menu::flw_gain_p.value();
			controller.gain_i = menu::flw_gain_i.value();
			controller.gain_d = menu::flw_gain_d.value();
		}
		void follow_mode_tick()
		{
			int16_t thresh = 360;
			int16_t in = analogRead(0);

			controller.in((analogRead(0) - thresh));
			int16_t out = controller.out();

			// default 95
			io::log(menu::flw_vel.value());
			motion::vel(menu::flw_vel.value());
			motion::dir(controller.out());

			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
			}
			delay(10);
		}
		void follow_mode_end()
		{
			controller.reset();
			motion::vel(0);
			motion::dir(0);
		}
	}

	Controller::Controller(uint16_t gain_p, uint16_t gain_i, uint16_t gain_d)
		: _prev(0), _now(0), gain_p(gain_p), gain_i(gain_i), gain_d(gain_d) { }

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
		follow_mode_begin,
		follow_mode_tick,
		follow_mode_end,
	};
}
