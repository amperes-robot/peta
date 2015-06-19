#pragma once
#include <Arduino.h>
#include <motor.h>
#include <phys253pins.h>
#include <ServoTimer2.h>
#include <LiquidCrystal.h>

extern LiquidCrystal LCD;

#define N_ANALOG 8

namespace io
{
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

#ifdef IO_SIM
	typedef std::string string;
#else
	typedef String string;
#endif

	template<typename T>
	string to_string(T t)
	{
#ifdef IO_SIM
		std::ostringstream os;
		os << t;
		return os.str();
#else
		return string(t);
#endif
	}

	template<typename T>
	inline void log(T msg)
	{
#ifdef IO_SIM
		std::cout << msg << std::endl;
#else
		LCD.clear();
		LCD.home();
		LCD.print(to_string(msg));
#endif
	}

	namespace Analog
	{
		enum Analog_t : uint8_t
		{
			FOLLOW = 0,
			TWEAK = 6,
			SELECT = 7
		};
	}

	uint16_t analog_in(uint8_t pin);
	void start_adc(uint8_t pin);

#ifdef IO_SIM
	void set_input(uint8_t pin, bool value);

	extern TestCallback test_suite[];
#endif
}
