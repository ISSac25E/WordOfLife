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

// NeoPixel Lib:
// NeoPixel
//.h
#ifndef NeoPixel_h
#define NeoPixel_h

#include "Arduino.h"

class NeoPixelDriver
{
public:
  NeoPixelDriver(uint8_t, uint8_t *, uint8_t); // set pin of neoPixel, give array to driver for neopixel CHANNELS. array must be double the size of neo pixel channels. input number of neopixels intended to be used. inputs(neoPixel_pin, array(double the number of pixelChannels), numberOfPixelChannels)

  void run(bool);               // runs neopixel driver. if pixel values change, driver will write immediatly. Inputs (forceWrite(default = false))
  void writeInterval(uint16_t); // set how often for neopixels to be forced written. default is 100ms
  uint8_t &pixelRef(uint8_t);   // returns reference to buffer for given pixel. inputs(pixelChannel). outputs reference to pixel/address location

private:
  uint8_t _lastPixel = 0;
  uint16_t _writeInterval_ms = 100;

  uint8_t *_pixelBuffer;
  uint8_t _pixelCount;

  volatile uint8_t *_pinPort_PIN;  // Pin read/state register
  volatile uint8_t *_pinPort_DDR;  // Pin set register (INPUT/OUTPUT)
  volatile uint8_t *_pinPort_PORT; // Pin write register (HIGH/LOW)

  uint8_t _pinMask;    // eg. B00100000
  uint8_t _pinMaskNot; // eg. B11011111

  friend class NeoPixel;
};

class NeoPixel
{
public:
  NeoPixel(NeoPixelDriver &, uint8_t); // give instance of NeoPixelDriver and state which pixel cahnnel is needed(optional). Automatically selects next pixelChannel on list. inputs(NeoPixelDriver instance, pixelChannel(optional))
  operator uint8_t &()
  {
    return *_pixelAddress;
  }
  NeoPixel &operator=(uint8_t val)
  {
    *_pixelAddress = val;
  }

private:
  uint8_t *_pixelAddress;
};

//.cpp

NeoPixelDriver::NeoPixelDriver(uint8_t pin, uint8_t *buffer, uint8_t pixelCount)
{
  pin %= 20; // make sure pin does not go out of bounds
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

  asm volatile(
      "st %a1, %3 \n\t"
      "st %a0, %2\n" ::
          "e"(_pinPort_DDR),
      "e"(_pinPort_PORT),
      "r"(*_pinPort_DDR | _pinMask),
      "r"(*_pinPort_PORT & _pinMaskNot));

  _pixelCount = pixelCount;
  _pixelBuffer = buffer;
}

void NeoPixelDriver::run(bool forceWrite = false)
{
  static uint32_t pixelWriteTimer = millis();
  static uint32_t frameCoolDownTimer = micros();
  bool pixelWriteInterval = (millis() - pixelWriteTimer >= _writeInterval_ms);
  bool pixelChange = false;
  if (!forceWrite && !pixelWriteInterval)
  {
    for (uint8_t x = 0; x < _pixelCount; x++)
      if (_pixelBuffer[x] != _pixelBuffer[x + _pixelCount])
      {
        pixelChange = true;
        break;
      }
  }
  if (forceWrite || pixelChange || pixelWriteInterval)
  {
    pixelWriteTimer = millis();
    for (uint8_t x = 0; x < _pixelCount; x++)
      _pixelBuffer[x + _pixelCount] = _pixelBuffer[x];
    while (micros() - frameCoolDownTimer < 100) // wait for frame cooldown, otherwise will lead to a miss write
      ;
    noInterrupts(); // turn off all interupts
    uint8_t lowBit = (*_pinPort_PORT & _pinMaskNot);
    uint8_t highBit = (*_pinPort_PORT | _pinMask);
    for (uint8_t x = 0; x < _pixelCount; x++)
    {
      for (uint8_t y = 0; y < 8; y++)
      {
        if (bitRead(_pixelBuffer[x], 7 - y))
        {
          // high bit
          asm volatile(
              "st %a0, %2 \n\t"
              ".rept 9 \n\t"
              "nop \n\t"
              ".endr \n\t"
              "st %a0, %1 \n\t"
              ".rept 5 \n\t"
              "nop \n\t"
              ".endr \n" ::
                  "e"(_pinPort_PORT),
              "r"(lowBit),
              "r"(highBit));
        }
        else
        {
          // low bit
          asm volatile(
              "st %a0, %2 \n\t"
              ".rept 5 \n\t"
              "nop \n\t"
              ".endr \n\t"
              "st %a0, %1 \n\t"
              ".rept 13 \n\t"
              "nop \n\t"
              ".endr \n" ::
                  "e"(_pinPort_PORT),
              "r"(lowBit),
              "r"(highBit));
        }
      }
    }
    interrupts();
    frameCoolDownTimer = micros();
  }
}
void NeoPixelDriver::writeInterval(uint16_t setWriteInterval)
{
  _writeInterval_ms = setWriteInterval;
}
uint8_t &NeoPixelDriver::pixelRef(uint8_t pixelChannel)
{
  return _pixelBuffer[pixelChannel];
}

