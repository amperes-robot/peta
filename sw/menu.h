#pragma once

#include <avr/eeprom.h>

#include "control.h"
#include "io.h"
#include "strings.h"

namespace menu
{
	/**
	 * Represents an EEPROM-saved parameter.
	 */
	struct Opt final
	{
		public:
			static uint8_t opt_count;
			Opt() = delete;
			Opt(const Opt& other) = delete;
			Opt(FSTR name, uint16_t def, uint16_t scale = 1);

			inline uint16_t value() const
			{
				return _value;
			}

			inline FSTR name() const
			{
				return _name;
			}

			inline uint16_t scale() const
			{
				return _scale;
			}

			inline void restore()
			{
				write(_default);
			}

			inline void write(uint16_t value)
			{
				_value = value;
				eeprom_write_word(_addr_eep, value);
			}
		private:
			uint16_t _scale;
			uint16_t _value;
			uint16_t* const _addr_eep;
			const FSTR _name;
			const uint16_t _default;
	};

	const size_t N_OPTS = 17;
	extern Opt flw_gain_p;
	extern Opt flw_gain_i;
	extern Opt flw_gain_d;
	extern Opt flw_vel;
	extern Opt flw_thresh_left;
	extern Opt flw_thresh_right;
	extern Opt flw_thresh_side;
	extern Opt flw_recover;
	extern Opt flw_drecover;

	extern Opt home_gain_p;
	extern Opt home_gain_i;
	extern Opt home_gain_d;
	extern Opt home_vel;

	extern Opt beacon_theta;

	extern Opt rev_dead_begin;
	extern Opt rev_dead_end;
	extern Opt rev_enable;

	/**
	 * Menu modes.
	 */
	extern const control::Mode main_mode;
	extern const control::Mode dbg_mode;
	extern const control::Mode opt_restore_mode;
	extern const control::Mode opt_mode;
	extern const control::Mode view_mode;

	/**
	 * Initialize the menu module.
	 */
	void init();

	/**
	 * Returns true if the stop button is on falling edge.
	 * Needs to be called once per tick in order to work.
	 */
	bool stop_falling();

	/**
	 * Retruns true if start button is on falling edge.
	 * Needs to be called once per tick in order to work.
	 */
	bool start_falling();
}
