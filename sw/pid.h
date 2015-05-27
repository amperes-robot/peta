#pragma once
#include "io.h"

namespace pid
{
	struct Controller
	{
		public:
			static const size_t buffer_size = 64;

			typedef int Entry;
			void update(Entry entry);
			Entry output() const;
		private:
			Data[buffer_size] _data;
	}

	void gen_controller();

#ifdef IO_TST
	extern TestCallback test_suite[];
#endif
}
