#pragma once
#include "io.h"

namespace isr
{
	/* 
	 * Attaches ISR(TIMER1_COMPA_vect) at the speficied frequency
	 * Valid interrupt frequencies: 1Hz to 65535 Hz
	 *
	 * WARNING: Will interfere with Motor 1.
	 */
	void attach_timer1(uint16_t freq);
	void detach_timer1();

	/* 
	 * Attaches ISR(TIMER3_COMPA_vect) at the speficied frequency
	 * Valid interrupt frequencies: 1Hz to 65535 Hz
	 *
	 * Note: may not work as intended
	 */
	void attach_timer3(uint16_t freq);
	void detach_timer3();

	/* Enables an external interrupt pin
	 * pin: which pin to trigger
	 * mode: which pin state should trigger the interrupt
	 * LOW     - trigger whenever pin state is LOW
	 * FALLING - trigger when pin state changes from HIGH to LOW
	 * RISING  - trigger when pin state changes from LOW  to HIGH 
	 */
	void attach_pin(uint8_t pin, uint8_t mode);
	void detach_pin(uint8_t pin);

	void attach_adc();
	void detach_adc();
}
