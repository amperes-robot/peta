#include "isr.h"

#include <avr/interrupt.h>

#define TIMSK1 TIMSK

namespace isr
{
	void attach_timer1(uint16_t freq)
	{
		const uint32_t timerOverflowHz[] = {F_CPU / 1, F_CPU / 8, F_CPU / 64, F_CPU / 256, F_CPU / 1024};
		for (uint8_t i = 4; i < 5; i++)
		{
			/* The number of 16-bit timer overflows needed to obtain the desired frequency */
			const uint32_t overflowsNeeded = timerOverflowHz[i] / freq;
			/* Check if the number of overflows can be stored in a 16-bit register */
			if (overflowsNeeded <= 0xFFFFU)
			{
				noInterrupts();
				TCCR1A = 0;                         /* Clear current comparison value */
				TCNT1  = 0;                         /* Clear current timer value      */
				OCR1A  = (uint16_t) overflowsNeeded; /* Set timer comparison value     */
				TCCR1B = (1 << WGM12);              /* Set timer comparison mode      */
				TCCR1B |= i + 1;                    /* Set timer prescaler value      */
				TIMSK1 |= (1 << OCIE1A);            /* Set timer interrupt enable     */
				interrupts();
				return;
			}
		}
	}

	void detach_timer1()
	{
		TIMSK1 &= ~(1 << OCIE1A);
	}

	void attach_adc()
	{
		ADCSRA |= (1 << ADIE);
	}

	void detach_adc()
	{
		ADCSRA &= ~(1 << ADIE);
	}

	void attach_pin(uint8_t pin, uint8_t mode)
	{
		if (pin > 3 || mode > 3 || mode == 1) return;
		cli();
		/* Allow pin to trigger interrupts        */
		EIMSK |= (1 << pin);
		/* Clear the interrupt configuration bits */
		EICRA &= ~(1 << (pin * 2));
		EICRA &= ~(1 << (pin * 2 + 1));
		/* Set new interrupt configuration bits   */
		EICRA |= (mode & (1 << 0)) << (pin * 2);
		EICRA |= (mode & (1 << 1)) << (pin * 2 + 1);
		sei();
	}

	void detach_pin(uint8_t pin)
	{
		EIMSK &= ~(1 << pin);
	}
}

ISR(TIMER1_COMPA_vect) { }
ISR(INT0_vect) { }
ISR(INT1_vect) { }
ISR(INT2_vect) { }
ISR(INT3_vect) { }
ISR(ADC_vect)
{
	io::end_adc();
}