NeoPixel::NeoPixel(NeoPixelDriver &NeoPixelDriver_point, uint8_t pixelChannel = 255)
{
  if (pixelChannel == 255)
  {
    _pixelAddress = &NeoPixelDriver_point._pixelBuffer[NeoPixelDriver_point._lastPixel++];
  }
  else
  {
    _pixelAddress = &NeoPixelDriver_point._pixelBuffer[pixelChannel];
    NeoPixelDriver_point._lastPixel = pixelChannel + 1;
  }
}
#endif

// LedMacro and SequenceBuild:
// LedMacro
//.h
#ifndef LedMacro_h
#define LedMacro_h

#include "Arduino.h"

class LedMacro
{
  /*
    note:
      - reference is required by every macro because it can be used as a form of identification
     */
public:
  bool run();   // run led macro. return true if ready for another macro. false if still running a macro
  bool ready(); // returns if ready for another macro/equivilent of run, just without running the macro(faster)
  void stop();  // stop macros, (ready with return true)

  // transition functions:
  void lineEase(uint8_t &, uint8_t, uint8_t);  // linear ease/fade, input(value(reference), target(0 - 255), frames(frames to run macro))
  void quadEase(uint8_t &, uint8_t, uint8_t);  // quadratic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames)
  void cubicEase(uint8_t &, uint8_t, uint8_t); // cubic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames)
  void set(uint8_t &, uint8_t, uint16_t);      // sets value to target and delays set amount(ms). inputs(value(reference), target(0 - 255), delay(ms))
  void delay(uint8_t &, uint16_t);             // delay all macros for set time. inputs(value(reference), delay(ms))

  void fps(uint16_t); // set fps(frames(refreshes) per second). default is set on 60 fps. fps is only used for linear and easing

  uint8_t *refValue();

private:
  float _quad_easeinout(float);  // calculate quadEase function. inputs(t(time)). outputs v(horizontal percentage)
  float _cubic_easeinout(float); // calculate cubicEase function. inputs(t(time)). outputs v(horizontal percentage)

  uint8_t _lerp(int16_t, int16_t, float); // Linear Interpolation function, inputs (point_a, point_b, percentage(0.0 - 1.0)). outputs value between point a and point b relative to percentage input

  bool _macroRun = false; // determines wether a macro is currently running

  uint8_t _transitionType; // use to distinguish between "set"(0), "linear"(1), "quad"(2), and "cubic"(3)
  uint8_t _startVal;       // start value captured at the call of a transition. laster will be used with "_lerp"
  uint8_t _endVal;         // end value captured at the call of a transition. also will be later used with "_lerp"
  uint8_t _frameCount;     // used to keep track how many frames completed during transition. always set to 0 when transition called
  uint8_t _totalFrames;    // set how many total frames transition needs to run for
  uint16_t _rate_ms = 16;  // delay between each transition step in ms. defaults to aprox 60 fps(16.667ms)
  uint16_t _nextDelay;     // value set when transition functions called. used during run
  uint32_t _timer;         // timer for keeping track of transitions. used with "millis()"

  uint8_t *_refValue; // pointer of referenced value giving in each transition function
};

class LedMacroManager
{
public:
  LedMacroManager(LedMacro *, uint8_t); // takes an array of LedMacro's and controls them. inputs(LedMacro(pointer), numberOfLedMacro(in the array))

  void run();            // runs all LedMacros in background
  bool run(uint8_t &);   // runs all LedMacros in background
  bool ready(uint8_t &); // returns if macro ready using reference value. inputs(refValue(used and value identification))
  void stop(uint8_t &);  // stop macro for given referenced value

  // transition functions:
  bool lineEase(uint8_t &, uint8_t, uint8_t);  // linear ease/fade, input(value(reference), target(0 - 255), frames(frames to run macro)). outputs true if macro successfully attached to val, false otherwise
  bool quadEase(uint8_t &, uint8_t, uint8_t);  // quadratic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames). outputs true if macro successfully attached to val, false otherwise
  bool cubicEase(uint8_t &, uint8_t, uint8_t); // cubic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames). outputs true if macro successfully attached to val, false otherwise
  bool set(uint8_t &, uint8_t, uint16_t);      // sets value to target and delays set amount(ms). inputs(value(reference), target(0 - 255), delay(ms)). outputs true if macro successfully attached to val, false otherwise
  bool delay(uint8_t &, uint16_t);             // delay all macros for set time. inputs(value(reference), delay(ms)). outputs true if macro successfully attached to val, false otherwise

