#include "Climate.h"

ClimateClass::Variable::Variable(float alpha, String name, String type)
{
    this->Name = name;
    this->Type = type;
    _alpha = alpha;
    _oneminusalpha = 1.0 - alpha;
}

void ClimateClass::Variable::Update(float value)
{
    time_t timenow = now();
    time_t timeexperiation = timenow - EXPIRATION_TIME;

    float currentvalue;
    if (_initialized)
    {
        currentvalue = (_currentvalue.Value * _oneminusalpha) + (_alpha * value);
    }
    else
    {
        currentvalue = value;
        _initialized = true;
    }
    _currentvalue.Value = currentvalue;
    _currentvalue.Time = timenow;

    if (!_settled)
    {
        _count++;
        _settled = (_count > (size_t)(1 / _alpha));

        if (_settled)
        {
            _maxvalues.push_back(_currentvalue);
            _minvalues.push_back(_currentvalue);
        }
    }
    else
    {
        for (std::vector<ValueTime>::reverse_iterator rit = _maxvalues.rbegin(); rit != _maxvalues.rend(); rit++)
        {
            if (currentvalue > rit->Value)
            {
                _maxvalues.pop_back();
            }
            else
                break;
        }
        for (std::vector<ValueTime>::reverse_iterator rit = _minvalues.rbegin(); rit != _minvalues.rend(); rit++)
        {
            if (currentvalue < rit->Value)
            {
                _minvalues.pop_back();
            }
            else
                break;
        }
        for (std::vector<ValueTime>::iterator it = _maxvalues.begin(); it != _maxvalues.end(); it++)
        {
            if (timeexperiation > it->Time)
            {
                _maxvalues.erase(it--);
            }
            else
                break;
        }
        for (std::vector<ValueTime>::iterator it = _minvalues.begin(); it != _minvalues.end(); it++)
        {
            if (timeexperiation > it->Time)
            {
                _minvalues.erase(it--);
            }
            else
                break;
        }

        _maxvalues.push_back(_currentvalue);
        _minvalues.push_back(_currentvalue);
    }
}

String ClimateClass::Variable::protoXML(String name, String type, ValueTime currentvalue)
{
    char buffer[32];
    time_t time = currentvalue.Time;
    snprintf(buffer, 32, "%04d-%02d-%02dT%02d:%02d:%02dZ", year(time), month(time), day(time), hour(time), minute(time), second(time));
    String timestamp(buffer);

    String result = "<status name=\"" + name;
    result += "\" type=\"" + type;
    result += "\" timestamp=\"" + timestamp;
    result += "\">" + String(currentvalue.Value, 2) + "</status>";

    return result;
}

void ClimateClass::Variable::SendXML(ESP8266WebServer *server)
{
    if (_initialized)
    {
        server->sendContent(protoXML(this->Name, this->Type, _currentvalue));
        if (_settled)
        {
            server->sendContent(protoXML("Max. " + this->Name, this->Type, this->Maximum()));
            server->sendContent(protoXML("Min. " + this->Name, this->Type, this->Minimum()));
        }
    }
}

ClimateClass::ClimateClass()
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    _ccs811.set_i2cdelay(50);
    if (_ccs811.begin()) _ccs811_connected = _ccs811.start(CCS811_MODE_1SEC);
    Serial.println(_ccs811_connected ? "CCS811 connected" : "CCS811 connection error");

    //_aht10_connected = _aht10.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    _aht10_connected = true;
    Serial.println(_aht10_connected ? "AHT10 connected" : "AHT10 connection error");

}

void ClimateClass::Update()
{
    float temp, hum;
    int temp_i, hum_i;
    uint16_t eco2, etvoc, errstat, raw;    

    if (_aht10_connected)
    {
        _aht10.readRawData();
        temp = _aht10.readTemperature(AHT10_USE_READ_DATA);
        hum = _aht10.readHumidity(AHT10_USE_READ_DATA);

        _variables[0].Update(temp + CELSIUS_TO_KELVIN);
        _variables[1].Update(hum);

        if (_ccs811_connected) {
            Serial.println("Setting env. variables");
            temp_i = (int)(temp*1000);
            hum_i = (int)(hum*1000);
            _ccs811.set_envdata210(temp_i, hum_i);
        }
        else {
            Serial.println("Unable to set env. variables");
        }
    }

    if (_ccs811_connected)
    {
        delay(50);
        _ccs811.read(&eco2,&etvoc,&errstat,&raw);

        Serial.print("Reading CCS811... ");
        if (errstat==CCS811_ERRSTAT_OK){
            _variables[2].Update(eco2);
            Serial.println("OK");
        }
        else {Serial.print(_ccs811.errstat_str(errstat)); Serial.println(" fail");}
    }
    else
    {
        Serial.println("Restarting CCS811");
        _ccs811_connected = _ccs811.start(CCS811_MODE_60SEC);
    }
}

void ClimateClass::SendXML(ESP8266WebServer *server)
{
    for (size_t i = 0; i < NUMBER_VARIABLES; i++)
    {
        _variables[i].SendXML(server);
    }
}