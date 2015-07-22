#include "pid.h"
#include "menu.h"
#include "motion.h"

namespace pid
{
	namespace
	{
		DigitalController controller(0, 0, 0);

		void follow_mode_begin()
		{
			io::lcd.clear();
			io::lcd.home();
			io::lcd.print(TO_FSTR(strings::retrieval));

			controller.reset();
			controller.gain_p = menu::flw_gain_p.value();
			controller.gain_i = menu::flw_gain_i.value();
			controller.gain_d = menu::flw_gain_d.value();
		}
		void follow_mode_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			int8_t in = follow_value_digital();

			controller.in(in);
			int16_t out = controller.out();

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

		if (left > 0) // previous sensor on tape
		{
			previous = -menu::flw_recover.value();
		}
		else if (right > 0)
		{
			previous = menu::flw_recover.value();
		}

		if (left < 0 && right < 0) // both lost
		{
			return right - left + previous;
		}
		else
		{
			return right - left;
		}
	}

	int8_t digital_recovery = 0;

	int8_t follow_value_digital()
	{
		int8_t left = io::Analog::qrd_tape_left.read() > menu::flw_thresh_left.value();
		int8_t right = io::Analog::qrd_tape_right.read() > menu::flw_thresh_right.value();

		if (left) // previous sensor on tape
		{
			digital_recovery = -((int8_t) menu::flw_drecover.value());
		}
		else if (right)
		{
			digital_recovery = menu::flw_drecover.value();
		}

		if (!left && !right) // both lost
		{
			return digital_recovery;
		}
		else if (left && right) // both on
		{
			return 0;
		}
		else if (left)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}

	DigitalController::DigitalController(uint16_t gain_p, uint16_t gain_i, uint16_t gain_d)
		: gain_p(gain_p), gain_i(gain_i), gain_d(gain_d), _now(0)
	{
		_prev0 = 0;
		_prev1 = 0;
		_time0 = 0;
		_time1 = 0;
	}

	void DigitalController::in(int8_t entry)
	{
		_int += entry;
		if (_int > _int_lim) _int = _int_lim;
		if (_int < -_int_lim) _int = -_int_lim;

		_time0++;
		_time1++;

		if (_now != entry)
		{
			_time0 = _time1;
			_prev0 = _prev1;
			_time1 = 0;
			_prev1 = _now;
		}

		_now = entry;
	}

	int16_t DigitalController::out() const
	{
		int16_t fp = (int32_t) _now * gain_p / 32;
		int16_t fi = (int32_t) _int * gain_i / 32;
		int16_t fd = (int32_t) (_now - _prev0) * gain_d / _time0;

		return -(fp + fi + fd);
	}

	void DigitalController::reset()
	{
		_prev0 = 0;
		_time0 = 0;
		_prev1 = 0;
		_time1 = 0;
		_now = 0;
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
