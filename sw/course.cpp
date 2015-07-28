#include "course.h"
#include "isr.h"
#include "io.h"
#include "menu.h"
#include "motion.h"
#include "pid.h"
#include "async.h"

namespace course
{
	namespace
	{
		using namespace async;

		pid::DigitalController dcontroller(0, 0, 0);

		uint8_t follow(uint8_t first)
		{
			if (first)
			{
				dcontroller.reset();
				dcontroller.gain_p = menu::flw_gain_p.value();
				dcontroller.gain_i = menu::flw_gain_i.value();
				dcontroller.gain_d = menu::flw_gain_d.value();
			}

			int8_t in = pid::follow_value_digital();

			dcontroller.in(in);
			int16_t out = dcontroller.out();

			motion::vel(menu::flw_vel.value());
			motion::dir(out);

			return 1;
		}

		void begin_tick()
		{
			begin();

			exec(&follow, Until(QRD_SIDE_GREATER_THAN, menu::flw_thresh_side.value()));

			end();

			control::set_mode(&async::async_mode);
		}
	}

	int16_t pet_id;

	const control::Mode begin_mode
	{
		&control::nop,
		&begin_tick,
		&control::nop
	};
}
