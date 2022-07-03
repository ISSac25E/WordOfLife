// declare definitions for master device:
#define OneWire_Master_DataPin 8                   // declare bus data pin
#define OneWire_Master_DeviceList 0, 1, 2, 3, 4, 5 // declare all possible devices on bus
#define OneWire_Master_DeviceCount 6               // state number of devices

// OneWire_Master
//.h
/*
  changes
    - removed all static vars to prevent future errors
*/
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
// DeviceBuffer
//.h
#ifndef DeviceBuffer_h
#define DeviceBuffer_h

#include "Arduino.h"

#ifndef DeviceBuffer_DeviceCount
#error "DeviceBuffer_DeviceCount was not declared"
#endif

#ifndef DeviceBuffer_MaxPacket
#error "DeviceBuffer_MaxPacket was not declared"
#endif

#ifndef DeviceBuffer_MaxByte
#error "DeviceBuffer_MaxByte was not declared"
#endif

class DeviceBuffer
{
public:
  DeviceBuffer(); // This is used to reset buffer before hand

  bool readAvailable(uint8_t);         // check if device has msg available
  uint8_t readAvailableBytes(uint8_t); // chack how many bytes are available

  bool writeAvailable(uint8_t, uint8_t);         // check if device can write to another device
  uint8_t writeAvailableBytes(uint8_t, uint8_t); // check how many bytes cann be written

  bool read(uint8_t, uint8_t &, uint8_t &, uint8_t *); // Read if message available for a device, return true if available
  bool write(uint8_t, uint8_t, uint8_t, uint8_t *);    // Write to a device, return true if successful

  void deviceFlush(uint8_t); // Flush all data from given device

private:
  bool _checkDevice(uint8_t); // check if device exists

  //_dataBuffer: [DEV][PCK NO.][PCK CONT.(0-(MX_BYT-1))::BYT CNT(MX_BYT+0)::TX IP(MX_BYT+1)]
  //_readMarker: [DEV][BYT_AVL(0)::BYT_MRK(1)]
  volatile uint8_t _dataBuffer[DeviceBuffer_DeviceCount][DeviceBuffer_DeviceCount * DeviceBuffer_MaxPacket][DeviceBuffer_MaxByte + 2];
  volatile uint8_t _readMarker[DeviceBuffer_DeviceCount][2];
};

//.cpp

DeviceBuffer::DeviceBuffer()
{
  for (uint8_t x = 0; x < DeviceBuffer_DeviceCount; x++)
    this->deviceFlush(x);
}

bool DeviceBuffer::readAvailable(uint8_t r_device)
{
  if (this->_checkDevice(r_device))
  {
    if (this->_readMarker[r_device][0] > 0)
      return true;
  }
  return false;
}
uint8_t DeviceBuffer::readAvailableBytes(uint8_t r_device)
{
  if (this->_checkDevice(r_device))
  {
    return this->_readMarker[r_device][0];
  }
  return 0;
}

bool DeviceBuffer::writeAvailable(uint8_t r_device, uint8_t t_device)
{
  if (this->_checkDevice(r_device) && this->_checkDevice(t_device))
  {
    uint8_t WriteCount = 0;
    for (uint8_t X = 0; X < this->_readMarker[r_device][0]; X++)
    {
      if (this->_dataBuffer[r_device][(this->_readMarker[r_device][1] + X) % (DeviceBuffer_MaxPacket * DeviceBuffer_DeviceCount)][DeviceBuffer_MaxByte + 1] == t_device)
        WriteCount++;
    }
    if (WriteCount >= DeviceBuffer_MaxPacket)
      return false;
    else
      return true;
  }
  return false;
}
uint8_t DeviceBuffer::writeAvailableBytes(uint8_t r_device, uint8_t t_device)
{
  if (this->_checkDevice(r_device) && this->_checkDevice(t_device))
  {
    uint8_t WriteCount = 0;
    for (uint8_t X = 0; X < this->_readMarker[r_device][0]; X++)
    {
      if (this->_dataBuffer[r_device][(this->_readMarker[r_device][1] + X) % (DeviceBuffer_MaxPacket * DeviceBuffer_DeviceCount)][DeviceBuffer_MaxByte + 1] == t_device)
        WriteCount++;
    }
    return (DeviceBuffer_MaxPacket - WriteCount);
  }
  return 0;
}

