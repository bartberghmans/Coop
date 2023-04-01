#include "MotorControl.h"

void MotorControlClass::Init()
{
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	Stop();
}

void MotorControlClass::Stop()
{
	digitalWrite(IN1, LOW);
	digitalWrite(IN2, LOW);
}

bool MotorControlClass::MoveDoor(bool direction_value)
{
  bool control = !DIRECTION_OPEN_VALUE ^ direction_value;
  
	digitalWrite(IN1, direction_value );
	digitalWrite(IN2, !direction_value);
}

MotorControlClass MotorControl;
