#include "control.h"
#include "io.h"

/**
 * C++ interfaces to main functions. These need to be inlined because this
 * file is included directly from main.pde, and because Processing is really
 * weird.
 */
inline void init_c()
{
	control::init();
}

inline void loop_c()
{
	control::loop();
}
