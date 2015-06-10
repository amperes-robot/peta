#pragma once
#include "io.h"

namespace isr
{
	typedef void (*Handler)();
	void attach_timer1(Handler handler, uint16_t freq);
	void detach_timer1();
}
