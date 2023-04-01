// MotorControl.h

#include <Arduino.h>

#ifndef _MOTORCONTROL_h
#define _MOTORCONTROL_h

//#define PWMA D1	// speed A
//#define PWMB D2	// speed B
//#define DA D3	// direction A
//#define DB D4	// direction B

#define IN1 D1
#define IN2 D2

#define DIRECTION_OPEN_VALUE true

class MotorControlClass
{
	protected:

	public:
		static void Stop();
		static void Init();
		static bool OpenDoor() { return MoveDoor(true); }
		static bool CloseDoor() { return MoveDoor(false); }
		static bool MoveDoor(bool direction);
};

extern MotorControlClass MotorControl;

#endif
