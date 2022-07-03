// OneWire Slave, RTX Protocol will be used in the future:
// OneWire_Slave
//.h
#ifndef OneWire_Slave_h
#define OneWire_Slave_h

#include "Arduino.h"
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

class OneWire_Slave
{
public:
  OneWire_Slave(uint8_t, uint8_t); // set pin and ip(3 bit/0 - 8) (pin,ip)

  bool run(); // return true if connected and false if not connected, keeps the connection active

  bool read(uint8_t &, uint8_t *, uint8_t &); // read if message available, also runs/keeps connection inputs((return)sender_ip, (return)dataArray, (return)dataBytes). returns true if message available, false otherwise
  bool write(uint8_t, uint8_t *, uint8_t);    // send message (reciever_ip, dataArray, dataBytes). returns true if message sent, false if not. updates writeError

  uint8_t writeError(); // return error code for previous write attempt, 0 = no error(message sent successfully), 1 = rx_ip not connected, 2 = rx_ip buffer full, 3 = other error(rtx disconnected etc.)
  uint8_t rtxError();   // return error code for rtx bus. 0 = connected/no error, 1 = not connected/no error or unknown error, 2 = not connected/rtx master error, 3 = not connected/duplicate ip

private:
  bool _parseInsert(bool *);                                // searches bus line for 5-bit message with correct ip. returns true if found, false otherwise. Input ((return)bitArray*), length ALWAYS 5 bits
  uint8_t _parseRead(bool *, uint8_t &, uint8_t, uint16_t); // wait for some time for a message to arrive. Input ((return)bitarray, (return)bitlength, maxBitsToRead, maxParseTime_us). returns 0 = message error/too long/corrupted, 1 = success, 2 = timeout/no msg
  bool _insert();                                           // wait for a empty "slot" on bus to read or send. waits predetermined time (approximately 30 - 60 us). return true if empty slot found, false otherwise

  // create private objects:
  OneWire_Driver _Pin;

  // variables:
  uint8_t _ip;
  bool _connected = false;  // connected state, true = connected to bus, false = not-connected to bus
  uint32_t _connectedTimer; // timer to keep track of last successful ack

  uint8_t _writeError = 0; // write error flag. 0 = no error(msg sent successfully), 1 = rx_ip not connected, 2 = rx_ip buffer full, 3 = other error(rtx disconnected, master error etc.)
  uint8_t _rtxError = 1;   // 0 = connected/no error, 1 = not connected/no error or unknown error, 2 = not connected/rtx master error, 3 = not connected/duplicate ip, 4 = timeouterror
};

//.cpp

OneWire_Slave::OneWire_Slave(uint8_t pin, uint8_t ip)
{
  _Pin.setPin(pin);
  _ip = (ip & B00000111);
}

bool OneWire_Slave::run()
{
  if (_connected && millis() - _connectedTimer >= 2000)
  {
    // watchdog timer, this is rarely needed, but if rtx-master starts hanging on responses, it is needed
    _connected = false;
    _rtxError = 4; // timeout error
  }
  if (!_connected || millis() - _connectedTimer >= 1000) // only run this loop if neccessary(ack or disconnected)
  {
    // set up vars:
    bool bitArrayRead[5];
    bool bitArraySend[5];
    uint8_t bitMarkerSend = 0;
    uint8_t bitMarkerRead = 3; // we already know ip is correct, skip 3 bits

    BoolConverter.compileVal(bitArraySend, bitMarkerSend, _ip, 3);
    BoolConverter.compileVal(bitArraySend, bitMarkerSend, 0, 2); // command 0 = connect, 1 = read, 2 = write, 3 = mx(not used)

    if (_parseInsert(bitArrayRead)) // if correct packet found
    {
      // "bitMarkerRead" is already at 3 bits, because we already know ip is correct
      uint8_t command;

      BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 1);

      if (!_connected && command) // check if duplicate ip
      {
        _rtxError = 3; // #flagError: duplicate ip
        return false;
      }

      _Pin.write(bitArraySend, bitMarkerSend); // send connect message

      if (_parseRead(bitArrayRead, bitMarkerRead, 5, 300) == 1)
      {
        if (bitMarkerRead == 5)
        {
          // prepare vars for bitarray:
          bitMarkerRead = 0;
          uint8_t ip;
          // command not needed, it is already created

          BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, ip, 3);
          BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 1);

          if (ip == _ip) // varify that return message is for us
          {
            if (command) // check if connected
            {
              _connectedTimer = millis();
              _connected = true;
              _rtxError = 0; // no error, connected
            }
            else
            {
              _connected = false;
              _rtxError = 1; // no error, not connected. Unknown error
            }
          }
          else
          {
            //#flagError: no return packet? rtx error?
          }
        }
        else
        {
          //#flagError: wrong packet size? rtx error?
        }
      }
      else
      {
        // no return packet at all? maybe corrupted?
        // #flagError: Maybe
      }
    }
  }
  return _connected;
}