  void fps(uint16_t); // sets GLOBAL fps(frames(refreshes) per second) for all LedMacros. default is set on 60 fps. fps is only used for linear and easing transitions
private:
  LedMacro *_LedMacro_Pointer;
  uint8_t _numberOfInstances;
};

//.cpp

bool LedMacro::run()
{
  if (_macroRun)
  {
    if (millis() - _timer >= _nextDelay)
    {
      if (_transitionType == 0) // setDelay transition
      {
        _macroRun = false; // delay already done, value set at the beginning of transition call. disable macros
      }
      else if (_transitionType == 1) // linearFade transition
      {
        _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
        // _timer = millis(); // relative timing, use if absolute timing becomes unstable
        _frameCount++;                                                                      // increment framecount before setting value
        *_refValue = _lerp(_startVal, _endVal, ((float)_frameCount / (float)_totalFrames)); // calculate linear fade according to framecount/frames

        if (_totalFrames <= _frameCount) // check if transition is done. if so, set "macroRun" to false
          _macroRun = false;
      }
      else if (_transitionType == 2) // quadraticEase transition
      {
        _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
        // _timer = millis(); // relative timing, use if absolute timing becomes unstable
        _frameCount++;                                                                                     // increment framecount before setting value
        *_refValue = _lerp(_startVal, _endVal, _quad_easeinout((float)_frameCount / (float)_totalFrames)); // calculate linear fade according to framecount/frames

        if (_totalFrames <= _frameCount) // check if transition is done. if so, set "macroRun" to false
          _macroRun = false;
      }
      else if (_transitionType == 3) // cubicEase transition
      {
        _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
        // _timer = millis(); // relative timing, use if absolute timing becomes unstable
        _frameCount++;                                                                                      // increment framecount before setting value
        *_refValue = _lerp(_startVal, _endVal, _cubic_easeinout((float)_frameCount / (float)_totalFrames)); // calculate linear fade according to framecount/frames

        if (_totalFrames <= _frameCount) // check if transition is done. if so, set "macroRun" to false
          _macroRun = false;
      }
      else                 // unknown transition
        _macroRun = false; // set "macroRun" to false so we dont end up with a stuck macro
    }
  }
  return !_macroRun; // return 'not' wether macros are currently running
}
bool LedMacro::ready()
{
  return !_macroRun; // return 'not' wether macros are currently running
}
void LedMacro::stop()
{
  _macroRun = false;
}

void LedMacro::lineEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  _refValue = &ref; // save reference value
  if (frames)       // only start macro if there are frames. if no frames and macro will run, divide by 0 error will occur
  {
    _macroRun = true;
    _transitionType = 1;    // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue; // start value is the current refValue
    _endVal = target;       // end value is target value
    _totalFrames = frames;
    _frameCount = 0; // reset frame count
    _nextDelay = _rate_ms;
    _timer = millis(); // reset timer for transition
  }
  else
  {
    _macroRun = false;
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::quadEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  _refValue = &ref; // save reference value
  if (frames)       // only start macro if there are frames. if no frames and macro will run, divide by 0 error will occur
  {
    _timer = millis(); // reset timer for transition
    _macroRun = true;
    _transitionType = 2;    // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue; // start value is the current refValue
    _endVal = target;       // end value is target value
    _totalFrames = frames;
    _frameCount = 0; // reset frame count
    _nextDelay = _rate_ms;
  }
  else
  {
    _macroRun = false;
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::cubicEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  _refValue = &ref; // save reference value
  if (frames)       // only start macro if there are frames. if no frames and macro will run, divide by 0 error will occur
  {
    _timer = millis(); // reset timer for transition
    _macroRun = true;
    _transitionType = 3;    // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue; // start value is the current refValue
    _endVal = target;       // end value is target value
    _totalFrames = frames;
    _frameCount = 0; // reset frame count
    _nextDelay = _rate_ms;
  }
  else
  {
    _macroRun = false;
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::set(uint8_t &ref, uint8_t target, uint16_t delay_ms)
{
  _refValue = &ref; // save reference value
  *_refValue = target;
  if (delay_ms) // pointless without a delay
  {
    _timer = millis();
    _macroRun = true;
    _transitionType = 0;
    _nextDelay = delay_ms;
  }
  else
  {
    _macroRun = false;
  }
}
void LedMacro::delay(uint8_t &ref, uint16_t delay_ms)
{
  _refValue = &ref; // save reference value
  if (delay_ms)     // pointless without a delay
  {
    _timer = millis();
    _macroRun = true;
    _transitionType = 0;
    _nextDelay = delay_ms;
  }
  else
  {
    _macroRun = false;
  }
}

void LedMacro::fps(uint16_t set_fps)
{
  if (set_fps)
    _rate_ms = (1000 / set_fps);
  else
    _rate_ms = 1;
}

uint8_t *LedMacro::refValue()
{
  return _refValue;
}

float LedMacro::_quad_easeinout(float time)
{
  time *= 2.0;
  if (time < 1.0)                      // ease in(first half)
    return (float)(time * time) / 2.0; // devide by 2 because we multiplied by 2
  time -= 1.0;
  return (float)((-1.0 * time * (time - 2.0) + 1.0) / 2.0);
}
float LedMacro::_cubic_easeinout(float time)
{
  time *= 2.0;
  if (time < 1.0) // ease in(first half)
    return (float)(time * time * time) / 2.0;
  time -= 2.0;
  return (float)(((time * time * time) + 2.0) / 2.0);
}

uint8_t LedMacro::_lerp(int16_t point_a, int16_t point_b, float time)
{
  return ((((int16_t)point_b - (int16_t)point_a) * (float)time) + (int16_t)point_a);
}

LedMacroManager::LedMacroManager(LedMacro *LedMacro_Pointer, uint8_t numberOfInstances)
{
  _LedMacro_Pointer = LedMacro_Pointer;
  _numberOfInstances = numberOfInstances;
}

void LedMacroManager::run()
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
    _LedMacro_Pointer[x].run();
}
bool LedMacroManager::run(uint8_t &ref)
{
  bool returnValue = true;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    _LedMacro_Pointer[x].run();
    if (_LedMacro_Pointer[x].refValue() == &ref)
      returnValue = _LedMacro_Pointer[x].ready();
  }
  return returnValue;
}
bool LedMacroManager::ready(uint8_t &ref)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (_LedMacro_Pointer[x].refValue() == &ref)
      return _LedMacro_Pointer[x].ready();
  }
  return true;
}
void LedMacroManager::stop(uint8_t &ref)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (_LedMacro_Pointer[x].refValue() == &ref)
      _LedMacro_Pointer[x].stop();
  }
}

