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

		enum MASKS : uint8_t
		{
			N_FORKS = 8,
			FORK_ACTIVE_MASK = 1 << 7,
			FIRST_CALL_MASK = 1 << 6,
			ADDRESS_MASK = 0xFFU >> 2
		};

		// forked commands, MSB is whether forks
		// 2nd MSB is whether first call
		uint8_t forks[N_FORKS];

		uint8_t IP; // 2nd high byte is whether is first call on current cmd
		CMD_TYPE program[256]; // determines type of call and other stuff

		Target targets[256];
		If ifs[256];
		uint16_t metadata[256];

		void async_begin()
		{
			IP = FIRST_CALL_MASK;
			memset(forks, 0, sizeof(uint8_t) * N_FORKS);
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
					uint8_t ptr = forks[i] & ADDRESS_MASK;

					if (!targets[ptr].func(forks[i] & FIRST_CALL_MASK, metadata[ptr]))
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

			uint8_t addr = IP & ADDRESS_MASK; // low 6 bits
			uint8_t type = program[addr]; // high 2 bits

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
					if (!targets[addr].func(IP & FIRST_CALL_MASK, metadata[addr]))
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
					}
					break;
				}
				case END:
				{
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

	uint8_t If::eval() const
	{
		switch (type)
		{
			case NEVER:
				return 0;
			case ARM_DEPRESSED:
				return !io::Digital::switch_lower.read();
			case QRD_SIDE_LEFT_GREATER_THAN:
				return io::Analog::qrd_side_left.read() > arg;
		}
	}

	const control::Mode async_mode
	{
		&async_begin,
		&async_tick,
		&control::nop
	};
}