bool DeviceBuffer::read(uint8_t r_device, uint8_t &t_device, uint8_t &bytesAvailable, uint8_t *dataArray)
{
  if (this->_checkDevice(r_device))
  {
    if (this->_readMarker[r_device][0])
    {
      for (uint8_t X = 0; X < this->_dataBuffer[r_device][this->_readMarker[r_device][1]][DeviceBuffer_MaxByte + 0]; X++)
        dataArray[X] = this->_dataBuffer[r_device][this->_readMarker[r_device][1]][X];
      t_device = this->_dataBuffer[r_device][this->_readMarker[r_device][1]][DeviceBuffer_MaxByte + 1];
      bytesAvailable = this->_dataBuffer[r_device][this->_readMarker[r_device][1]][DeviceBuffer_MaxByte + 0];
      if (this->_readMarker[r_device][0])
        this->_readMarker[r_device][0]--;
      this->_readMarker[r_device][1]++;
      if (this->_readMarker[r_device][1] >= (DeviceBuffer_MaxPacket * DeviceBuffer_DeviceCount))
        this->_readMarker[r_device][1] = 0;
      return true;
    }
  }
  return false;
}
bool DeviceBuffer::write(uint8_t r_device, uint8_t t_device, uint8_t bytesToWrite, uint8_t *dataArray)
{
  if (this->_checkDevice(r_device) && this->_checkDevice(t_device) && bytesToWrite <= DeviceBuffer_MaxByte)
  {
    uint8_t WriteCount = 0;
    for (uint8_t X = 0; X < this->_readMarker[r_device][0]; X++)
    {
      if (this->_dataBuffer[r_device][(this->_readMarker[r_device][1] + X) % (DeviceBuffer_MaxPacket * DeviceBuffer_DeviceCount)][DeviceBuffer_MaxByte + 1] == t_device)
        WriteCount++;
    }
    if (WriteCount < DeviceBuffer_MaxPacket)
    {
      uint8_t WriteMarker = (this->_readMarker[r_device][1] + this->_readMarker[r_device][0]) % (DeviceBuffer_MaxPacket * DeviceBuffer_DeviceCount);
      for (uint8_t X = 0; X < bytesToWrite; X++)
        this->_dataBuffer[r_device][WriteMarker][X] = dataArray[X];
      this->_dataBuffer[r_device][WriteMarker][DeviceBuffer_MaxByte + 0] = bytesToWrite;
      this->_dataBuffer[r_device][WriteMarker][DeviceBuffer_MaxByte + 1] = t_device;
      this->_readMarker[r_device][0]++;
      return true;
    }
  }
  return false;
}

void DeviceBuffer::deviceFlush(uint8_t device)
{
  if (this->_checkDevice(device))
  {
    this->_readMarker[device][0] = 0;
    this->_readMarker[device][1] = 0;
  }
}

bool DeviceBuffer::_checkDevice(uint8_t device)
{
  if (device < DeviceBuffer_DeviceCount)
    return true;
  else
    return false;
}
#endif

// OneWire_Driver
//.h
#ifndef OneWire_Driver_h
#define OneWire_Driver_h

#include "Arduino.h"

class OneWire_Driver
{
public:
  void setPin(uint8_t);                     // set pin that will be used for onewire transmission
  void write(bool *, uint8_t);              // write message through onewire via bool bitarray
  uint8_t read(bool *, uint8_t &, uint8_t); // read from onewire as soon as pin is drawn low (1 = msg recieved, 0 = msg not found/corrupted, 2 = start condition too short/possibly glitch)