// transition functions:
bool LedMacroManager::lineEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].lineEase(ref, target, frames);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].lineEase(ref, target, frames);
    return true;
  }
  return false;
}
bool LedMacroManager::quadEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].quadEase(ref, target, frames);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].quadEase(ref, target, frames);
    return true;
  }
  return false;
}
bool LedMacroManager::cubicEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].cubicEase(ref, target, frames);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].cubicEase(ref, target, frames);
    return true;
  }
  return false;
}
bool LedMacroManager::set(uint8_t &ref, uint8_t target, uint16_t delay_ms)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].set(ref, target, delay_ms);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].set(ref, target, delay_ms);
    return true;
  }
  return false;
}
bool LedMacroManager::delay(uint8_t &ref, uint16_t delay_ms)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].delay(ref, delay_ms);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].delay(ref, delay_ms);
    return true;
  }
  return false;
}

void LedMacroManager::fps(uint16_t set_fps)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
    _LedMacro_Pointer[x].fps(set_fps);
}
#endif

// SequenceBuild
//.h
/*
  changes from v1.0.1:
    - you can now set one of two priorities with sequences(normal-priority or high-priority)
      if any sequence with normal-priority is called and a high-priority sequence is currently running,
      the called sequence will be ignored until the running sequence is stopped or resets its priority
 */
#ifndef SequenceBuild_h
#define SequenceBuild_h

#include "Arduino.h"

class SequenceBuild
{
public:
  void run(); // runs in the background keeps up the sequence

  void setSequence(uint8_t (*)(uint8_t), uint8_t, bool);         // set a new sequence and start running it only if it isn't already running and a higher priority sequence is not currently running. input(sequenceFunct, indexToStartFrom, forceFirstSter(default false/wait for current sequence to finish))
  void setSequence(uint8_t (*)(uint8_t));                        // set sequence without running it, will stop whichever sequence is currently running(except priority sequences). Use "start" to begin sequence
  void setPrioritySequence(uint8_t (*)(uint8_t), uint8_t, bool); // set and start a priority sequence. can only be interrupted with "stop()" or another priority sequence
  void setPrioritySequence(uint8_t (*)(uint8_t));                // set a priority sequence. use "start" to begin
  void resetPriority();                                          // set current sequence priority to normal. this will allow any other sequences to take over
  void loop(uint8_t);                                            // loop a sequence back to a different index/use inside sequence funct. input(indexToJumpTo)
  void start(uint8_t, bool);                                     // same as "setSequence()" but runs current funct instead of setting new. input(indexToStart, forceStart(default false))
  void stop();                                                   // stop current sequence(priority also)/put on standby. use "start()" to resume again

  uint8_t index(); // returns current running index

private:
  uint8_t _index;
  uint8_t (*_funct)(uint8_t);
  bool _run = false;
  bool _forceFirstStep = false;

