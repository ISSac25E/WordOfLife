// OneWire_Slave
//.h
#ifndef OneWire_Slave_h
#define OneWire_Slave_h

#include "Arduino.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\core\OneWire_Driver\OneWire_Driver_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\core\BoolConverter\BoolConverter_1.0.0.h"

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
