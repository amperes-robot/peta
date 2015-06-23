#pragma once
#include "io.h"

namespace motion
{
	struct Motor final
	{
		public:
			Motor(uint8_t id);
			void speed(int16_t id);
		private:
			const uint8_t _id;
	};

	/*
	 * Move in a direction, with -255 being right, 255 being left.
	 */
	void dir(int16_t x);

	/*
	 * Speed.
	 */
	void vel(int16_t x);

	extern Motor left;
	extern Motor right;
	extern Motor retrieval;
	extern Motor zipline;
}
