#pragma once
#include "io.h"

namespace isr
{
	/* Configures Timer1 to call an interrupt routine with the desired frequency  */
	/* The interrupt routine that is called is ISR(TIMER1_COMPA_vect)             */
	/* Valid interrupt frequencies: 1Hz to 65535 Hz                               */
	/* If the frequency is impossible to achieve, no interrupt will be configured */
	void attach_timer1(uint16_t freq);
	void detach_timer1();

	/* Enables an external interrupt pin
	 * INTX: Which interrupt should be configured?
	 * mode: Which pin state should trigger the interrupt?
	 * LOW     - trigger whenever pin state is LOW
	 * FALLING - trigger when pin state changes from HIGH to LOW
	 * RISING  - trigger when pin state changes from LOW  to HIGH 
	*/
	void attach_pin(uint8_t pin, uint8_t mode);
	void detach_pin(uint8_t pin);

	void attach_adc();
	void detach_adc();
}
