#include "isr.h"
#define TIMSK1 TIMSK

namespace isr
{
	namespace
	{
		Handler h_timer1;
	}

	/* Configures Timer1 to call an interrupt routine with the desired frequency  */
	/* The interrupt routine that is called is ISR(TIMER1_COMPA_vect)             */
	/* Valid interrupt frequencies: 1Hz to 65535 Hz                               */
	/* If the frequency is impossible to achieve, no interrupt will be configured */
	void attach_timer1(Handler handle, uint16_t freq)
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
				h_timer1 = handle;
				return;
			}
		}
	}

	void detach_timer1()
	{
		TIMSK1 &= ~(1 << OCIE1A);
	}
}

ISR(TIMER1_COMPA_vect)
{
	if (isr::h_timer1)
	{
		isr::h_timer1();
	}
}
