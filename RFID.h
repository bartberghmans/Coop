// RFID.h
#ifndef _RFID_h
#define _RFID_h

#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define RFIDSIGNATURE_LENGTH 32
#define STORE_SIZE 16*4*16

class RFIDClass
{
	public:
        class RFIDEntry
        {
            public:
                void Set(long id);
                String GetXMLNode();

                class RFIDTag
                {
                    public:
                        long ID = 0;
                        String Label = "";
                        RFIDTag(long id, String label);
                };

            private:
                long _id;
                time_t _timestamp;
        };
        
        RFIDClass::RFIDEntry _entries[STORE_SIZE];

        RFIDClass();
        void Register(unsigned long long id);
        void Register() { Register(_current_rfid_id); }
        bool Parse(char d);
        void SendXML(ESP8266WebServer* server);
        long CurrentID() { return _current_rfid_id; }

	private:
        size_t _current_pointer = 0;
        unsigned long long _current_rfid_id = 0;
        unsigned char _received_checksum;
        bool _id_is_valid;
        int _state = 0;  
        byte get_val(char c);
        int get_checksum(unsigned long long data);
};

#endif

