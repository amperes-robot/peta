#include "isr.h"
#include "motion.h"

#include <avr/interrupt.h>

namespace isr
{
	namespace
	{
		const uint32_t timer_overflows_hz[] = { F_CPU / 1, F_CPU / 8, F_CPU / 64, F_CPU / 256, F_CPU / 1024 };
	}

	void attach_timer1(uint16_t freq)
	{
		for (uint8_t i = 0; i < 5; i++)
		{
			/* The number of 16-bit timer overflows needed to obtain the desired frequency */
			const uint32_t overflowsNeeded = timer_overflows_hz[i] / freq;
			/* Check if the number of overflows can be stored in a 16-bit register */
			if (overflowsNeeded <= 0xFFFFU)
			{
				cli();
				TCCR1A = 0;                         /* Clear current comparison value */
				TCNT1 = 0;                         /* Clear current timer value      */
				OCR1A = (uint16_t) overflowsNeeded; /* Set timer comparison value     */
				TCCR1B = (1 << WGM12);              /* Set timer comparison mode      */
				TCCR1B |= i + 1;                    /* Set timer prescaler value      */
				TIMSK |= (1 << OCIE1A);            /* Set timer interrupt enable     */
				sei();
				return;
			}
		}
	}

	void detach_timer1()
	{
		TIMSK &= ~(1 << OCIE1A);
	}

	void attach_timer3(uint16_t freq)
	{
		for (uint8_t i = 0; i < 5; i++)
		{
			/* The number of 16-bit timer overflows needed to obtain the desired frequency */
			const uint32_t overflowsNeeded = timer_overflows_hz[i] / freq;

			/* Check if the number of overflows can be stored in a 16-bit register */
			if (overflowsNeeded <= 0xFFFFU)
			{
				cli();
				TCCR3A = 0;                         /* Clear current comparison value */
				TCNT3 = 0;                         /* Clear current timer value      */
				OCR3A = (uint16_t) overflowsNeeded; /* Set timer comparison value     */
				TCCR3B = (1 << WGM12);              /* Set timer comparison mode      */
				TCCR3B |= i + 1;                    /* Set timer prescaler value      */
				ETIMSK |= (1 << OCIE3A);            /* Set timer interrupt enable     */
				sei();
				return;
			}
		}
	}

	void detach_timer3()
	{
		ETIMSK &= ~(1 << OCIE3A);
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
		EICRA |= (mode & (1 << 1)) << (pin * 2);
		EICRA |= (mode & (1 << 0)) << (pin * 2 + 1);
		sei();
	}

	void detach_pin(uint8_t pin)
	{
		EIMSK &= ~(1 << pin);
	}
}
