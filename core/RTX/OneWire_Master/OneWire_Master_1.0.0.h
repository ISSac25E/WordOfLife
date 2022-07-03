// OneWire_Master
//.h
#ifndef OneWire_Master_h
#define OneWire_Master_h

#ifndef OneWire_Master_DataPin
#error "OneWire_Master_DataPin was not declared"
#endif

#ifndef OneWire_Master_DeviceCount
#error "OneWire_Master_DeviceCount was not declared"
#endif

#ifndef OneWire_Master_DeviceList
#error "OneWire_Master_DeviceList was not declared. Format: ip_1, ip_2, ip_3..."
#endif

// definitions for DeviceBuffer core:

#define DeviceBuffer_DeviceCount OneWire_Master_DeviceCount

#ifndef DeviceBuffer_MaxPacket
#define DeviceBuffer_MaxPacket 1 // default 1 packet per device
#endif

#ifndef DeviceBuffer_MaxByte
#define DeviceBuffer_MaxByte 10 // default 10 bytes per packet
#endif

#include "Arduino.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\core\DeviceBuffer\DeviceBuffer_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\core\OneWire_Driver\OneWire_Driver_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\core\BoolConverter\BoolConverter_1.0.0.h"

class OneWire_Master
{
public:
  OneWire_Master();
  void run();                    // handles communication with devices. Run as frequently as possible
  bool deviceConnected(uint8_t); // Input Device IP, Ooutput true/false device connected

private:
  bool _parseRead(bool *, uint8_t &, uint8_t, uint16_t); // input(bitArray(return Array), bit count(return count), max allowed length(of packet), max parse time(us)) output true if msg recieved. false otherwise
  bool _connectCommand(uint8_t, uint8_t);                // input(device ip, command). output depends on inputed command
  bool _checkIp(uint8_t, uint8_t &);                     // input (true/actual ip, return tagged ip) return false if ip not found
  bool _oneWireSafe();                                   // set delay to check if data line is

  // create private objects:
  DeviceBuffer _DeviceBuffer;
  OneWire_Driver _Pin;

  const uint8_t _ipList[OneWire_Master_DeviceCount] = {OneWire_Master_DeviceList};
  const uint8_t _ipCount = OneWire_Master_DeviceCount;
};

//.cpp

