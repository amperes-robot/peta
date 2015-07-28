#include "control.h"
#include "io.h"

namespace async
{
	typedef uint8_t (*Action)(uint8_t begin, uint16_t meta);

	union Target
	{
		Action func;
		int8_t addr;
	};

	enum Condition
	{
		NEVER = 0,
		FOREVER = 0,
		ARM_DEPRESSED,
		QRD_SIDE_LEFT_GREATER_THAN,
	};

	struct If
	{
		inline If() : type(NEVER), arg(0) { }
		inline If(Condition type, uint16_t arg = 0) : type(type), arg(arg) { }

		uint8_t eval() const;

		Condition type;
		uint16_t arg;
	};
	typedef If Until;

	extern void begin();

	extern void fork(Action, Until, uint16_t = 0);
	extern void exec(Action, Until, uint16_t = 0);
	extern void branch(int8_t, If);

	extern void end();

	extern const control::Mode async_mode;
}
