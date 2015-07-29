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
		EITHER_SIDE_QRD_GT,
		FRONT_LEFT_QRD_GT,
		FRONT_RIGHT_QRD_GT,

		L_ENC_GT,
		L_ENC_LT,
		R_ENC_GT,
		R_ENC_LT,
		A_ENC_GT,
		A_ENC_LT,

		L_MINUS_R_ENC_GT,
		L_PLUS_R_ENC_GT
	};

	struct If
	{
		inline If() : type(TRUE), arg(0) { }
		inline If(Condition type, int16_t arg = 0) : type(type), arg(arg) { }

		uint8_t eval() const;
		void init() const;

		Condition type;
		int16_t arg;
	};
	typedef If Until;

	extern void begin();

	extern void fork(Action, Until, uint16_t = 0);
	extern void exec(Action, Until, uint16_t = 0);
	extern void branch(int8_t, If);

	extern void end();

	extern const control::Mode async_mode;
}
