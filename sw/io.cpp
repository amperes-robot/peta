#include "io.h"

#include <wiring_private.h>

namespace io
{
	uint16_t analog_pins[N_ANALOG] = { };

	uint16_t analog_in(uint8_t pin)
	{
		// TODO
		return analogRead(pin);
	}

	void start_adc(uint8_t pin)
	{
#if defined(analogPinToChannel)
#if defined(__AVR_ATmega32U4__)
		if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#endif
		pin = analogPinToChannel(pin);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		if (pin >= 54) pin -= 54; // allow for channel or pin numbers
#elif defined(__AVR_ATmega32U4__)
		if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
		if (pin >= 24) pin -= 24; // allow for channel or pin numbers
#else
		if (pin >= 14) pin -= 14; // allow for channel or pin numbers
#endif

#if defined(ADCSRB) && defined(MUX5)
		// the MUX5 bit of ADCSRB selects whether we're reading from channels
		// 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
		ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#endif

		// set the analog reference (high two bits of ADMUX) and select the
		// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
		// to 0 (the default).
#if defined(ADMUX)
		ADMUX = (DEFAULT << 6) | (pin & 0x07);
#endif

		// start the conversion
		sbi(ADCSRA, ADSC);
	}

	namespace Digital
	{
		DIGITAL_INPUT(start, G, 2);
		DIGITAL_INPUT(stop, G, 1);
	}
}
