#pragma once
#include "control.h"
#include "io.h"

namespace pid
{
	struct DigitalController final
	{
		public:
			DigitalController(uint16_t gain_p, uint16_t gain_i, uint16_t gain_d);

			void in(int8_t entry);
			int16_t out() const;
			void reset();

			uint16_t gain_p, gain_i, gain_d;
		private:
			const int8_t _int_lim = 128;
			int8_t _int, _now, _prev0, _prev1;
			uint8_t _time0, _time1;
	};

	struct Controller final
	{
		public:
			Controller(uint16_t gain_p, uint16_t gain_i, uint16_t gain_d);

			void in(int16_t entry);
			int16_t out() const;
			void reset();

			uint16_t gain_p, gain_i, gain_d;
		private:
			const int16_t _int_lim = 4086;
			int16_t _int, _prev, _now;
	};

	int16_t follow_value();
	int8_t follow_value_digital();

	extern const control::Mode follow_mode;
	extern int8_t digital_recovery;

#ifdef IO_SIM
	extern TestCallback test_suite[];
#endif
}
