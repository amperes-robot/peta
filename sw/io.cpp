#include "io.h"

namespace io
{
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
}
