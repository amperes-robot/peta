#include "io.h"
#include "isr.h"

namespace io
{
	volatile uint16_t analog_pins[8] = { };

	namespace
	{
		volatile uint8_t analog_roundrobin = 0;
	}

	LiquidCrystal lcd(26, 27, 28, 16, 17, 18, 19, 20, 21, 22, 23);

	void delay_ms(uint16_t ms)
	{
		uint16_t start = (uint16_t) micros();

		while (ms)
		{
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
		}
	}

	void init()
	{
		lcd.begin(16, 2);

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

		analog_pins[analog_roundrobin++] = (high << 8) | low;
		
		if (analog_roundrobin >= 8)
		{
			analog_roundrobin = 0;
		}

		start_adc();
	}

	namespace Digital
	{
		DIGITAL_INPUT(start, G, 2);
		DIGITAL_INPUT(stop, G, 1);

		DIGITAL_INPUT(enc_left, D, 0);
		DIGITAL_INPUT(enc_right, D, 1);
		DIGITAL_INPUT(enc_arm, D, 2);

		DIGITAL_INPUT(switch_upper, G, 2);
		DIGITAL_INPUT(switch_lower, D, 5);
		DIGITAL_INPUT(switch_front, D, 3);
    DIGITAL_INPUT(switch_arm, D, 7);

		DIGITAL_OUTPUT(zipline_enable, E, 3);
	}
	namespace Analog
	{
		ANALOG_INPUT(qrd_tape_left, 0);
		ANALOG_INPUT(qrd_tape_right, 1);
		ANALOG_INPUT(pd_left, 2);
		ANALOG_INPUT(pd_right, 3);
		ANALOG_INPUT(qrd_side_right, 4); //qrd side right
		ANALOG_INPUT(qrd_side_left, 5); // qrd side left

		ANALOG_INPUT(tweak, 6);
		ANALOG_INPUT(select, 7);
	}

	namespace Timer
	{
		uint32_t _start;
	}
}

ISR(ADC_vect)
{
	io::end_adc();
}