bool OneWire_Slave::read(uint8_t &sender_ip, uint8_t *dataArray, uint8_t &dataBytes)
{
  bool msgAvailable = false; // return this value at the end
  if (_connected)            // no point in checking if not connected
  {
    bool bitArrayRead[88]; // prepare for largest possible msg - 88 bit
    bool bitArraySend[5];  // send message, only request needed - 5 bit
    // bitMarkers for read and send:
    uint8_t bitMarkerRead = 3; // set to 3 because we already know ip from "_parseInsert()"
    uint8_t bitMarkerSend = 0;

    BoolConverter.compileVal(bitArraySend, bitMarkerSend, _ip, 3);
    BoolConverter.compileVal(bitArraySend, bitMarkerSend, 1, 2); // request command for read, 1 = read request

    if (_parseInsert(bitArrayRead))
    {
      // get command. don't neet ip. remember that "bitMarkerRead" is already set to 3
      uint8_t command;
      BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 1); // decompile connected bit

      if (command) // verify that we are connected
      {
        BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 1); // decompile read availability

        if (command) // check if message available in buffer
        {
          _Pin.write(bitArraySend, bitMarkerSend); // send request

          if (_parseRead(bitArrayRead, bitMarkerRead, 88, 300) == 1) // we may recieve a packet up to 88 bits long
          {
            uint8_t ip;
            if (bitMarkerRead % 8 == 0 && bitMarkerRead >= 8)
            {
              dataBytes = ((bitMarkerRead / 8) - 1);
              bitMarkerRead = 0; // set to 0 to decompile

              BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, ip, 3);
              if (ip == _ip) // check if ip matches
              {
                BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 2); // use existing "command" var
                if (command == 1)
                {
                  // message available
                  BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, sender_ip, 3);
                  BoolConverter.decompileArray(bitArrayRead, bitMarkerRead, dataArray, dataBytes);
                  msgAvailable = true; // message successfully retreived
                }
                // update connection timer:
                _connectedTimer = millis();
                _rtxError = 0; // no error, connected
              }
              else
              {
                //#flagError: no return packet? rtx error?
              }
            }
            else if (bitMarkerRead == 5)
            {
              bitMarkerRead = 0; // set to 0 to decompile

              BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, ip, 3);
              if (ip == _ip) // check if ip matches
              {
                BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 2);
                if (command == 0)
                {
                  // no messages available, buffer is empty
                }
                // update connection timer, ip is correct
                _connectedTimer = millis();
                _rtxError = 0; // no error, connected
              }
              else
              {
                //#flagError: no return packet(skipped our device)? rtx error?
              }
            }
            else
            {
              //#flagError: wrong packet size? rtx error?
            }
          }
          else
          {
            // no return packet at all? maybe corrupted?
            // #flagError: Maybe
          }
        }
      }
      else
      {
        // not connected, better not to recompile a message and send out to reconnect:
        _rtxError = 1; // no error, not connected
        _connected = false;
      }
    }
  }
  run();               // run at the end of read to keep up connection
  return msgAvailable; // return if message was found
}
bool OneWire_Slave::write(uint8_t reciever_ip, uint8_t *dataArray, uint8_t dataBytes)
{
  _writeError = 3;      // #flagWrite: set as default, other error, not connected
  bool msgSent = false; // return this value at the end of function
  if (_connected)
  {
    bool bitArrayRead[5];                   // prepare for insert msg
    bool bitArraySend[(dataBytes * 8) + 8]; // send message, must be large enough for provided message
    // bitMarkers for read and send:
    uint8_t bitMarkerRead = 3; // set to 3 because we already know ip from "_parseInsert()"
    uint8_t bitMarkerSend = 0; // set sendMarker to 0 to compile message
    // prepare message to send:
    BoolConverter.compileVal(bitArraySend, bitMarkerSend, _ip, 3);                 // start packet with device ip
    BoolConverter.compileVal(bitArraySend, bitMarkerSend, 2, 2);                   // set command to send message(2)
    BoolConverter.compileVal(bitArraySend, bitMarkerSend, reciever_ip, 3);         // declare the ip that the message is being sent to
    BoolConverter.compileArray(bitArraySend, bitMarkerSend, dataArray, dataBytes); // compile actual data at the end

    if (_parseInsert(bitArrayRead)) // find insert message from master on bus
    {
      uint8_t command; // prepare variabe to check connection

      BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 1); // first bit is connection status(bitMarkerRead is already set to 3rd bit)

      if (command) // check that we are connected before sending packet
      {
        _Pin.write(bitArraySend, bitMarkerSend); // send msg

        if (_parseRead(bitArrayRead, bitMarkerRead, 5, 300) == 1) // wait for a response
        {
          uint8_t ip;
          if (bitMarkerRead == 5)
          {
            bitMarkerRead = 0; // set to 0 to decompile response

            BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, ip, 3); // decompile ip

            if (ip == _ip) // check if ip matches
            {
              BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 1); // decompile only first bit of command, 1 = successful write, 0 = unsuccessful

              if (command) //  write was successful
              {
                _writeError = 0; // #flagWrite: write was successful
                msgSent = true;  // set msgSent to true to return at the end of funct
              }
              else // write was unsuccessful
              {
                BoolConverter.decompileVal(bitArrayRead, bitMarkerRead, command, 1); // decompile the second bit of command, 0 = buffer full on reciever, 1 = reciever not connected
                _writeError = (command ? 1 : 2);                                     // #flagWrite: reason why write was successful("command" == 0/rx_buffer full, "command" == 1/rx not connected)
              }
              _connectedTimer = millis(); // update connect timer
              _rtxError = 0;              // no error, connected
            }
            else
            {
              //#flagError: no return packet? rtx error?
            }
          }
          else
          {
            //#flagError: wrong packet size? rtx error?
          }
        }
        else
        {
          // no return packet at all? maybe corrupted?
          // #flagError: Maybe
        }
      }
      else
      {
        // not connected, message can't be sent
        _rtxError = 1; // no error, not connected
        _connected = false;
      }
    }
  }
  run();
  return msgSent;
}

