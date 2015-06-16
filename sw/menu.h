#pragma once
#include <avr/EEPROM.h>
#include "control.h"
#include "io.h"

namespace menu
{
	struct Opt final
	{
		public:
			static uint8_t opt_count;
			inline Opt(String name, float scale) : _name(name), _scale(scale)
			{
				_addr_eep = (uint16_t*)(2 * opt_count++);
				_value = eeprom_read_word(_addr_eep);
			}

			inline uint16_t value() const
			{
				return _value;
			}

			inline float scale() const
			{
				return _scale;
			}

			inline String name() const
			{
				return _name;
			}

			inline void write(uint16_t value)
			{
				_value = value;
				eeprom_write_word(_addr_eep, value);
			}
		private:
			String _name;
			float _scale;
			uint16_t* _addr_eep;
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

	void init();

	/**
	 * Returns true if the stop button is on falling edge.
	 */
	bool stop_falling();

	/**
	 * Retruns treu if start button is on falling edge.
	 */
	bool start_falling();
}
