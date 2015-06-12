#pragma once
#include "control.h"
#include "io.h"

namespace menu
{
	struct Opt final
	{
		public:
			static uint16_t opt_count;
			Opt(String name, float scale);

			inline uint16_t value()
			{
				return _value;
			}

			inline float scale()
			{
				return _scale;
			}

			inline String name()
			{
				return _name;
			}

			inline void write(uint16_t value)
			{
				_value = value;
				eeprom_write_word(_addr_eep, value);
			}
		private:
			float _scale;
			uint16_t* _addr_eep;
			String _name;
			uint16_t _value;
	};

	extern Opt flw_gain_p;
	extern Opt flw_gain_i;
	extern Opt flw_gain_d;
	extern Opt flw_vel;


	/**
	 * Menu modes.
	 */
	extern const control::Mode main_mode;
	extern const control::Mode opt_mode;

	/**
	 * Returns true if the stop button is on falling edge.
	 */
	bool stop_falling();

	/**
	 * Retruns treu if start button is on falling edge.
	 */
	bool start_falling();
}
