#pragma once

#ifndef nullptr
#define nullptr ((void*) 0)
#endif

namespace control
{
	/**
	 * Interface that performs the main handling of everything.
	 */
	struct Mode final
	{
		public:
			void (*begin)(void* args);
			void (*tick)();
			void (*end)();
	};

	extern const Mode idle_mode;

	/**
	 * Sets the current mode to the given mode with the given arguments.
	 */
	void set_mode(const Mode* mode, void* args = nullptr);

	void init();
	void loop();

	inline void nop() { }
	inline void nop_void(void*) { }
}
