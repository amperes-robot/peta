#pragma once
#include <Arduino.h>
#include <motor.h>
#include <phys253pins.h>
#include <ServoTimer2.h>
#include <LiquidCrystal.h>

extern LiquidCrystal LCD;

namespace io
{
	extern volatile uint16_t analog_pins[8];
	extern uint8_t analog_attached;

	namespace Analog
	{
		struct In final
		{
			public:
				In() = delete;
				In(const In&) = delete;

				inline In(uint8_t pin) : _pin(pin)
				{
					analog_attached |= 1 << pin;
				}

				inline bool read() const
				{
					return analog_pins[_pin];
				}

			private:
				const uint8_t _pin;
		};

#define ANALOG_INPUT(NAME, NUM) const io::Analog::In NAME(NUM)
		/* WARNING:
		 * there must be at least one analog input
		 * otherwise the ADC interrupt will never end! 
		 */
		const extern In select;
		const extern In tweak;
		const extern In qrd_tape;
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
	}

	typedef String string;

	void init();

	template<typename T>
	inline void log(T msg)
	{
		LCD.clear();
		LCD.home();
		LCD.print(string(msg));
	}

	void delay_ms(uint16_t ms);
	void delay_us(uint16_t us);
	void start_adc();
	void end_adc();
}