uint8_t OneWire_Slave::writeError()
{
  return _writeError;
}
uint8_t OneWire_Slave::rtxError()
{
  return _rtxError;
}

bool OneWire_Slave::_parseInsert(bool *bitArray)
{
  /*
  purpose of this function is to find
  a valid "welcome" packet from master device
  that would allow this slave device to do actions
  such as read, write, or connect/ack

  function would look for a 5 bit packet with
  its ip in it then return true

  by default, function will look for welcom packet
  for 20ms. This can be configured through the
  "delay" value below
  */
  const uint8_t delay = 20;        // look for packet for 20ms max
  const uint32_t timer = millis(); // mark the start of the function to keep track of time passed
  uint8_t bitCount;                // this will be used to return packet length from each read

  while (millis() - timer < delay) // run while time is still left
  {
    if (_insert()) // only from from bus if empty slot found
    {
      uint8_t readStatus = _parseRead(bitArray, bitCount, 5, 300); // find a packet in empty slot for 300us. There should be one, otherwise there is an issue with the master device
      // check read status, 1 means packet found, 2 means no packet found within 300us: master error, 0 means corrupted packet or wrong packet(try again):
      if (readStatus == 1) // successfully found 5-bit(or less) packet
      {
        if (bitCount == 5)
        {
          uint8_t ip;            // store ip of packet
          uint8_t bitMarker = 0; // use to decompile the first 3 bits(ip) of packet
          BoolConverter.decompileVal(bitArray, bitMarker, ip, 3);
          if (ip == _ip) // correct packet parsed, now return true
            return true;
          // else wrong packet/device, keep looking
        }
      }
      else if (readStatus == 2) // no packet found within 300us. something is wrong with master or no device connected
      {
        _rtxError = 2; // #flagError: master error
        return false;
      }
    }
  }
  // time out, could not find device
  //  #flagError: master error? could not find correct packet within "delay" ms?
  return false;
}
uint8_t OneWire_Slave::_parseRead(bool *bitArray, uint8_t &bitCount, uint8_t maxBitLength, uint16_t maxParseTime_us)
{
  uint16_t counter = 0;
  maxParseTime_us *= 2.5; // multiply by a factor of 2.5(magic number)
  while (counter < maxParseTime_us)
  {
    // delaying using lost clock cylces. Aprox 16 clock pulses per loop all together
    counter++;
    if (!_Pin.pinRead()) // if pin goes low, start condition potentially
    {
      uint8_t readStatus = _Pin.read(bitArray, bitCount, maxBitLength); // 1 = msg found, 0 = error/no msg/corrupted msg, 2 = false start condition
      if (readStatus == 0)
        return 0; // no msg or corrupted msg
      if (readStatus == 1)
        return 1; // msg found, return 1
      // otherwise, keep looking for msg until time runs out
    }
  }
  return 2; // time ran out, return 2
}
bool OneWire_Slave::_insert()
{
  // by default, find slot of 30 - 60 us(should be 40 us):
  uint16_t counter = 0;
  const uint16_t delay_us = 40; // (magic number)
  while (counter < delay_us && _Pin.pinRead())
  {
    counter++;
    delayMicroseconds(2); // delay for 2 us, it actually turns out to be about 1 us
  }
  if (counter >= delay_us) // slot found, return true
    return true;
  return false; // return false otherwise
}

