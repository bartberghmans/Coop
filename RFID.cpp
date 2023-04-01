#include "RFID.h"

RFIDClass::RFIDClass()
{
  for (size_t i = 0; i < STORE_SIZE; i++)
  {
    _entries[i] = RFIDEntry();
  }
}

void RFIDClass::Register(unsigned long long id)
{
  _entries[_current_pointer].Set(id);
  _current_pointer++;
  if (_current_pointer == STORE_SIZE)
    _current_pointer = 0;
}

void RFIDClass::SendXML(ESP8266WebServer* server)
{
  server->sendContent("<traffic>");
  for (size_t i = 0; i < STORE_SIZE; i++)
  {
    String xmlnode = _entries[i].GetXMLNode();
    if (xmlnode != "") server->sendContent(xmlnode);
  }
  server->sendContent("</traffic>");
}

bool RFIDClass::Parse(char d)
{
  unsigned long long val = 0;
  size_t desl = 0;

  _id_is_valid = false;

  switch (_state)
  {
  case 0:
    if (d == 0x02)
    {
      _current_rfid_id = 0;
      _received_checksum = 0;
      _state = 1;
    }
    // else => garbage!
    break;
  case 11: //checksum byte
    _received_checksum = d;
    _state++;
    break;
  case 12:
    if (d == 0x03 && _received_checksum == get_checksum(_current_rfid_id))
    {
      _id_is_valid = true;
    }
    _state = 0;
    break;
  default:
    val = get_val(d);
    desl = (10 - _state) * 4;
    _current_rfid_id |= val << desl;
    _state++;
    break;
  }
  return _id_is_valid;
}

byte RFIDClass::get_val(char c)
{
  static const char ascii_diff = 48;
  c -= ascii_diff;
  if (c > 9)
    c -= 7;
  return c;
}

int RFIDClass::get_checksum(unsigned long long data)
{
  union
  {
    unsigned char uc[8];
    unsigned long long ul;
  } tmp;
  tmp.ul = data;
  return tmp.uc[0] ^ tmp.uc[1] ^ tmp.uc[2] ^ tmp.uc[3] ^ tmp.uc[4];
}

String RFIDClass::RFIDEntry::GetXMLNode()
{
  if (_timestamp == 0)
    return "";

  char buffer[RFIDSIGNATURE_LENGTH + 48];
  snprintf(buffer, RFIDSIGNATURE_LENGTH + 48, "<passage time=\"%04d-%02d-%02dT%02d:%02d:%02dZ\" id=\"%ld\"/>",
           year(_timestamp), month(_timestamp), day(_timestamp),
           hour(_timestamp), minute(_timestamp), second(_timestamp), _id);

  String result(buffer);
  return result;
}

void RFIDClass::RFIDEntry::Set(long id)
{
  _timestamp = now();
  _id = id;
}

RFIDClass::RFIDEntry::RFIDTag::RFIDTag(long id, String label)
{
  ID = id;
  Label = label;
}