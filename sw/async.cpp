#include "async.h"
#include "menu.h"
#include "motion.h"

namespace async
{
	namespace
	{
		enum CMD_TYPE : uint8_t
		{
			FORK = 0,
			EXEC = 1,
			BRANCH = 2,
			END = 3
		};

		enum MASKS : uint16_t
		{
			N_FORKS = 4U,
			FORK_ACTIVE_MASK = 1U << 15,
			FIRST_CALL_MASK = 1U << 14,
			ADDRESS_MASK = 0x3FFFU
		};

		uint16_t forks[N_FORKS];
		uint16_t IP; // 2nd high byte is whether is first call on current cmd

		CMD_TYPE program[256]; // determines type of call and other stuff

		Target targets[256];
		If ifs[256];
		uint16_t metadata[256];

		void async_begin()
		{
			IP = FIRST_CALL_MASK;
			memset(forks, 0, sizeof(uint16_t) * N_FORKS);
		}

		void async_tick()
		{
			if (menu::stop_falling())
			{
				control::set_mode(&menu::main_mode);
				return;
			}

			for (uint8_t i = 0; i < N_FORKS; i++)
			{
				if (forks[i] & FORK_ACTIVE_MASK) // enumerate active forks
				{
					// index is 6 lowest bits
					uint16_t ptr = forks[i] & ADDRESS_MASK;
					uint8_t first_call = (forks[i] & 16384U) == 16384U;

					if (first_call)
					{
						ifs[ptr].init();
					}

					if (!targets[ptr].func(first_call, metadata[ptr]))
					{
						// returned false -> disable fork
						forks[i] = 0;
					}
					else
					{
						forks[i] &= ~FIRST_CALL_MASK; // clear first call bit

						if (ifs[ptr].eval())
						{
							// returned true -> disable fork
							forks[i] = 0;
						}
					}
				}
			}

			uint16_t addr = IP & ADDRESS_MASK; // low 14 bits
			CMD_TYPE type = program[addr];

			uint8_t inc = 0; // whether to increment IP

			switch (type)
			{
				case FORK:
				{
					for (uint8_t i = 0; i < N_FORKS; i++)
					{
						if (!(forks[i] & FORK_ACTIVE_MASK))
						{
							forks[i] = addr | FORK_ACTIVE_MASK | FIRST_CALL_MASK;
							inc = 1;
							break;
						}
					}

					if (inc) break;

					io::lcd.clear();
					io::lcd.home();
					io::lcd.print("FORKS EXCEEDED");
					control::set_mode((const control::Mode*) nullptr);

					break;
				}
				case EXEC:
				{
					uint8_t first_call = (IP & 16384U) == 16384U;

					if (first_call)
					{
						ifs[addr].init();
					}

					if (!targets[addr].func(first_call, metadata[addr]))
					{
						inc = 1;
					}
					else
					{
						IP &= ~FIRST_CALL_MASK; // clear first call bit

						if (ifs[addr].eval()) // condition met
						{
							inc = 1;
						}
					}
					break;
				}
				case BRANCH:
				{
					if (ifs[addr].eval()) // true
					{
						IP += targets[addr].addr; // branch off
						IP |= FIRST_CALL_MASK;
					}
					break;
				}
				case END:
				{
					motion::left.halt();
					motion::right.halt();
					motion::arm.halt();
					motion::excavator.halt();

					control::set_mode(&menu::main_mode);
					return;
				}
			}

			if (inc)
			{
				IP++;
				IP |= FIRST_CALL_MASK;

				if (!(IP & ADDRESS_MASK)) // IP is 0
				{
					// wraparound, overflow happened
					io::lcd.clear();
					io::lcd.home();
					io::lcd.print("IP OVERFLOW");
					control::set_mode((const control::Mode*) nullptr);
				}
			}

			motion::update_enc();
		}
	}

	void begin()
	{
		IP = 0;
	}
	void end()
	{
		program[IP] = END;
	}
	void fork(Action action, Until until, uint16_t meta)
	{
		program[IP] = FORK;
		targets[IP].func = action;
		ifs[IP] = until;
		metadata[IP] = meta;
		IP++;
	}
	void exec(Action action, Until until, uint16_t meta)
	{
		program[IP] = EXEC;
		targets[IP].func = action;
		ifs[IP] = until;
		metadata[IP] = meta;
		IP++;
	}
	void branch(int8_t loc, If if_)
	{
		program[IP] = BRANCH;
		targets[IP].addr = loc;
		ifs[IP] = if_;
		IP++;
	}

	namespace { uint16_t ir_hysteresis_prev; }

	void If::init() const
	{
		switch (type)
		{
			case L_ENC_GT:
			case L_ENC_LT:
				motion::update_enc();
				motion::left_theta = 0;
				break;

			case R_ENC_GT:
			case R_ENC_LT:
				motion::update_enc();
				motion::right_theta = 0;
				break;

			case X_ENC_GT:
			case X_ENC_LT:
				motion::update_enc();
				motion::excavator_theta = 0;
				break;

			// don't reset the theta counts for the arm and excavator

			case L_MINUS_R_ENC_GT:
			case L_PLUS_R_ENC_GT:
				motion::update_enc();
				motion::left_theta = 0;
				motion::right_theta = 0;
				break;

			case TIMER_GT:
				io::Timer::start();
				break;

			case IR_HYSTERESIS_GT:
				ir_hysteresis_prev = 0;
		}
	}

	uint8_t If::eval() const
	{
		switch (type)
		{
			case TRUE:
				return 1;
			case FALSE:
				return 0;
			case IR_HYSTERESIS_GT:
				ir_hysteresis_prev = (ir_hysteresis_prev * 3 + io::Analog::pd_left.read() + io::Analog::pd_right.read()) / 4;
				io::delay_ms(100);
				io::lcd.clear();
				io::lcd.home();
				io::lcd.print(ir_hysteresis_prev);
				return ir_hysteresis_prev > arg;
			case EITHER_SIDE_QRD_GT:
				return io::Analog::qrd_side_left.read() > arg || io::Analog::qrd_side_right.read() > arg;
			case FRONT_LEFT_QRD_GT:
				return io::Analog::qrd_tape_left.read() > arg;
			case FRONT_RIGHT_QRD_GT:
				return io::Analog::qrd_tape_right.read() > arg;
			case L_ENC_GT:
				return motion::left_theta > arg;
			case L_ENC_LT:
				return motion::left_theta < arg;
			case R_ENC_GT:
				return motion::right_theta > arg;
			case R_ENC_LT:
				return motion::right_theta < arg;
			case A_ENC_GT:
				return motion::arm_theta > arg;
			case A_ENC_LT:
				return motion::arm_theta < arg;
			case X_ENC_GT:
				return motion::excavator_theta > arg;
			case X_ENC_LT:
				return motion::excavator_theta < arg;
			case L_MINUS_R_ENC_GT:
				return motion::left_theta - motion::right_theta > arg;
			case L_PLUS_R_ENC_GT:
				return motion::left_theta + motion::right_theta > arg;
			case FRONT_SWITCH_TRIGGER:
				return io::Digital::switch_front.read();
			case TIMER_GT:
				return io::Timer::time() > arg;
		}
	}

	const control::Mode async_mode
	{
		&async_begin,
		&async_tick,
		&control::nop
	};
}