#endif

// use atem lib:
// AtemControl
//.h
#ifndef AtemControl_h
#define AtemControl_h

#define AtemDefaultPort 9910

// #define AtemSerialDebug

#include "Arduino.h"

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

class AtemControl
{
public:
  AtemControl(IPAddress, uint16_t); // setup atem. inputs (ip, arduinoPort(optianal))

  bool run(); // run atem connection. run often enough. returns true if connected, false otherwise

  void doCut();               // send cut command on atem
  void doAuto();              // send auto command on atem
  void previewInput(uint8_t); // change preview input of atem
  void programInput(uint8_t); // change program input on atem
  void ftb();                 // fade to black on atem

  void sendCMD(char *, uint8_t *, uint8_t); // send command packet to ATEM. ATEM api is needed for this. inputs(commandChar(4 bytes), commandBytes(up to 16 bytes))
  uint8_t tally(uint8_t);                   // input which tally is needed. returns tally state. updates each time packet recieved from atem
private:
  IPAddress _atemIp;
  uint16_t _udpPort;

  bool _connected = false;
  char _buffer[96];
  uint16_t _sessionId;
  uint16_t _lastPacketId;
  uint16_t _sendPacketId;
  uint16_t _last_TlIn_packetId;
  uint32_t _connectionTimer;
  bool _packetCount;

  uint8_t _atemTally[8];

  EthernetUDP _atemUDP;
};

//.cpp

AtemControl::AtemControl(IPAddress atemIp, uint16_t udpPort = 6565)
{
  _atemIp = atemIp;
  _udpPort = udpPort;
  _connectionTimer = millis();
}

