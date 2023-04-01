// Climate.h
#ifndef _CLIMATE_h
#define _CLIMATE_h

#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <AHT10.h>
#include <ccs811.h>
#include <Esp.h>
#include <vector>

#define EXPIRATION_TIME 24*60*60
#define NUMBER_VARIABLES 3
#define CELSIUS_TO_KELVIN		273.15

#define CCS811_WAKE_PIN D8
#define I2C_SDA_PIN D7
#define I2C_SCL_PIN D6

class ClimateClass {

    private:
    CCS811 _ccs811 = CCS811(CCS811_WAKE_PIN);
    AHT10 _aht10 = AHT10(AHT10_ADDRESS_0X38);
    bool _aht10_connected, _ccs811_connected = false;

    class Variable {
        public:
        Variable(float alpha, String name, String type);
        class ValueTime {
            public:
            float Value;
            time_t Time;
            ValueTime() {};
            ValueTime(float value, time_t time)
                {this->Value = value; this->Time = time; }
        };

        void Update(float value);
        bool Initialized() { return _initialized; }
        ValueTime Value() { return _currentvalue; }
        ValueTime Maximum() { return _maxvalues[0]; }
        ValueTime Minimum() { return _minvalues[0]; }
        String Name;
        String Type;
        void Update();
        void SendXML(ESP8266WebServer* server);
        
        private:
        bool _initialized, _settled = false;
        size_t _count = 0;
        float _alpha, _oneminusalpha;
        ValueTime _currentvalue;
        std::vector<ValueTime> _maxvalues, _minvalues;
        String protoXML(String name, String type, ValueTime currentvalue);
    };

    Variable _variables[NUMBER_VARIABLES] = {
        Variable(0.1, "Temperature", "temperature/kelvin"),
        Variable(0.1, "Humidity", "relative_humidity/percentage"),
        Variable(0.2, "CO2 Concentration", "co2/ppm")
    };

    public:
    ClimateClass();
    void Update();
    void SendXML(ESP8266WebServer* server);
};

#endif