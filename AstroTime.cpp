// 
// 
// 
#include "AstroTime.h"

AstroTimeClass::AstroTimeClass()
{
}

void AstroTimeClass::init(double lat, double lon)
{
	_lattitude = lat;
	_longitude = lon;
}

String AstroTimeClass::getTimeString(time_t time)
{
	char buffer[32];
	snprintf(buffer, 32, "%04d-%02d-%02dT%02d:%02d:%02dZ", year(time), month(time), day(time), hour(time), minute(time), second(time));
	String result(buffer);
	return result;
}

time_t AstroTimeClass::getUT(uint16_t day_of_year, bool sunrise)
{
	// convert the longitude to hour value and calculate an approximate time
	double lnHour = _longitude / 15.0;
	double t;
	if (sunrise) t = day_of_year + ((6.0 - lnHour) / 24.0);
	else t = day_of_year + ((18 - lnHour) / 24.0);

	//calculate the Sun's mean anomaly
	double M = (0.9856*t) - 3.289;

	//calculate the Sun's true longitude
	double L = M + (1.916 * sin(M * C)) + (0.020 * sin(2 * M * C)) + 282.634;
	if (L > 360.0) L = L - 360.0;
	else if (L < 0.0) L = L + 360.0;

	//calculate the Sun's right ascension
	double RA = atan(0.91764 * tan(L * C)) / C;
	if (RA > 360.0) RA = RA - 360.0;
	else if (RA < 0.0) RA = RA + 360.0;

	//right ascension value needs to be in the same qua
	double Lquadrant = (floor(L / 90.0)) * 90.0;
	double RAquadrant = (floor(RA / 90.0)) * 90.0;
	RA = RA + (Lquadrant - RAquadrant);

	//right ascension value needs to be converted into hours
	RA /= 15.0;
	
	//calculate the Sun's declination
	double sinDec = -0.39782 * sin(L * C);
	double cosDec = cos(asin(sinDec));

	//calculate the Sun's local hour angle
	double cosH = (sinDec * sin(_lattitude * C)) / (cosDec * cos(_lattitude * C));
	double H;
	if (sunrise) H = 360.0 - acos(cosH) / C;
	else H = acos(cosH) / C;
	H /= 15.0;

	//calculate local mean time of rising/setting
	double T = H + RA - (0.06571 * t) - 6.622;

	time_t result = (time_t)trunc(T * 3600.0);
	return result;
}

time_t AstroTimeClass::getMidnight()
{
	tmElements_t today_offset;
	today_offset.Year = year() - 1970;
	today_offset.Month = month();
	today_offset.Day = day();
	today_offset.Hour = 0;
	today_offset.Minute = 0;
	today_offset.Second = 0;
	time_t dateoffset = makeTime(today_offset) - trunc(240.0 * _longitude);

	return dateoffset;
}

time_t AstroTimeClass::adjustTime(time_t time)
{
	time_t lower_limit = now();
	time_t upper_limit = lower_limit + SECONDS_IN_DAY;

	while (time < lower_limit) time += SECONDS_IN_DAY;
	while (time > upper_limit) time -= SECONDS_IN_DAY;

	return time;
}

uint16_t AstroTimeClass::getDayOfYear(uint8_t day, uint8_t month)
{
	// approximate day_of_year (not counting leap days)
	uint16_t day_in_month[12] = { 0,31,59,90,120,151,181,212,243,273,304,334 };
	return day+day_in_month[month-1];
}