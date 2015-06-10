#pragma once
#include "io.h"

namespace pid
{
	struct Controller final
	{
		public:
			typedef int16_t Entry;
			typedef int32_t EntryAccumulator;

			/**
			 * Construct a controller with gain specified.
			 */
			Controller(float gain_p, float gain_i, float gain_d);

			/**
			 * Add a new entry to the data buffer.
			 */
			void update(Entry entry);

			/**
			 * Retrieves the output of the controller.
			 */
			Entry output() const;
		private:
			static const size_t _buffer_size = 32;
			static const size_t _deriv_hyst = 4;
			EntryAccumulator _int;

			Entry _data[_buffer_size];
			Entry* _begin;
			size_t _count;

			float _gain_p, _gain_i, _gain_d;

			inline Entry* end() const
			{
				Entry* r = _begin + _count;
				return r >= _data + _buffer_size ? r - _buffer_size : r;
			}
	};

#ifdef IO_SIM
	extern TestCallback test_suite[];
#endif
}