bool AtemControl::run()
{
  if (_connected)
  {
    if (millis() - _connectionTimer >= 1000)
    {
      if (!_packetCount)
      {
#ifdef AtemSerialDebug
        Serial.println("disconnected: timeout");
#endif
        _connected = false;
        _connectionTimer = millis();
        return false;
      }
      else
      {
        _packetCount = false;
        _connectionTimer = millis();
      }
    }
    // run packet check:
    uint16_t ParsedPacketSize = _atemUDP.parsePacket();
    if (_atemUDP.available() && ParsedPacketSize)
    {
      bool sendACK = false; // only send ack packet if we recieved correct packet

      _atemUDP.read(_buffer, 12);                                     // retrieve packet header
      uint16_t packetSize = word(_buffer[0] & B00000111, _buffer[1]); // get declared packet size to verify there were no errors
      uint16_t packetId = word(_buffer[10], _buffer[11]);
      uint8_t command = _buffer[0] & B11111000;
      bool commandACK = command & B00001000;
#ifdef AtemSerialDebug
      Serial.print("rPID: ");
      Serial.print(packetId);
      Serial.print(" ePID: ");
      Serial.println(_lastPacketId + 1);
#endif

      // increment packet id if we recieved worrecnt packet:
      if (packetId == _lastPacketId + 1)
      {
        _lastPacketId++;
        sendACK = true;
      }

      if (ParsedPacketSize == packetSize) // verify packet sizes match, otherwise flush buffer and exit
      {
        _packetCount = true; // successful packet, mark packetCount

        if (packetSize > 12 && packetId > _last_TlIn_packetId)
        {
          bool parsePacketDone = false;
          uint16_t indexPointer = 12; // we already read first 12 bytes
          while (indexPointer < packetSize && !parsePacketDone)
          {
            _atemUDP.read(_buffer, 2);
            uint16_t commandSize = word(0, _buffer[1]);
            if (commandSize > 2 && commandSize <= sizeof(_buffer))
            {
              _atemUDP.read(_buffer, commandSize - 2);
              char cmd_str[] =
                  {_buffer[2],
                   _buffer[3],
                   _buffer[4],
                   _buffer[5],
                   '\0'};
              if (!strcmp(cmd_str, "TlIn"))
              {
                // if (packetId > _last_TlIn_packetId)
                // {
                sendACK = true;
                _last_TlIn_packetId = packetId;
                _lastPacketId = packetId;
                _atemTally[0] = _buffer[8];
                _atemTally[1] = _buffer[9];
                _atemTally[2] = _buffer[10];
                _atemTally[3] = _buffer[11];
                _atemTally[4] = _buffer[12];
                _atemTally[5] = _buffer[13];
                _atemTally[6] = _buffer[14];
                _atemTally[7] = _buffer[15];
#ifdef AtemSerialDebug
                Serial.print("tally: ");
                for (uint8_t x = 0; x < 8; x++)
                  Serial.print(_atemTally[x]);
                Serial.println();
#endif
                // }
              }
              indexPointer += commandSize;
            }
            else
            {
              parsePacketDone = true;
              while (_atemUDP.available())
                _atemUDP.read(_buffer, sizeof(_buffer));
            }
          }
        }
        if (commandACK && sendACK)
        {
#ifdef AtemSerialDebug
          Serial.print("ACK: ");
          Serial.println(_lastPacketId);
#endif
          const byte answerACK[12] =
              {((12 / 256) | 0x80), (12 % 256), 0x80,
               _sessionId, (_lastPacketId / 256),
               (_lastPacketId % 256), 0x00, 0x00,
               0x00, 0x00, 0x00, 0x00};

          _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
          _atemUDP.write(answerACK, 12);
          _atemUDP.endPacket();
        }
      }
      else
      {
        while (_atemUDP.available())
          _atemUDP.read(_buffer, sizeof(_buffer));
      }
    }
  }
  else
  {
    _atemUDP.stop();
    if (millis() - _connectionTimer >= 500) // don't spam reconnection
    {
#ifdef AtemSerialDebug
      Serial.print("connectAttempt: ");
#endif
      _atemUDP.begin(_udpPort);
      { // send connected packet:
        const byte connectPacket[] =
            {0x10, 0x14, 0x53, 0xAB, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x3A,
             0x00, 0x00, 0x01, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00};
        while (_atemUDP.available()) // flush udp buffer
          _atemUDP.read(_buffer, sizeof(_buffer));
        // send packet:
        _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
        _atemUDP.write(connectPacket, 20);
        _atemUDP.endPacket();
      }

      // listen for response:
      uint8_t udpParsePacket = _atemUDP.parsePacket();
      uint32_t parseTimeout = micros();
      while (micros() - parseTimeout < 10000 && udpParsePacket != 20) // keep looking for packet for 10(ms)
        udpParsePacket = _atemUDP.parsePacket();

      // check if response found
      if (udpParsePacket == 20)
      {
#ifdef AtemSerialDebug
        Serial.println("connected");
#endif
        // we assume this packet is from atem, no time to verify
        _connected = true;
        _sendPacketId = 1; // wierd but sendPacketId is expected to start from 1. wont work if it is 0
        _lastPacketId = 0;
        _last_TlIn_packetId = 0;

        // get session id from packet:
        _atemUDP.read(_buffer, 20);
        _sessionId = _buffer[15]; // this is where our session id is stored

        { // send conect response packet to confirm connection:
          const byte connectResponsePacket[] =
              {0x80, 0x0c, 0x53, 0xab, 0x00, 0x00,
               0x00, 0x00, 0x00, 0x03, 0x00, 0x00};
          // send packet:
          _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
          _atemUDP.write(connectResponsePacket, 12);
          _atemUDP.endPacket();
        }
        _packetCount = true;
      }
      else
      {
#ifdef AtemSerialDebug
        Serial.println("notConnected");
#endif
        // we did not get successful connection:
        _atemUDP.stop();
      }
      _connectionTimer = millis();
    }
  }
  return _connected;
}

