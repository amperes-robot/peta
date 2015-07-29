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
		TRUE,
		FALSE,
		ARM_DEPRESSED,
		EITHER_SIDE_QRD_GREATER_THAN,
		FRONT_LEFT_QRD_GREATER_THAN,
		FRONT_RIGHT_QRD_GREATER_THAN,

		LEFT_ENC_GREATER_THAN,
		LEFT_ENC_LESS_THAN,
		RIGHT_ENC_GREATER_THAN,
		RIGHT_ENC_LESS_THAN,
		ARM_ENC_GREATER_THAN,
		ARM_ENC_LESS_THAN
	};

	struct If
	{
		inline If() : type(TRUE), arg(0) { }
		inline If(Condition type, uint16_t arg = 0) : type(type), arg(arg) { }

		uint8_t eval() const;
		void init() const;

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
