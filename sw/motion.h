#pragma once
#include "io.h"

namespace motion
{
	extern volatile uint8_t enc0_counts;
	extern volatile uint8_t enc1_counts;
	extern volatile uint8_t enc2_counts;

	struct Motor final
	{
		public:
			Motor(uint8_t id, uint8_t reverse);
			void speed(int16_t id);
			void halt();
			inline int8_t dir() const
			{
				return _dir;
			}
		private:
			const uint8_t _id, _rev;
			int8_t _dir;
	};

	void init();

	/*
	 * Move in a direction, with -255 being right, 255 being left.
	 */
	void dir(int16_t x);

	/*
	 * Speed.
	 */
	void vel(int16_t x);

	/**
	 * Update encoder state.
	 */
	void enc0();
	void enc1();
	void update_enc();

	extern Motor left;
	extern Motor right;
	extern Motor arm;
	extern Motor zipline;

	extern volatile uint32_t x;
	extern volatile uint32_t y;
	extern volatile uint16_t theta;

	extern volatile int8_t arm_theta;
}
