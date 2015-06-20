#include "io.h"
#include "isr.h"
// #include <wiring_private.h>

namespace io
{
	namespace
	{
		uint8_t analog_roundrobin = 0;
	}

	void delay_ms(uint16_t ms)
	{
		uint16_t start = (uint16_t) micros();

		while (ms)
		{
			yield();
			if ((uint16_t) micros() - start >= 1000)
			{
				ms--;
				start += 1000;
			}
		}
	}
	void delay_us(uint16_t us)
	{
		uint16_t start = (uint16_t) micros();

		while (micros() > start + us)
		{
			yield();
		}
	}

	volatile uint16_t analog_pins[N_ANALOG] = { };

	void init()
	{
		// turn off ADC free-running
		ADCSRA &= ~(1 << ADFR);

		// start ADC continuous polling
		isr::attach_adc();
		start_adc();
	}

	void start_adc()
	{
		// set the analog reference (high two bits of ADMUX) and select the
		// channel (low 4 bits), and ADLAR to 0
		ADMUX = (1 << 6) | (analog_roundrobin & 0x07);
		
		// start the conversion
		ADCSRA |= 1 << ADSC;
	}

	void end_adc()
	{
		uint8_t low, high;
		low = ADCL;
		high = ADCH;
		analog_pins[analog_roundrobin] = (high << 8) | low;
		
		analog_roundrobin++;
		if (analog_roundrobin > N_ANALOG)
		{
			analog_roundrobin = 0;
		}

		start_adc();
	}

	namespace Digital
	{
		DIGITAL_INPUT(start, G, 2);
		DIGITAL_INPUT(stop, G, 1);
	}
	namespace Analog
	{
		ANALOG_INPUT(select, 7);
		ANALOG_INPUT(tweak, 6);
		ANALOG_INPUT(qrd_tape, 0);
	}
}
