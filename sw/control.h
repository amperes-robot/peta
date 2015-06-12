#pragma once

#ifndef nullptr
#define nullptr ((void*) 0)
#endif

namespace control
{
	/**
	 * Interface that controls the robot at any given point in time.
	 */
	struct Mode final
	{
		public:
			void (*begin)();
			void (*tick)();
			void (*end)();
	};

	extern const Mode idle_mode;

	void set_mode(const Mode* mode);

	void init();
	void loop();
}
