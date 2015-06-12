#pragma once
#include "control.h"
#include "io.h"

namespace menu
{
	struct Opt final
	{
		public:
			static uint16_t opt_count;
			Opt(String name);

			inline uint8_t value()
			{
				return _value;
			}

			inline String name()
			{
				return _name;
			}

			inline void write(uint8_t value)
			{
				_value = value;
				eeprom_write_word(_addr_eep, value);
			}
		private:
			uint16_t* _addr_eep;
			String _name;
			uint16_t _value;
	};

	/**
	 * Menu modes.
	 */
	extern const control::Mode main_mode;
	extern const control::Mode opt_mode;
}
