#include <LiquidCrystal.h>
#include "loop.cpp"

void setup()
{
	/*
	RCServo0.attach(RCServo0Output);
	RCServo1.attach(RCServo1Output);
	RCServo2.attach(RCServo2Output);
	*/
	init_c();
}

void loop()
{
	loop_c();
}