  void pinSet(bool);   // Set pin to inputPullup(false/0) or output(true/1)
  bool pinRead();      // Read currecnt state of pin
  void pinWrite(bool); // Write state to pin (OUTPUT)

private:
  volatile uint8_t *_pinPort_PIN;  // Pin read/state register
  volatile uint8_t *_pinPort_DDR;  // Pin set register (INPUT/OUTPUT)
  volatile uint8_t *_pinPort_PORT; // Pin write register (HIGH/LOW)

  uint8_t _pinMask;    // eg. B00100000
  uint8_t _pinMaskNot; // eg. B11011111
  bool _boot = false;
};

//.cpp

void OneWire_Driver::setPin(uint8_t pin)
{
  if (pin <= 19)
  {
    if (_boot)
    {
      *_pinPort_DDR &= _pinMaskNot;
      *_pinPort_PORT &= _pinMaskNot;
    }
    _boot = true;
    if (pin <= 7)
    {
      // DigitalPins (0 - 7)
      // Port D
      _pinPort_PIN = &PIND;
      _pinPort_DDR = (_pinPort_PIN + 1);
      _pinPort_PORT = (_pinPort_PIN + 2);
      _pinMask = (1 << pin);
      _pinMaskNot = ~_pinMask;
    }
    else if (pin <= 13)
    {
      // DigitalPins (8 - 13)
      // Port B
      _pinPort_PIN = &PINB;
      _pinPort_DDR = (_pinPort_PIN + 1);
      _pinPort_PORT = (_pinPort_PIN + 2);
      _pinMask = (1 << (pin - 8));
      _pinMaskNot = ~_pinMask;
    }
    else
    {
      // Analog Pins (A0 - A7)
      // Port C
      _pinPort_PIN = &PINC;
      _pinPort_DDR = (_pinPort_PIN + 1);
      _pinPort_PORT = (_pinPort_PIN + 2);
      _pinMask = (1 << (pin - 14));
      _pinMaskNot = ~_pinMask;
    }
    // Set Pin to input Pullup:
    *_pinPort_DDR &= _pinMaskNot;
    *_pinPort_PORT |= _pinMask;
  }
}
void OneWire_Driver::write(bool *bitArray, uint8_t length)
{
  if (_boot)
  {
    const uint8_t startBitDalay = 20;
    const uint8_t spikeBitDelay_us = 10;
    const uint8_t dataBitDelay_us = 20;

    pinSet(1);
    pinWrite(0);
    delayMicroseconds(startBitDalay);
    pinWrite(1);
    for (uint8_t x = 0; x < length; x++)
    {
      delayMicroseconds(spikeBitDelay_us);
      pinWrite(bitArray[x]);
      delayMicroseconds(dataBitDelay_us);
      pinWrite(!bitArray[x]);
    }
    delayMicroseconds(spikeBitDelay_us);
    pinWrite(1);
    delayMicroseconds(5);
    pinSet(0);
  }
}
uint8_t OneWire_Driver::read(bool *bitArray, uint8_t &bitCount, uint8_t maxLength)
{
  if (_boot)
  {
    uint16_t counter = 0;
    uint16_t delay = 20;
    uint16_t delayMin = 5;
    delay *= 2.5;
    delayMin *= 2.5;

    bitCount = 0;
    bool BitHold;

    while (!pinRead() && counter < delay)
      counter++;
    if (counter < delay && counter > delayMin)
    {
      delay = 15;
      while (bitCount <= maxLength)
      {
        delayMicroseconds(18);
        BitHold = pinRead();
        counter = 0;
        while (pinRead() == BitHold && counter < delay)
        {
          counter++;
          delayMicroseconds(2);
        }
        if (counter >= delay)
          return 1;
        else
        {
          if (bitCount < maxLength)
            bitArray[bitCount] = BitHold;
          bitCount++;
        }
      }
    }
    else if (!(counter > delayMin))
      return 2;
  }
  return 0;
}

