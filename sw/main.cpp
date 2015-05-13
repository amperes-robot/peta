#include "control.h"
#include "log.h"

/**
 * C++ interfaces to main functions. These need to be inlined because this
 * file is included directly from main.pde, and because Wiring is really
 * weird.
 */
inline void init_c()
{
	log("begin");
	control::init();
}

inline void loop_c()
{
	control::loop();
}
