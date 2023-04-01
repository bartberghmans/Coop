// AstroTime.h
#include  <arduino.h>

#ifndef _ASTROTIME_h
#define _ASTROTIME_h

#include <TimeLib.h>

#define C DEG_TO_RAD // 3.14159276 / 180
#define SECONDS_IN_DAY 60*60*24  

class AstroTimeClass
{

private:
	double _lattitude = 50.8503396;
	double _longitude = 4.3517103;
	time_t getUT(uint16_t day_of_year, bool sunrise);

 public:
	 AstroTimeClass();
	void init(double lat, double lon);
	double getLatitude() { return _lattitude; }
	double getLongitude() { return _longitude; }
	time_t getMidnight();
	uint16_t getDayOfYear(uint8_t day, uint8_t month);
	time_t getSunset(uint16_t day) { return getUT(day, false); }
	time_t getSunrise(uint16_t day) { return getUT(day, true); }
	time_t adjustTime(time_t time);
	String getTimeString(time_t time);
};

#endif
