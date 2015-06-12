#pragma once
#include "control.h"
#include "io.h"

namespace pid
{
	struct Controller final
	{
		public:
			Controller(uint16_t gain_p, uint16_t gain_i, uint16_t gain_d);

			void in(int16_t entry);
			int16_t out() const;
			void reset();
		private:
			const int16_t _int_lim = 4086;
			int16_t _int, _prev, _now;
			uint16_t _gain_p, _gain_i, _gain_d;
	};

	extern const control::Mode follow_mode;

#ifdef IO_SIM
	extern TestCallback test_suite[];
#endif
}