  bool _currentPriority = false; // false = normal, true = priority
};

//.cpp

void SequenceBuild::run()
{
  if ((_run && _funct(255) == 1) || _forceFirstStep)
  {
    _forceFirstStep = false;
    _index++;
    if (_funct(_index) == 2)
      stop();
  }
}

void SequenceBuild::setSequence(uint8_t (*funct)(uint8_t), uint8_t startIndex, bool forceFirstStep = false)
{
  if (funct != _funct && !_currentPriority)
  {
    _funct = funct;
    start(startIndex, forceFirstStep);
  }
}
void SequenceBuild::setSequence(uint8_t (*funct)(uint8_t))
{
  if (!_currentPriority)
  {
    _funct = funct;
    stop();
  }
}
void SequenceBuild::setPrioritySequence(uint8_t (*funct)(uint8_t), uint8_t startIndex, bool forceFirstStep = false)
{
  if (funct != _funct)
  {
    _currentPriority = true;
    _funct = funct;
    start(startIndex, forceFirstStep);
  }
}
void SequenceBuild::setPrioritySequence(uint8_t (*funct)(uint8_t))
{
  _funct = funct;
  stop();
  _currentPriority = true;
}
void SequenceBuild::resetPriority()
{
  _currentPriority = false;
}
void SequenceBuild::loop(uint8_t index)
{
  _index = index - 1;
}
void SequenceBuild::start(uint8_t startIndex, bool forceFirstStep = false)
{
  _index = startIndex - 1;
  _forceFirstStep = forceFirstStep;
  _run = true;
}
void SequenceBuild::stop()
{
  _run = false;
  _currentPriority = false;
}

uint8_t SequenceBuild::index()
{
  return _index + 1;
}

#define SB_FUNCT(funct, cond)    \
  uint8_t funct(uint8_t _Index_) \
  {                              \
    uint8_t _IndexTest_ = 0;     \
    if (_Index_ == 255)          \
      return (cond ? 1 : 0);

#define SB_STEP(step)           \
  if (_IndexTest_++ == _Index_) \
  {                             \
    step                        \
  }                             \
  else

#define SB_END \
  return 2;    \
  return 0;    \
  }

#endif

// pinDriver and pinMacro:
// PinDriver
//.h

/*
  changes:
    - static variables removed. caused unknown error when used with
      multiple instances. problem should be resolved
*/

/*
purpose of this lib is to use port registers to read from pinInputs
this lib also handles debounce and delays the start state of the button
to prevent false positives
*/

#ifndef PinDriver_h
#define PinDriver_h

#ifndef PinDriverBootDelay_ms
#define PinDriverBootDelay_ms 100 // define how long button should wait before reading inputs
#endif

#ifndef PinDriverDeBounceDelay_ms
#define PinDriverDeBounceDelay_ms 10 // define debounce delay. 10ms is a good middle ground(glitchy buttons will need longer delay)
#endif

#include "Arduino.h"

class PinDriver
{
public:
  PinDriver(uint8_t, bool); // set pin and pullupMode. input(pinNumber, pullupMode(default = true))

  operator bool(); // same as "run()"
  bool run();      // run button debounce and startup delay. returns button state
  bool state();    // return button state without running button

private:
  bool _pinRead(); // return button state at given point using port read

  bool _pinBoot = false; // used for button cooldown during bootUp
  bool _buttonState;     // keep track of buttonState

  volatile uint8_t *_pinPort_PIN;  // Pin read/state register
  volatile uint8_t *_pinPort_DDR;  // Pin set register (INPUT/OUTPUT)
  volatile uint8_t *_pinPort_PORT; // Pin write register (HIGH/LOW)

  uint8_t _pinMask;    // eg. B00100000
  uint8_t _pinMaskNot; // eg. B11011111

  bool _pinTest = false;         // keep track of when pin is testing
  uint32_t _pinTimer = micros(); // use time to wait for boot up and testing the pin
};

//.cpp

PinDriver::PinDriver(uint8_t pin, bool pullupMode = true)
{
  // set pin registers:
  pin %= 20; // make sure pin does not go out of bounds
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

  *_pinPort_DDR &= _pinMaskNot; // set pin to input
  if (pullupMode)
  {
    *_pinPort_PORT |= _pinMask; // set to pullup
    _buttonState = true;        // set start value
  }
  else
  {
    *_pinPort_PORT = _pinMaskNot; // set to floating pin
    _buttonState = false;         // set start value
  }
}

PinDriver::operator bool()
{
  return run();
}

