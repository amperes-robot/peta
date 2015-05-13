#pragma once

namespace control
{
	/**
	 * Interface that controls the robot.
	 * All functions must be overriden.
	 */
	class Mode
	{
		public:
			virtual void begin() = 0;
			virtual void tick() = 0;
			virtual void end() = 0;
	};

	class IdleMode : public Mode
	{
		public:
			void begin();
			void tick();
			void end();
			IdleMode() { }
	};

	void init();

	void loop();
}
