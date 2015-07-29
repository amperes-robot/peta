#pragma once
#include <Arduino.h>
#include <LiquidCrystal.h>

namespace io
{
	extern volatile uint16_t analog_pins[8];
	extern LiquidCrystal lcd;

	/**
	 * Initializes the IO module.
	 */
	void init();

	/**
	 * Sleeps for the given number of milliseconds.
	 */
	void delay_ms(uint16_t ms);

	/**
	 * Sleeps for the given number of microseconds.
	 */
	void delay_us(uint16_t us);

	/**
	 * Starts the next ADC conversion. Should never be called
	 * outside of end_adc() or init().
	 */
	void start_adc();

	namespace Timer
	{
		extern uint32_t _start;

		inline void start()
		{
			_start = millis();
		}
		inline uint16_t time()
		{
			return millis() - _start;
		}
	}

	namespace Analog
	{
		/**
		 * Represents an analog input.
		 */
		struct In final
		{
			public:
				In() = delete;
				In(const In&) = delete;
				inline In(uint8_t pin) : _pin(pin)
				{
				}

				/**
				 * Read this pin's value.
				 */
				inline uint16_t read() const
				{
					return analog_pins[_pin];
				}
			private:
				const uint8_t _pin;
		};

#define ANALOG_INPUT(NAME, NUM) const io::Analog::In NAME(NUM)
		/**
		 * WARNING:
		 * there must be at least one analog input
		 * otherwise the ADC interrupt will never end! 
		 */
		const extern In qrd_tape_left;
		const extern In qrd_tape_right;
		const extern In pd_left;
		const extern In pd_right;
		const extern In qrd_side_right;
		const extern In qrd_side_left;
		
		const extern In select;
		const extern In tweak;
	}
	namespace Digital
	{
		struct In final
		{
			public:
				In() = delete;
				In(const In&) = delete;

				inline In(volatile uint8_t* port, volatile uint8_t* ddr, volatile uint8_t* pin, uint8_t bit) : _pin(pin), _bit(bit)
				{
					*ddr &= ~bit;
					*port &= ~bit;
				}

				inline bool read() const
				{
					return *_pin & _bit;
				}
			private:
				const volatile uint8_t* _pin;
				const uint8_t _bit;
		};
		struct Out final
		{
			public:
				Out() = delete;
				Out(const Out&) = delete;

				inline Out(volatile uint8_t* port, volatile uint8_t* ddr, volatile uint8_t* pin, uint8_t bit) : _port(port), _pin(pin), _bit(bit)
				{
					*ddr |= bit;
				}

				inline void write(bool value) const
				{
					if (value)
					{
						*_port |= _bit;
					}
					else
					{
						*_port &= ~_bit;
					}
				}

				inline bool read() const
				{
					return *_pin & _bit;
				}
			private:
				volatile uint8_t* _port;
				const volatile uint8_t* _pin;
				const uint8_t _bit;
		};

#define DIGITAL_INPUT(NAME, PORTx, NUM) const io::Digital::In NAME(&PORT ## PORTx, &DDR ## PORTx, &PIN ## PORTx, 1 << PIN ## NUM)
#define DIGITAL_OUTPUT(NAME, PORTx, NUM) const io::Digital::Out NAME(&PORT ## PORTx, &DDR ## PORTx, &PIN ## PORTx, 1 << PIN ## NUM)
		const extern In start;
		const extern In stop;

		const extern In switch_upper;
		const extern In switch_lower;
		const extern In enc_left;
		const extern In enc_right;
		const extern In enc_arm;

		const extern In switch_front;
    const extern In switch_arm;

		const extern Out zipline_enable;
	}
}