void AtemControl::doCut()
{
  uint8_t cmdPacket[] =
      {0x00, 0x9d, 0x59, 0x99};
  sendCMD("DCut", cmdPacket, 4);
}
void AtemControl::doAuto()
{
  uint8_t cmdPacket[] =
      {0x00, 0xad, 0x59, 0x99};
  sendCMD("DAut", cmdPacket, 4);
}
void AtemControl::previewInput(uint8_t input)
{
  uint8_t cmdPacket[] =
      {0x00, 0x75, 0x00, input};
  sendCMD("CPvI", cmdPacket, 4);
}
void AtemControl::programInput(uint8_t input)
{
  uint8_t cmdPacket[] =
      {0x00, 0x75, 0x00, input};
  sendCMD("CPgI", cmdPacket, 4);
}
void AtemControl::ftb()
{
  uint8_t cmdPacket[] =
      {0x00, 0x5b, 0x49, 0x97};
  sendCMD("FtbA", cmdPacket, 4);
}

void AtemControl::sendCMD(char *cmd_str, uint8_t *cmd_packet, uint8_t cmd_bytes)
{
  if (_connected) // only send command if we are connected. otherwise, arduino can freeze up
  {
    if (cmd_bytes <= 16)
    {

#ifdef AtemSerialDebug
      Serial.print("sendCMD: ");
      Serial.print(cmd_str);
      Serial.print(" bytes: ");
      for (uint8_t x = 0; x < cmd_bytes; x++)
      {
        Serial.print(cmd_packet[x]);
        Serial.print(" ");
      }
      Serial.println();
#endif

      const uint16_t packetSize = (20 + cmd_bytes);
      uint8_t answerPacket[packetSize];
      for (uint16_t x = 0; x < packetSize; x++) // clear entire buffer
        answerPacket[x] = 0;

      // declare packetSize:
      answerPacket[0] = packetSize / 256;
      answerPacket[1] = packetSize % 256;
      answerPacket[0] |= B00001000; // << api

      // declare session id and packet id:
      answerPacket[2] = 0x80; // << api
      answerPacket[3] = _sessionId;
      answerPacket[10] = _sendPacketId / 256;
      answerPacket[11] = _sendPacketId % 256;

      // declare cmd packet size:
      answerPacket[12] = (8 + cmd_bytes) / 256;
      answerPacket[13] = (8 + cmd_bytes) % 256;

      // cmd str:
      for (uint8_t x = 0; x < 4; x++)
        answerPacket[16 + x] = cmd_str[x];

      // command packet bytes:
      for (uint8_t x = 0; x < cmd_bytes; x++)
        answerPacket[20 + x] = cmd_packet[x];

      // send packet:
      _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
      _atemUDP.write(answerPacket, packetSize);
      _atemUDP.endPacket();

      // increment send packet id:
      _sendPacketId++;
    }
  }
}
uint8_t AtemControl::tally(uint8_t input)
{
  if (input < 8)
    return _atemTally[input];
  return 0;
}

#endif

// include VarPar, useful for state changes:
//VarPar
//.h
#ifndef VarPar_h
#define VarPar_h

#include "Arduino.h"

class Par_bool
{
public:
    Par_bool(bool var = false)
    {
        _var = var;
        _prevar = var;
    }
    Par_bool &operator=(bool var)
    {
        _var = var;
        return *this;
    }
    operator bool()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    bool _var;
    bool _prevar;
};