OneWire_Master::OneWire_Master()
{
  _Pin.setPin(OneWire_Master_DataPin);
  for (uint8_t x = 0; x < OneWire_Master_DeviceCount; x++)
  {
    _connectCommand(x, 0); // reset all connections
  }
}
void OneWire_Master::run()
{
  for (uint8_t ip = 0; ip < _ipCount; ip++) // Run through all ip's each loop
  {
    if (_connectCommand(ip, 2)) // ip Connected
    {
      bool bitArray[88];     // buffer max possible msg
      uint8_t bitMarker = 0; // bitmarker for messages

      BoolConverter.compileVal(bitArray, bitMarker, _ipList[ip], 3);                     // compile first 3 bits(ip address)
      BoolConverter.compileVal(bitArray, bitMarker, 1, 1);                               // next bit, 1 = connected, 0 = disconnected
      BoolConverter.compileVal(bitArray, bitMarker, _DeviceBuffer.readAvailable(ip), 1); // if read is availabe for device, 1 = available, 0 = not available

      if (_oneWireSafe()) // if safe, send packet
      {
        _Pin.write(bitArray, bitMarker);
        if (_parseRead(bitArray, bitMarker, 88, 40)) // check if any response:
        {
          // check packet lengths:
          if (bitMarker == 5) // read request, connection command request
          {
            // set up vars to parse packet:
            uint8_t packet_ip;
            uint8_t packet_command;
            bitMarker = 0;

            BoolConverter.decompileVal(bitArray, bitMarker, packet_ip, 3);
            BoolConverter.decompileVal(bitArray, bitMarker, packet_command, 2);

            if (packet_ip == _ipList[ip]) // verify that ip is correct
            {
              bitMarker = 0;
              if (packet_command == 0) // connect command
              {
                _connectCommand(ip, 1);

                BoolConverter.compileVal(bitArray, bitMarker, _ipList[ip], 3);
                BoolConverter.compileVal(bitArray, bitMarker, 1, 1);
                BoolConverter.compileVal(bitArray, bitMarker, _DeviceBuffer.readAvailable(ip), 1);

                _Pin.write(bitArray, bitMarker);
                delayMicroseconds(60);
              }
              else if (packet_command == 1) // read command, return any available messages
              {
                _connectCommand(ip, 1);

                // read from buffer:
                uint8_t sender_ip_tag;
                uint8_t bytesAvailable;
                uint8_t dataArray[10];
                bool readAvailable = _DeviceBuffer.read(ip, sender_ip_tag, bytesAvailable, dataArray);

                // compile message:
                BoolConverter.compileVal(bitArray, bitMarker, _ipList[ip], 3);
                BoolConverter.compileVal(bitArray, bitMarker, readAvailable, 2);
                if (readAvailable)
                {
                  BoolConverter.compileVal(bitArray, bitMarker, _ipList[sender_ip_tag], 3);
                  BoolConverter.compileArray(bitArray, bitMarker, dataArray, bytesAvailable);
                }
                _Pin.write(bitArray, bitMarker);
                delayMicroseconds(60);
              }
            }
          }
          else if (bitMarker % 8 == 0 && bitMarker >= 8) // write request
          {
            uint8_t packet_ip;
            uint8_t packet_command;
            uint8_t bytesToSend = (bitMarker / 8) - 1;
            bitMarker = 0; // set marker to 0 for decompiling

            BoolConverter.decompileVal(bitArray, bitMarker, packet_ip, 3);      // decompile packet ip
            BoolConverter.decompileVal(bitArray, bitMarker, packet_command, 2); // decompile packet command

            if (packet_ip == _ipList[ip]) // verify recieved ip
            {
              if (packet_command == 2)
              {
                _connectCommand(ip, 1);

                uint8_t r_ip;
                uint8_t r_ip_tag;

                BoolConverter.decompileVal(bitArray, bitMarker, r_ip, 3); // decompile reciever ip

                if (_checkIp(r_ip, r_ip_tag)) // check if valid ip and convert to tagged-ip
                {
                  if (_connectCommand(r_ip_tag, 2)) // check if recieving ip is connected also
                  {
                    uint8_t dataArray[bytesToSend]; // prepare data array to decompile to
                    BoolConverter.decompileArray(bitArray, bitMarker, dataArray, bytesToSend);
                    // start compileing message:
                    bitMarker = 0; // set marker to 0 to compile message
                    BoolConverter.compileVal(bitArray, bitMarker, _ipList[ip], 3);
                    BoolConverter.compileVal(bitArray, bitMarker, _DeviceBuffer.write(r_ip_tag, ip, bytesToSend, dataArray), 1);
                    BoolConverter.compileVal(bitArray, bitMarker, 0, 1);
                  }
                  else // recieving ip is not connected
                  {
                    // start compileing message:
                    bitMarker = 0; // set marker to 0 to compile message
                    BoolConverter.compileVal(bitArray, bitMarker, _ipList[ip], 3);
                    BoolConverter.compileVal(bitArray, bitMarker, 2, 2);
                  }
                  _Pin.write(bitArray, bitMarker); // send message
                  delayMicroseconds(60);           // buffer a little
                }
              }
            }
          }
          else if (bitMarker % 8 == 5 && bitMarker >= 5) // Master command request
          {
          }
        }
      }
    }
    else // ip not connected
    {
      bool bitArray[5]; // buffer for max bossible msg: 5 bits
      uint8_t bitMarker = 0;

      BoolConverter.compileVal(bitArray, bitMarker, _ipList[ip], 3); // compile first 3 bits(ip address)
      BoolConverter.compileVal(bitArray, bitMarker, 0, 2);           // next two bits: bit_0: connected, bit_1: Read available(both are 0 since the is not connected)

      if (_oneWireSafe()) // send msg if bus safe
      {
        _Pin.write(bitArray, bitMarker);
        if (_parseRead(bitArray, bitMarker, 5, 40)) // check if any response
        {
          if (bitMarker == 5) // connect command only
          {
            // vars for parseing packet:
            uint8_t packet_ip;
            uint8_t packet_command;
            bitMarker = 0; // set marker to 0 for decompiling

            BoolConverter.decompileVal(bitArray, bitMarker, packet_ip, 3);      // decompile packet_ip
            BoolConverter.decompileVal(bitArray, bitMarker, packet_command, 2); // decompile packet_command

            if (packet_ip == _ipList[ip]) // check if ip matches
            {
              if (packet_command == 0) // check if packet_command is "connect"(0)
              {
                _connectCommand(ip, 0); // disconnect first to flush buffer
                _connectCommand(ip, 1); // mark as connected

                bitMarker = 0; // set marker to 0 to compile packet

                BoolConverter.compileVal(bitArray, bitMarker, _ipList[ip], 3);                     // compile ip
                BoolConverter.compileVal(bitArray, bitMarker, 1, 1);                               // compile 1 (for connected status)
                BoolConverter.compileVal(bitArray, bitMarker, _DeviceBuffer.readAvailable(ip), 1); // set to 1 or 0 if read available

                _Pin.write(bitArray, bitMarker); // send msg
                delayMicroseconds(60);           // buffer a little
              }
            }
          }
        }
      }
    }
  }
}
bool OneWire_Master::deviceConnected(uint8_t ip)
{
  if (ip < _ipCount)
    return _connectCommand(ip, 2);
}