void OneWire_Driver::pinSet(bool dir)
{
  if (dir)
  {
    // set pin to output
    *_pinPort_DDR |= _pinMask;
  }
  else
  {
    // set pin to input pullup
    *_pinPort_DDR &= _pinMaskNot;
    *_pinPort_PORT |= _pinMask;
  }
}
bool OneWire_Driver::pinRead()
{
  // return state of pin
  return (*_pinPort_PIN & _pinMask);
}
void OneWire_Driver::pinWrite(bool dir)
{
  if (dir)
  {
    // set pin HIGH
    *_pinPort_PORT |= _pinMask;
  }
  else
  {
    // set pin LOW
    *_pinPort_PORT &= _pinMaskNot;
  }
}
#endif

// BoolConverter
//.h
#ifndef BoolConverterClass_h
#define BoolConverterClass_h

#include "Arduino.h"

class BoolConverterClass
{
public:
  void compileVal(bool *, uint8_t &, uint8_t, uint8_t);       // bitArray, bitMarker, val(to compile), number of bits to compile
  void compileArray(bool *, uint8_t &, uint8_t *, uint8_t);   // bitArray, bitmarker, byte array(to compile), bytes to compile
  void decompileVal(bool *, uint8_t &, uint8_t &, uint8_t);   // bitArray, bitMarker, decompile result(val), number of bits to decompile
  void decompileArray(bool *, uint8_t &, uint8_t *, uint8_t); // bitArray, bitmarker, decompiled result(byte array), bytes to decompile
};
extern BoolConverterClass BoolConverter;

//.cpp

void BoolConverterClass::compileVal(bool *bitArray, uint8_t &bitMarker, uint8_t val, uint8_t bitsToCompile)
{
  for (uint8_t x = 0; x < bitsToCompile; x++)
  {
    bitArray[bitMarker] = (val & (1 << x));
    bitMarker++;
  }
}
void BoolConverterClass::compileArray(bool *bitArray, uint8_t &bitMarker, uint8_t *array, uint8_t bytesToCompile)
{
  for (uint8_t x = 0; x < bytesToCompile; x++)
  {
    bitArray[bitMarker] = (array[x] & (1 << 0));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 1));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 2));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 3));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 4));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 5));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 6));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 7));
    bitMarker++;
  }
}
void BoolConverterClass::decompileVal(bool *bitArray, uint8_t &bitMarker, uint8_t &val, uint8_t bitsToDecompile)
{
  val = 0;
  for (uint8_t x = 0; x < bitsToDecompile; x++)
  {
    val |= (bitArray[bitMarker] << x);
    bitMarker++;
  }
}
void BoolConverterClass::decompileArray(bool *bitArray, uint8_t &bitMarker, uint8_t *array, uint8_t bytesToDecompile)
{
  for (uint8_t x = 0; x < bytesToDecompile; x++)
  {
    array[x] = 0;
    array[x] |= (bitArray[bitMarker] << 0);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 1);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 2);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 3);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 4);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 5);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 6);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 7);
    bitMarker++;
  }
}
#endif


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

  bool _deviceConnected[OneWire_Master_DeviceCount];
  uint32_t _deviceTimer[OneWire_Master_DeviceCount];
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
  const uint16_t maxConnectionTimeout = 3000; // 3 second connection timeout
  if (tag_ip < _ipCount)
  {
    switch (command)
    {
    case 0:
      // disconnect command
      _deviceConnected[tag_ip] = false;
      _deviceTimer[tag_ip] = millis();
      // flush buffer for device:
      _DeviceBuffer.deviceFlush(tag_ip);
      break;
    case 1:
      // connect command
      _deviceConnected[tag_ip] = true;
      _deviceTimer[tag_ip] = millis();
      break;
    case 2:
      // check connect command
      if (_deviceConnected[tag_ip])
      {
        if ((millis() - _deviceTimer[tag_ip]) > maxConnectionTimeout)
        {
          _deviceConnected[tag_ip] = false;
          _deviceTimer[tag_ip] = millis();
          _DeviceBuffer.deviceFlush(tag_ip);
        }
      }
      break;
    }
    return _deviceConnected[tag_ip];
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


OneWire_Master Master;

void setup()
{
}

void loop()
{
  Master.run();
}