bool PinDriver::run()
{
  if (_pinBoot)
  {
    if (_pinTest)
    {
      if (micros() - _pinTimer >= ((uint32_t)PinDriverDeBounceDelay_ms * (uint32_t)1000))
      {
        if (_pinRead() != _buttonState)
          _buttonState = !_buttonState;
        _pinTest = false;
      }
    }
    else if (_pinRead() != _buttonState)
    {
      _pinTest = true;
      _pinTimer = micros();
    }
  }
  else
  {
    if (micros() - _pinTimer >= ((uint32_t)PinDriverBootDelay_ms * (uint32_t)1000))
      _pinBoot = true;
  }
  return _buttonState;
}
bool PinDriver::state()
{
  return _buttonState;
}

bool PinDriver::_pinRead()
{
  return (*_pinPort_PIN & _pinMask);
}

#endif

// InputMacro
//.h

/*
  changes
    - include a way to keep track of timed interrupts(for actions)
      during steady states. only single interrupt allowed for now
*/

#ifndef InputMacro_h
#define InputMacro_h

class InputMacro
{
public:
  InputMacro(bool); // declare starting state of Macro

  bool run(bool);          // run input macro. inputs(input(bool)). returns true is state/input changes
  bool operator()(bool);   // same as "run()"
  operator bool();         // return state of macro, does not update state, only returns last recorded state
  bool state();            // same as "operator bool". returns last state of macro
  bool stateChange();      // returns true if StateChange from "last" run call
  bool triggered();        // returns true if "trigger" var was triggered last run
  bool prevTriggered();    // returns last triggered state
  void trigger();          // use to trigger flag
  uint32_t interval();     // returns current interval(ms) of input from last stateChange
  uint32_t prevInterval(); // returns previous recorded interval(ms)
  void reset();            // reset current interval to 0ms

private:
  bool _state;               // global state of macro
  bool _stateChange = false; // stateChange status from last "run" call

  bool _triggered = false;
  bool _prevTriggered = false;

  uint32_t _timerInterval;    // this hold the millis() value at the time of last stateChange
  uint32_t _prevInterval = 0; // this hold actual value(in ms) of last interval
};

//.cpp

InputMacro::InputMacro(bool startState)
{
  _state = startState;
  reset();
}

bool InputMacro::run(bool stateInput)
{
  if (stateInput != _state)
  {
    if (_triggered)
      _prevTriggered = true;
    else
      _prevTriggered = false;
    _triggered = false;
    _state = stateInput;
    _stateChange = true;

    _prevInterval = (millis() - _timerInterval);
    reset();
  }
  else
    _stateChange = false;

  return _stateChange;
}
bool InputMacro::operator()(bool stateInput)
{
  return run(stateInput);
}
InputMacro::operator bool()
{
  return _state;
}
bool InputMacro::state()
{
  return _state;
}
bool InputMacro::stateChange()
{
  return _stateChange;
}
bool InputMacro::triggered()
{
  return _triggered;
}
bool InputMacro::prevTriggered()
{
  return _prevTriggered;
}
void InputMacro::trigger()
{
  _triggered = true;
}
uint32_t InputMacro::interval()
{
  return (millis() - _timerInterval);
}
uint32_t InputMacro::prevInterval()
{
  return _prevInterval;
}
void InputMacro::reset()
{
  _timerInterval = millis();
}

#endif

// set to pin 8 and ip 3. Etherent ip "should" be on ip 1:
OneWire_Slave rtx(8, 3);
const uint8_t ethernet_ip = 1;

// set up NeoPixels:
uint8_t pixelBuffer[12];                        // 12 bytes for 6 pixelChannels(r,g,b - (onPedal) and r,g,b - (Tally))
NeoPixelDriver pixelDriver(12, pixelBuffer, 6); // set neoPixels on pin 12. set buffer and declare 6 pixelChannels

// make sure pixels are listed in correct order "pre pixel"! (green(first), red(second), blue(last)):
// pedal:
NeoPixel pedal_G(pixelDriver); // green pixel on pedal led
NeoPixel pedal_R(pixelDriver); // Red pixel on pedal led
NeoPixel pedal_B(pixelDriver); // Blue pixel on pedal led
// tally:
NeoPixel tally_G(pixelDriver); // green pixel on pedal led
NeoPixel tally_R(pixelDriver); // Red pixel on pedal led
NeoPixel tally_B(pixelDriver); // Blue pixel on pedal led

uint8_t otherMacro; // use as a placeHolder macro

// set led macros:
LedMacro _macros[6];                                                // create 6 macros(for 6 pixel channels)
LedMacroManager macro(_macros, sizeof(_macros) / sizeof(LedMacro)); // set macro and number of available macros.

SequenceBuild ledBuild; // create sequence builder, only one is needed unless multiple sequences are need to be run

// set pin and inputMacros:
PinDriver LeftPedal(2);
PinDriver MiddlePedal(3);
PinDriver RightPedal(4);

InputMacro pinMacro[4] = {InputMacro(true), InputMacro(true), InputMacro(true), InputMacro(true)};

// connection states:
bool atem_connected = false;