bool OneWire_Master::_parseRead(bool *bitArray, uint8_t &bitCount, uint8_t maxLength, uint16_t maxParseTime)
{
  uint16_t counter = 0;
  maxParseTime *= 2.5;
  while (counter < maxParseTime)
  {
    counter++;
    if (!_Pin.pinRead())
    {
      uint8_t readState = _Pin.read(bitArray, bitCount, maxLength);
      if (readState == 0)
        return false; // corrupted message
      if (readState == 1)
        return true; // valid message
      // else, timeout/start-bit error (keep trying)
    }
  }
  return false; // timeout, return false
}
bool OneWire_Master::_connectCommand(uint8_t tag_ip, uint8_t command)
{
  static bool deviceConnected[OneWire_Master_DeviceCount];
  static uint32_t deviceTimer[OneWire_Master_DeviceCount];
  const uint16_t maxConnectionTimeout = 3000; // 3 second connection timeout
  if (tag_ip < _ipCount)
  {
    switch (command)
    {
    case 0:
      // disconnect command
      deviceConnected[tag_ip] = false;
      deviceTimer[tag_ip] = millis();
      // flush buffer for device:
      _DeviceBuffer.deviceFlush(tag_ip);
      break;
    case 1:
      // connect command
      deviceConnected[tag_ip] = true;
      deviceTimer[tag_ip] = millis();
      break;
    case 2:
      // check connect command
      if (deviceConnected[tag_ip])
      {
        if ((millis() - deviceTimer[tag_ip]) > maxConnectionTimeout)
        {
          deviceConnected[tag_ip] = false;
          deviceTimer[tag_ip] = millis();
          _DeviceBuffer.deviceFlush(tag_ip);
        }
      }
      break;
    }
    return deviceConnected[tag_ip];
  }
  return false;
}
bool OneWire_Master::_checkIp(uint8_t ip, uint8_t &tag_ip)
{
  for (uint8_t x = 0; x < _ipCount; x++)
  {
    if (ip == _ipList[x])
    {
      tag_ip = x;
      return true;
    }
  }
  tag_ip = 0;
  return false;
}
bool OneWire_Master::_oneWireSafe()
{
  uint16_t counter = 0;
  uint16_t delay = 20;
  while (counter < delay && _Pin.pinRead())
  {
    counter++;
    delayMicroseconds(2);
  }
  if (counter >= delay)
    return true;
  return false;
}

#endif