class Par_uint8_t
{
public:
    Par_uint8_t(uint8_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_uint8_t &operator=(uint8_t var)
    {
        _var = var;
        return *this;
    }
    operator uint8_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    uint8_t _var;
    uint8_t _prevar;
};

class Par_uint16_t
{
public:
    Par_uint16_t(uint16_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_uint16_t &operator=(uint16_t var)
    {
        _var = var;
        return *this;
    }
    operator uint16_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    uint16_t _var;
    uint16_t _prevar;
};

class Par_uint32_t
{
public:
    Par_uint32_t(uint32_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_uint32_t &operator=(uint32_t var)
    {
        _var = var;
        return *this;
    }
    operator uint32_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    uint32_t _var;
    uint32_t _prevar;
};

class Par_int8_t
{
public:
    Par_int8_t(int8_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_int8_t &operator=(int8_t var)
    {
        _var = var;
        return *this;
    }
    operator int8_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    int8_t _var;
    int8_t _prevar;
};

class Par_int16_t
{
public:
    Par_int16_t(int16_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_int16_t &operator=(int16_t var)
    {
        _var = var;
        return *this;
    }
    operator int16_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    int16_t _var;
    int16_t _prevar;
};

class Par_int32_t
{
public:
    Par_int32_t(int32_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_int32_t &operator=(int32_t var)
    {
        _var = var;
        return *this;
    }
    operator int32_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    int32_t _var;
    int32_t _prevar;
};

//.cpp
//#include "VarPar.h"
//#include "Arduino.h"

#endif

// set network credentials:
// mac address of arduino:
byte network_mac[] = {0x90, 0xA2, 0xDA, 0x00, 0xE8, 0xE9};
// ip address of arduino:
IPAddress network_ip(192, 168, 86, 44);
// set atem ip location:
IPAddress atem_network_ip(192, 168, 86, 68);

// set atem instance:
AtemControl atem(atem_network_ip); // set ip

// set OneWire rtx:
OneWire_Slave rtx(8, 1); // set pin(8) and ip(1), 1 is default for ethernet
const uint8_t pedal_ip = 3;

// set connection states and tally:
Par_bool atem_connected = false;
Par_uint8_t atem_programTally = 0;
Par_uint8_t atem_previewTally = 0;

void setup()
{
  Ethernet.begin(network_mac, network_ip);
}

void loop()
{
  rtxHandle();
  atemHandle();
}

void rtxHandle()
{
  uint8_t sender_ip;
  uint8_t dataArray[10]; // prepare for max possible bytes
  uint8_t dataBytes;
  if (rtx.read(sender_ip, dataArray, dataBytes))
  {
    if (dataBytes && sender_ip == pedal_ip)
    {
      switch (dataArray[0])
      {
        // command 0: null
      case 1:
        if (dataBytes == 1)
          atem.doCut();
        break;
      case 2:
        if (dataBytes == 1)
          atem.doAuto();
        break;
      case 3:
        if (dataBytes == 2)
          atem.previewInput(dataArray[1]);
        break;
      case 4:
        if (dataBytes == 2)
          atem.programInput(dataArray[1]);
        break;
      case 5:
        if (dataBytes == 1)
          atem.ftb();
        break;
      }
    }
  }
  static uint32_t sendTimer = millis();
  bool change = false;
  if (atem_connected.change())
    change = true;
  if (atem_previewTally.change())
    change = true;
  if (atem_programTally.change())
    change = true;

  if (change || millis() - sendTimer >= 500)
  {
    sendTimer = millis();
    uint8_t dataArray[3];
    dataArray[0] = 0;
    dataArray[0] = atem_connected;
    dataArray[1] = atem_programTally;
    dataArray[2] = atem_previewTally;
    rtx.write(pedal_ip, dataArray, 3);
  }
}

void atemHandle()
{
  atem_connected = atem.run();

  uint8_t atem_programTally_newVal = 0;
  uint8_t atem_previewTally_newVal = 0;

  for (uint8_t x = 0; x < 8; x++)
  {
    bitWrite(atem_programTally_newVal, x, bitRead(atem.tally(x), 0));
    bitWrite(atem_previewTally_newVal, x, bitRead(atem.tally(x), 1));
  }

  atem_programTally = atem_programTally_newVal;
  atem_previewTally = atem_previewTally_newVal;
}