// connection timer:
uint32_t connectionTimer = millis();

// tally states:
const uint8_t pedalTally = 5; // this is the tally that will show in the led's. 1 - 8 tally input
uint8_t tally_program = 0;
uint8_t tally_preview = 0;

void setup()
{
}

void loop()
{
  rtxHandle();
  buttonHandle();
  ledHandle();
}

void rtxHandle()
{
  uint8_t sender_ip;
  uint8_t dataArray[10]; // prepare for max possible bytes
  uint8_t dataBytes;
  if (rtx.read(sender_ip, dataArray, dataBytes))
  {
    if (sender_ip == ethernet_ip && dataBytes == 3)
    {
      connectionTimer = millis();

      atem_connected = dataArray[0] & (1);

      tally_program = dataArray[1];
      tally_preview = dataArray[2];
    }
  }
  if (millis() - connectionTimer >= 1000)
  {
    atem_connected = false;
  }
}

/*
buttonCommand():
  inputs: (pinMask: to specify which buttons on the pedal to check(eg. B110: left and middle button))
          (pinStates: run pinDrivers ahead of time and input the results, to keep all macros consistant, order them according to pinMask. Right most in "pinMask" will be first in pinState Array(right pedal should come first in array))
          (commandType: to specify if the button commands are "or - ||" or "and - &&"(false = or, true = and))
  returns: true if all button state match commands, false otherwise
*/
bool buttonCommand(uint8_t pinMask, bool *pinStates, bool commandType) // (pinMasks to specify which of the three buttons will be used)
{
  bool commandMatch = false;

  if (commandType) // and command
  {
    if (pinMask & B111) // only mark commandMatch true if any button mask is set
      commandMatch = true;
    for (uint8_t x = 0; x < 3 && commandMatch; x++)
    {
      if (pinMask & (1 << x))
        if (pinStates[x]) // << buttons are pressed in the low state! high means invalid "and"
          commandMatch = false;
    }
  }
  else // or command
  {
    for (uint8_t x = 0; x < 3 && !commandMatch; x++)
    {
      if (pinMask & (1 << x))
        if (!pinStates[x]) // << buttons are pressed in the low state! any low means valid "or"
          commandMatch = true;
    }
  }
  return commandMatch;
}

void buttonHandle()
{
  const uint8_t defaultInput = 5;
  const uint8_t secondaryInput = 6;

  bool pinStates[] =
      {RightPedal, MiddlePedal, LeftPedal};

  if (pinMacro[0](buttonCommand(B111, pinStates, false))) // state change
  {
    if (pinMacro[0] && pinMacro[0].prevTriggered())
    {
      pinMacro[0].trigger();

      if (!programTally(defaultInput) || !programTally(secondaryInput))
      {
        if (programTally(defaultInput))
        {
          if (!previewTally(secondaryInput))
          {
            rtx_preview(secondaryInput);
          }
        }
        if (programTally(secondaryInput))
        {
          if (!previewTally(defaultInput))
          {
            rtx_preview(defaultInput);
          }
        }
      }
    }
    else if (!pinMacro[0] && pinMacro[0].prevTriggered())
    {
      if (pinMacro[0].prevInterval() >= 300)
      {
        if (programTally(defaultInput) || programTally(secondaryInput))
          rtx_doAuto();
        else
        {
          rtx_preview(secondaryInput);
          rtx_doAuto();
        }
      }
      else
      {
        if (programTally(defaultInput) || programTally(secondaryInput))
          rtx_doCut();
        else
        {
          rtx_preview(defaultInput);
          rtx_doAuto();
        }
      }
    }
  }
  if (!pinMacro[0] && !pinMacro[0].triggered() && pinMacro[0].interval() >= 300)
  {
    pinMacro[0].trigger();
  }

  if (pinMacro[1](buttonCommand(B000, pinStates, false))) // state change
  {
    if (pinMacro[1])
    {
    }
    else
    {
    }
  }
  if (pinMacro[1] && !pinMacro[1].triggered() && pinMacro[1].interval() >= 0)
  {
    pinMacro[1].trigger();
  }

  if (pinMacro[2](buttonCommand(B000, pinStates, false))) // state change
  {
    if (pinMacro[2])
    {
    }
    else
    {
    }
  }
  if (pinMacro[2] && !pinMacro[2].triggered() && pinMacro[2].interval() >= 0)
  {
    pinMacro[2].trigger();
  }

  if (pinMacro[3](buttonCommand(B000, pinStates, false))) // state change
  {
    if (pinMacro[3])
    {
    }
    else
    {
    }
  }
  if (pinMacro[3] && !pinMacro[3].triggered() && pinMacro[3].interval() >= 0)
  {
    pinMacro[3].trigger();
  }
}

