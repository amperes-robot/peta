#include <phys253.h> 
#include <LiquidCrystal.h> 

#define SAMPLES 100
unsigned long ravg[SAMPLES];
int aptr = 0;
unsigned long tick = 0;
unsigned long updatetick = 0;
int previous = -1;

void setup()
{ 
#include <phys253setup.txt>
	Serial.begin(9600);
	for (int i = 0; i < SAMPLES; i++)
	{
		ravg[i] = 0;
	}
}
void loop()
{
	int s = digitalRead(0);

	if (s != previous)
	{
		previous = s;

		LCD.clear();
		LCD.home();
		LCD.print(s ? "tick" : "tock");

		/*
		unsigned long tock = micros();

		ravg[aptr] = tock - tick;
		tick = tock;

		if (++aptr >= SAMPLES)
		{
			aptr = 0;
		}

		if (tock > updatetick + 1000000L)
		{
			updatetick = tock;
			unsigned long sum = 0;
			for (int i = 0; i < SAMPLES; i++)
			{
				sum += ravg[i];
			}
			double avg = (double) sum / SAMPLES;

			LCD.clear();
			LCD.home();
			LCD.print(500000.0D / avg);
			LCD.print("Hz");
			LCD.setCursor(0, 1);
			LCD.print(avg);
			LCD.print("us");
		}
		*/
	}
} 