void ledHandle()
{
  macro.run();
  pixelDriver.run();
  ledBuild.run();

  if (atem_connected)
  {
    if (programTally(pedalTally))
    {
      // program tally:
      ledBuild.setSequence(setRed, 0, true);
    }
    else if (previewTally(pedalTally))
    {
      // preview tally:
      ledBuild.setSequence(setGreen, 0, true);
    }
    else
    {
      // default:
      ledBuild.setSequence(setDefault, 0, true);
    }
  }
  else
  {
    ledBuild.setSequence(connectionError, 0, true);
  }
}

inline bool allLedReady()
{
  return (macro.ready(pedal_R) && macro.ready(pedal_G) && macro.ready(pedal_B) &&
          macro.ready(tally_R) && macro.ready(tally_G) && macro.ready(tally_B) &&
          macro.ready(otherMacro));
}

/*
  Next few functions are just to simplify sending atem command
  reliably.
    using this chart to send commands(for first byte of packet):
      command 0: null
      command 1: doCut. expects one byte
      command 2: doAuto. expects one byte
      command 3: previewInput. expects two bytes, byte[1] should include preview input
      command 4: programInput. expects two bytes, byte[1] should include program input
      command 5: toggle_ftb. expects one byte
*/

void rtx_doCut()
{
  uint8_t dataArray[1];
  dataArray[0] = 1;
  rtx_sendPacket(ethernet_ip, dataArray, 1);
}
void rtx_doAuto()
{
  uint8_t dataArray[1];
  dataArray[0] = 2;
  rtx_sendPacket(ethernet_ip, dataArray, 1);
}
void rtx_ftb()
{
  uint8_t dataArray[1];
  dataArray[0] = 5;
  rtx_sendPacket(ethernet_ip, dataArray, 1);
}
void rtx_preview(uint8_t input)
{
  uint8_t dataArray[2];
  dataArray[0] = 3;
  dataArray[1] = input;
  rtx_sendPacket(ethernet_ip, dataArray, 2);
}
void rtx_program(uint8_t input)
{
  uint8_t dataArray[2];
  dataArray[0] = 4;
  dataArray[1] = input;
  rtx_sendPacket(ethernet_ip, dataArray, 2);
}

bool rtx_sendPacket(uint8_t receiver_ip, uint8_t *dataArray, uint8_t bytes)
{
  if (!rtx.write(receiver_ip, dataArray, bytes))
  {
    if (rtx.writeError() == 2) // buffer full, try to send a second time
    {
      return rtx.write(receiver_ip, dataArray, bytes);
    }
    else if (rtx.writeError() == 0) // << MisCommunication? - this error should NEVER happen
    {
      return true;
    }
    else
      return false;
  }
  else
    return true;
}

bool programTally(uint8_t input)
{
  if (input >= 1 && input <= 8)
    return tally_program & (1 << (input - 1));
  return false;
}
bool previewTally(uint8_t input)
{
  if (input >= 1 && input <= 8)
    return tally_preview & (1 << (input - 1));
  return false;
}

SB_FUNCT(connectionError, allLedReady())
SB_STEP(macro.quadEase(pedal_R, 0, 15);
        macro.quadEase(pedal_G, 0, 15);
        macro.quadEase(pedal_B, 0, 15);
        macro.quadEase(tally_R, 0, 15);
        macro.quadEase(tally_G, 0, 15);
        macro.quadEase(tally_B, 0, 15);)
SB_STEP(macro.quadEase(pedal_R, 255, 15);
        macro.quadEase(tally_R, 255, 15);)
SB_STEP(macro.quadEase(pedal_R, 0, 15);
        macro.quadEase(tally_R, 0, 15);)
SB_STEP(ledBuild.loop(1);)
SB_END

SB_FUNCT(setGreen, false)
SB_STEP(macro.quadEase(pedal_R, 0, 15);
        macro.quadEase(pedal_G, 255, 15);
        macro.quadEase(pedal_B, 0, 15);
        macro.quadEase(tally_R, 0, 15);
        macro.quadEase(tally_G, 255, 15);
        macro.quadEase(tally_B, 0, 15);)
SB_END

SB_FUNCT(setRed, false)
SB_STEP(macro.quadEase(pedal_R, 255, 15);
        macro.quadEase(pedal_G, 0, 15);
        macro.quadEase(pedal_B, 0, 15);
        macro.quadEase(tally_R, 255, 15);
        macro.quadEase(tally_G, 0, 15);
        macro.quadEase(tally_B, 0, 15);)
SB_END

SB_FUNCT(setDefault, false)
SB_STEP(macro.quadEase(pedal_R, 50, 15);
        macro.quadEase(pedal_G, 50, 15);
        macro.quadEase(pedal_B, 50, 15);
        macro.quadEase(tally_R, 50, 15);
        macro.quadEase(tally_G, 50, 15);
        macro.quadEase(tally_B, 50, 15);)
SB_END