// RTX_Protocol
//.h
#ifndef RTX_Protocal_h
#define RTX_Protocal_h

#ifndef WriteEntireEE
#define WriteEntireEE true
#endif

#ifndef RTX_MaxWriteTimes
#define RTX_MaxWriteTimes 2 // if receiver's ip's buffer is full, this is the max number of write tries
#endif

#ifndef RTX_RequestProtocol_MessageTimeOut_ms
// message timeout begins immediately AFTER request is sent:
#define RTX_RequestProtocol_MessageTimeOut_ms 10
#endif

#ifndef CONFIG_Sectors
#define CONFIG_Sectors 0
#warning "CONFIG_Sectors not declared. format to declare: #define CONFIG_Sectors 32"
#endif

#ifndef DEBUG_Sectors
#define DEBUG_Sectors 0
#warning "DEBUG_Sectors not declared. format to declare: #define DEBUG_Sectors 32"
#endif

#ifndef RTX_Name
#define RTX_Name ""
#warning "RTX_Name not declared. format to declare: #define RTX_Name "rtxDev""
#endif

#ifndef RTX_Version
#define RTX_Version 1, 0, 0
#warning "RTX_Version not declared. format to declare: #define RTX_Version 1,0,0"
#endif

#include "Arduino.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Slave\OneWire_Slave_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\core\ConfigEEPROM\ConfigEEPROM_1.0.1.h"

class RTX_Protocol
{
public:
  RTX_Protocol(uint8_t, uint8_t); // initialize the methods inside RTX_Protocol. Input(rtx_pin, rtx_ip)

  bool run(); // this does not serve much more purpose than to keep the device connected to the network and respond to incoming requests. returns true = connected to rtx, false = not connected

  bool read(uint8_t &, uint8_t *, uint8_t &, bool = false); // runs the connection in the background and return true if message received, false otherwise. inputs(tx_ip(return), msg(return)(min 10-bytes), msg_bytes(return), returnResponse_messages_too(default false))
  bool write(uint8_t, uint8_t *, uint8_t);                  // send a message. inputs(rx_ip, msg(up to 10 bytes), bytesInMsg). returns true if message sent, false otherwise

  uint8_t writeError(); // returns write status of last write attempt. 0 = no error(message sent successfully), 1 = rx_ip not connected, 2 = rx_ip buffer full, 3 = other error(rtx disconnected etc.)

  bool EEPROM_writing(); // returns if EEPROM is currently writing

  // use this variable to verify EEPROM status and to set also:
  bool EEPROM_error = false; // if true, last EEPROM read contained error. to ignore error, set false

  uint8_t configBuffer[CONFIG_Sectors * 8]; // this is the public config buffer where EE bytes will be stored each read
  uint8_t debugBuffer[DEBUG_Sectors * 8];   // this is the public debug buffer. anything written to it can be viewed over the rtx protocol by any other device

private:
  bool _EEPROM_writing = false; // if true, EEPROM is currently writing data. setting to false will have no effect as will setting true
  uint8_t _writeError = 0;

  OneWire_Slave _OneWire_Slave;
  ConfigEEPROM _ConfigEEPROM{CONFIG_Sectors, WriteEntireEE};
};

class RTX_RequestProtocol
{
public:
  /*
    this class is only used on devices that intend to config, debug, or request
    from other devices on the network. Each request can take up to and even more
    than the declared "RTX_RequestProtocol_MessageTimeOut_ms"

    each request will attempt up to "RTX_MaxWriteTimes" times to send request
   */
  RTX_RequestProtocol(RTX_Protocol &); // give RTX_Protocol instance to run off of'

  /*
  "request_Error()"
   request_Error refreshes its status after each request
    returns error codes
     - 0: no Error/last request successful(also boot state)
     - 1: rtx bus not connected/rtx bus error
     - 2: receiver_ip not connected/can't send
     - 3: receiver_ip buffer full
     - 4: timeout/no response
     - 5: other error(check respective error code for more info if available)(more than likely a response was received, but was not consistant)
  */
  uint8_t requestError();

  /*
    "*" = request sent with function(also updates "requestError")
  */

  // system commands:
  bool ping(uint8_t);                           // * send ping to a device. input(target_ip(0 - 7)). returns true if ping successful, false otherwise(use "request_Error" to get reason)
  bool ping(uint8_t, uint32_t &);               // * send ping to a device and get ping result time(us). input(target_ip, ping_result(return-val)). return true if ping successful, false otherwise
  bool deviceName(uint8_t, char *);             // * return device name. all names are 6 characters or less. input(target_ip, deviceName(return-buffer)(min 7-bytes(final char is always '\0')). returns true if name retrieved, false otherwise
  bool version(uint8_t, uint8_t *, uint8_t &);  // * return device software-version. input(target_ip, softwareVer(return-buffer)(min 6-bytes), versionLength(return)(number of values in buffer)(0 - 6)). returns true if software-version retrieved. false otherwise
  bool date(uint8_t, uint8_t *);                // * return date of software upload. input(target_ip, uploadDate(return-buffer)(always 3-Bytes(D[0](1-31),M[1](1-12),Y[2](2000+)))). returns true if upload date retrieved, false otherwise
  bool time(uint8_t, uint8_t *);                // * return time of software upload. input(target_ip, uploadTime(return-buffer)(always 3-Bytes(H[0],M[1],S[2]))). returns true if upload time retrieved, false otherwise(will return false if data is retrieved but not consistant (request_Error = 5))
  bool dateTime(uint8_t, uint8_t *, uint8_t *); // ** returns both time and date of upload. warning - it is not faster than "time" and "date", it is only easier to implement. input(target_ip, date(return-buffer), time(return-buffer)). returns true if time and date found, false otherwise(will return false if data is retrieved but not consistant (request_Error = 5))

  // buffer and EEPROM commands:
  bool configRead(uint8_t, uint8_t, uint8_t &); // * read a byte from the config buffer of another device. input(target_ip, byteAddress(0 - addressRange), returnByte(return val)). returns true if byte retrieved, false otherwise or if address out of range(check "bufferError")
  bool configWrite(uint8_t, uint8_t, uint8_t);  // * write to an address on the config buffer. input(target_ip, byteAddress(0 - addressRange), valueToWrite(0 - 255)). returns true if byte written, false otherwise or if address out of range(check "bufferError")

  bool debugRead(uint8_t, uint8_t, uint8_t &); // * read a byte from the debug buffer of another device. input(target_ip, byteAddress(0 - addressRange), returnByte(return val)). returns true if byte retrieved, false otherwise or if address out of range(check "bufferError")
  bool debugWrite(uint8_t, uint8_t, uint8_t);  // * write to an address on the debug buffer. input(target_ip, byteAddress(0 - addressRange), valueToWrite(0 - 255)). returns true if byte written, false otherwise or if address out of range(check "bufferError")

  bool bufferAddressError(); // returns wether last read or write to config or debug buffer was out of its address range. returns true of previous address was out of range, false if other or no error

  bool EEPROM_load(uint8_t); // * loads values from EEPROM to config buffer and updates "EEPROM_error". input(target_ip). returns true if EEPROM loaded, false otherwise
  bool EEPROM_burn(uint8_t); // * loads all values from configBuffer to EEPROM and sets error detection. progress can be tracked through EEPROM_burnProgress and updated through "EEPROM_updateStatus". input(target_ip). returns true if request has been sent, false otherwise

  bool EEPROM_updateStatus(uint8_t); // * update EEPROM error-status, writing-status, writeProgress-status. returns true if EEPROM status has been updated, false otherwise
  bool EEPROM_error();               // returns true if EEPROM ed(error detection) found error. updates with "EEPROM_load" or "EEPROM_status". returns true of EEPROM has error. false otherwise
  bool EEPROM_writing();             // returns true if EEPROM is currently writing/burning. status updates with "EEPROM_updateStatus"
  uint16_t EEPROM_burnProgress();    // returns how many bytes have been burned. Total bytes to burn can be found in "EEPROM_totalBytes". "EEPROM_burnProgress" is updated through "EEPROM_updateStatus"

  bool EEPROM_setError(uint8_t, bool); // * set EEPROM ed error to true (error) or false(no error). input(target_ip, errorSet(true - error)(false - no error)). returns true if request set, false otherwise

  bool configBytes(uint8_t, uint16_t &);  // * requests total bytes in config buffer. input(configBufferBytes(return val)). returns true if "configBufferBytes" returned, false otherwise
  bool debugBytes(uint8_t, uint16_t &);   // * requests total bytes in debug buffer. input(debugBufferBytes(return val)). returns true if "debugBufferBytes" returned, false otherwise
  bool EEPROM_bytes(uint8_t, uint16_t &); // * requests total bytes in EEPROM buffer. input(EEPROM_BufferBytes(return val)). returns true if "EEPROM_BufferBytes" returned, false otherwise

  bool EEPROM_writeCount(uint8_t, uint32_t &); // * request EEPROM write cycles. input(target_ip, EEPROM_writeCycles(return)). returns true is received response, false otherwise

private:
  RTX_Protocol *_RTX_Protocol_pointer;

  void _rtx_errorCodeSet(); // to simplify converting error code with rtx error. will set "_requestError" to either 1, 2, or 3

  uint8_t _requestError = 0;

  bool _bufferAddressError = false;
  bool _EEPROM_error = false;
  bool _EEPROM_writing = false;
  uint16_t _EEPROM_burnProgress = 0;
};

//.cpp

RTX_Protocol::RTX_Protocol(uint8_t pin, uint8_t ip) : _OneWire_Slave(pin, ip)
{
  // immediately fill the config buffer with eeprom data on boot:
  EEPROM_error = !_ConfigEEPROM.read(configBuffer);
}

bool RTX_Protocol::run()
{
  uint8_t sender_ip;
  uint8_t dataArray[10];
  uint8_t bytes;

  read(sender_ip, dataArray, bytes);

  return (_OneWire_Slave.rtxError() ? false : true); // if "rtxError" returns anything other than '0', then rtx is not connected
}

bool RTX_Protocol::read(uint8_t &sender_ip, uint8_t *dataArray, uint8_t &bytes, bool returnResponse = false)
{
  _EEPROM_writing = _ConfigEEPROM.run();
  if (_OneWire_Slave.read(sender_ip, dataArray, bytes))
  {
    if (bytes)
    {
      if (dataArray[0] & B00000001) // first bit declares packet as general purpose(0) or command packet(1)
      {
        if (!(dataArray[0] & B00000010))   // second bit declares systemCommand(0) or bufferCommand(1)
        {                                  // system command:
          if (!(dataArray[0] & B00100000)) // check if packet is requesting
          {
            switch ((dataArray[0] & B00011100) >> 2)
            {
            case 0:
              // ping request:
              if (bytes == 1)
              {
                uint8_t responsePacket[1];
                responsePacket[0] = B00100001;
                _OneWire_Slave.write(sender_ip, responsePacket, 1);
              }
              break;
            case 1:
              // device name request:
              if (bytes == 1)
              {
                uint8_t nameLength = 6; // max length of device name
                if (sizeof(RTX_Name) - 1 < nameLength)
                  nameLength = sizeof(RTX_Name) - 1;

                uint8_t responsePacket[nameLength + 1];
                responsePacket[0] = B00100101;

                for (uint8_t x = 0; x < nameLength; x++)
                  responsePacket[x + 1] = RTX_Name[x];
                _OneWire_Slave.write(sender_ip, responsePacket, nameLength + 1);
              }
              break;
            case 2:
              // software version request:
              if (bytes == 1)
              {
                uint8_t version[] = {RTX_Version};
                uint8_t versionLength = 6; // max version digits
                if (sizeof(version) < versionLength)
                  versionLength = sizeof(version);
                uint8_t responsePacket[sizeof(version) + 1];
                responsePacket[0] = B00101001;

                for (uint8_t x = 0; x < versionLength; x++)
                  responsePacket[x + 1] = version[x];
                _OneWire_Slave.write(sender_ip, responsePacket, versionLength + 1);
              }
              break;
            case 3:
              // software-upload date request:
              if (bytes == 1)
              {
                uint8_t responsePacket[4]; //[1] = day, [2] = month, [3] = year
                responsePacket[0] = B00101101;
                char month[] = {__DATE__[0], __DATE__[1], __DATE__[2], '\0'};
                if (!strcmp(month, "Jan"))
                  responsePacket[2] = 1;
                else if (!strcmp(month, "Feb"))
                  responsePacket[2] = 2;
                else if (!strcmp(month, "Mar"))
                  responsePacket[2] = 3;
                else if (!strcmp(month, "Apr"))
                  responsePacket[2] = 4;
                else if (!strcmp(month, "May"))
                  responsePacket[2] = 5;
                else if (!strcmp(month, "Jun"))
                  responsePacket[2] = 6;
                else if (!strcmp(month, "Jul"))
                  responsePacket[2] = 7;
                else if (!strcmp(month, "Aug"))
                  responsePacket[2] = 8;
                else if (!strcmp(month, "Sep"))
                  responsePacket[2] = 9;
                else if (!strcmp(month, "Oct"))
                  responsePacket[2] = 10;
                else if (!strcmp(month, "Nov"))
                  responsePacket[2] = 11;
                else if (!strcmp(month, "Dec"))
                  responsePacket[2] = 12;
                else
                  responsePacket[2] = 0;

                if (__DATE__[5] >= '0' && __DATE__[5] <= '9')
                  responsePacket[1] = (__DATE__[5] - '0');
                if (__DATE__[4] >= '0' && __DATE__[4] <= '9')
                  responsePacket[1] += (__DATE__[4] - '0') * 10;
                if (__DATE__[10] >= '0' && __DATE__[10] <= '9')
                  responsePacket[3] = (__DATE__[10] - '0');
                if (__DATE__[9] >= '0' && __DATE__[9] <= '9')
                  responsePacket[3] += (__DATE__[9] - '0') * 10;

                _OneWire_Slave.write(sender_ip, responsePacket, 4);
              }
              break;
            case 4:
              // software-upload time request:
              if (bytes == 1)
              {
                uint8_t responsePacket[4]; //[1] = hours, [2] = minutes, [3] = seconds
                responsePacket[0] = B00110001;

                if (__TIME__[1] >= '0' && __TIME__[1] <= '9')
                  responsePacket[1] = (__TIME__[1] - '0');
                if (__TIME__[0] >= '0' && __TIME__[0] <= '9')
                  responsePacket[1] += (__TIME__[0] - '0') * 10;
                if (__TIME__[4] >= '0' && __TIME__[4] <= '9')
                  responsePacket[2] = (__TIME__[4] - '0');
                if (__TIME__[3] >= '0' && __TIME__[3] <= '9')
                  responsePacket[2] += (__TIME__[3] - '0') * 10;
                if (__TIME__[7] >= '0' && __TIME__[7] <= '9')
                  responsePacket[3] = (__TIME__[7] - '0');
                if (__TIME__[6] >= '0' && __TIME__[6] <= '9')
                  responsePacket[3] += (__TIME__[6] - '0') * 10;

                _OneWire_Slave.write(sender_ip, responsePacket, 4);
              }
              break;
            }
          }
          else
          {
            // packet is responding to previous request
            // return true only if "returnResponse" set
            if (returnResponse)
              return true; // response found
          }
        }
        else // buffer config command:
        {
          if (!(dataArray[0] & B00100000)) // check if packet is requesting
          {
            switch ((dataArray[0] & B00011100) >> 2)
            {
            case 0:
              // buffer read(config or debug):
              if (bytes == 2)
              {
                if (!(dataArray[0] & B01000000)) // check if config-buffer or debug-buffer is needed
                {
                  // config-buffer:
                  if (dataArray[1] < CONFIG_Sectors * 8) // for now, only up to 256 bytes are allowed in rtx-protocol
                  {
                    uint8_t responsePacket[3];
                    responsePacket[0] = B00100011;
                    responsePacket[1] = dataArray[1];
                    responsePacket[2] = configBuffer[responsePacket[1]];
                    _OneWire_Slave.write(sender_ip, responsePacket, 3);
                  }
                  else // out of range, send back error
                  {
                    uint8_t responsePacket[2];
                    responsePacket[0] = B10100011;
                    responsePacket[1] = dataArray[1];
                    _OneWire_Slave.write(sender_ip, responsePacket, 2);
                  }
                }
                else
                {
                  // debug-buffer:
                  if (dataArray[1] < DEBUG_Sectors * 8) // for now, only up to 256 bytes are allowed in rtx-protocol
                  {
                    uint8_t responsePacket[3];
                    responsePacket[0] = B01100011;
                    responsePacket[1] = dataArray[1];
                    responsePacket[2] = debugBuffer[responsePacket[1]];
                    _OneWire_Slave.write(sender_ip, responsePacket, 3);
                  }
                  else // out of range, send back error
                  {
                    uint8_t responsePacket[2];
                    responsePacket[0] = B11100011;
                    responsePacket[1] = dataArray[1];
                    _OneWire_Slave.write(sender_ip, responsePacket, 2);
                  }
                }
              }
              break;
            case 1:
              // buffer write(config or debug):
              if (bytes == 3)
              {
                if (!(dataArray[0] & B01000000)) // check if config-buffer or debug-buffer is needed
                {
                  // config buffer:
                  if (dataArray[1] < CONFIG_Sectors * 8) // for now, only up to 256 bytes are allowed in rtx-protocol
                  {
                    configBuffer[dataArray[1]] = dataArray[2];
                    uint8_t responsePacket[3];
                    responsePacket[0] = B00100111;
                    responsePacket[1] = dataArray[1];
                    responsePacket[2] = configBuffer[responsePacket[1]];
                    _OneWire_Slave.write(sender_ip, responsePacket, 3);
                  }
                  else // out of range, send back error
                  {
                    uint8_t responsePacket[2];
                    responsePacket[0] = B10100111;
                    responsePacket[1] = dataArray[1];
                    _OneWire_Slave.write(sender_ip, responsePacket, 2);
                  }
                }
                else
                {
                  // debug buffer:
                  if (dataArray[1] < DEBUG_Sectors * 8) // for now, only up to 256 bytes are allowed in rtx-protocol
                  {
                    debugBuffer[dataArray[1]] = dataArray[2];
                    uint8_t responsePacket[3];
                    responsePacket[0] = B01100111;
                    responsePacket[1] = dataArray[1];
                    responsePacket[2] = debugBuffer[responsePacket[1]];
                    _OneWire_Slave.write(sender_ip, responsePacket, 3);
                  }
                  else // out of range, send back error
                  {
                    uint8_t responsePacket[2];
                    responsePacket[0] = B11100111;
                    responsePacket[1] = dataArray[1];
                    _OneWire_Slave.write(sender_ip, responsePacket, 2);
                  }
                }
              }
              break;
            case 2:
              // EEPROM load(load EEPROM to config buffer and update ED):
              if (bytes == 1)
              {
                EEPROM_error = !_ConfigEEPROM.read(configBuffer);
                uint8_t responsePacket[1];
                responsePacket[0] = B00101011;
                if (EEPROM_error)
                  responsePacket[0] |= B10000000;
                _OneWire_Slave.write(sender_ip, responsePacket, 1);
              }
              break;
            case 3:
              // EEPROM Burn:
              if (bytes == 1)
              {
                // start EEPROM burn:
                _ConfigEEPROM.write(configBuffer);
                uint8_t responsePacket[3];
                responsePacket[0] = B10101111;
                responsePacket[1] = _ConfigEEPROM.totalBytes();
                responsePacket[2] = _ConfigEEPROM.totalBytes() >> 8;
                _OneWire_Slave.write(sender_ip, responsePacket, 3);
              }
              break;
            case 4:
              // EEPROM status(error, writing, burn status):
              if (bytes == 1)
              {
                if (_EEPROM_writing)
                {
                  uint8_t responsePacket[3]; // send write progress also
                  responsePacket[0] = B01110011;
                  if (EEPROM_error)
                    responsePacket[0] |= B10000000;
                  responsePacket[1] = _ConfigEEPROM.progressBytes();
                  responsePacket[2] = _ConfigEEPROM.progressBytes() >> 8;
                  _OneWire_Slave.write(sender_ip, responsePacket, 3);
                }
                else
                {
                  uint8_t responsePacket[1];
                  responsePacket[0] = B00110011;
                  if (EEPROM_error)
                    responsePacket[0] |= B10000000;
                  _OneWire_Slave.write(sender_ip, responsePacket, 1);
                }
              }
              break;
            case 5:
              // EEPROM error reset:
              if (bytes == 1)
              {
                uint8_t responsePacket[1];
                responsePacket[0] = B00110111;
                if (dataArray[0] & B10000000)
                {
                  EEPROM_error = true;
                  responsePacket[0] |= B10000000;
                }
                else
                  EEPROM_error = false;
                _OneWire_Slave.write(sender_ip, responsePacket, 1);
              }
              break;
            case 6:
              // buffer total bytes(config, debug or EEPROM):
              if (bytes == 1)
              {
                switch ((dataArray[0] & B11000000) >> 6)
                {
                case 0:
                  // config buffer bytes
                  {
                    uint8_t responsePacket[3];
                    responsePacket[0] = B00111011;
                    responsePacket[1] = (CONFIG_Sectors * 8);
                    responsePacket[2] = (CONFIG_Sectors * 8) >> 8;
                    _OneWire_Slave.write(sender_ip, responsePacket, 3);
                  }
                  break;
                case 1:
                  // debug buffer bytes
                  {
                    uint8_t responsePacket[3];
                    responsePacket[0] = B01111011;
                    responsePacket[1] = (DEBUG_Sectors * 8);
                    responsePacket[2] = (DEBUG_Sectors * 8) >> 8;
                    _OneWire_Slave.write(sender_ip, responsePacket, 3);
                  }
                  break;
                case 2:
                  // EEPROM total write bytes
                  {
                    uint8_t responsePacket[3];
                    responsePacket[0] = B10111011;
                    responsePacket[1] = _ConfigEEPROM.totalBytes();
                    responsePacket[2] = _ConfigEEPROM.totalBytes() >> 8;
                    _OneWire_Slave.write(sender_ip, responsePacket, 3);
                  }
                  break;
                }
              }
              break;
            case 7:
              // EEPROM write count:
              if (bytes == 1)
              {
                uint8_t responsePacket[5];
                responsePacket[0] = B00111111;
                for (uint8_t x = 0; x < 4; x++)
                  responsePacket[x + 1] = _ConfigEEPROM.writeCount() >> (x * 8);
                _OneWire_Slave.write(sender_ip, responsePacket, 5);
              }
              break;
            }
          }
          else
          {
            // packet is responding to previous request
            // return true only if "returnResponse" set
            if (returnResponse)
              return true; // response found
          }
        }
      }
    }
    else
    {
      // no bytes, but still considered general purpose:
      return true;
    }
  }
  return false;
}
bool RTX_Protocol::write(uint8_t receiver_ip, uint8_t *dataArray, uint8_t bytes)
{

  for (uint8_t x = 0; x < RTX_MaxWriteTimes; x++)
  {
    if (_OneWire_Slave.write(receiver_ip, dataArray, bytes))
    {
      _writeError = 0;
      return true;
    }
    _writeError = _OneWire_Slave.writeError();
    if (_writeError == 0) // miscommunication? should never happen
      return true;
    else if (_writeError != 2) // if message is not sent, it can only be resent if error is a 2
      return false;
  }
  return false;
}

uint8_t RTX_Protocol::writeError()
{
  return _writeError;
}

bool RTX_Protocol::EEPROM_writing()
{
  return _EEPROM_writing;
}

RTX_RequestProtocol::RTX_RequestProtocol(RTX_Protocol &RTX_Protocol_pointer)
{
  _RTX_Protocol_pointer = &RTX_Protocol_pointer;
}

uint8_t RTX_RequestProtocol::requestError()
{
  return _requestError;
}

bool RTX_RequestProtocol::ping(uint8_t target_ip)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00000001;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00100001)
        {
          if (bytes == 1)
          {
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::ping(uint8_t target_ip, uint32_t &pingResult_us)
{
  uint32_t pingResultTimer = micros();
  {
    uint8_t dataArray[1];
    dataArray[0] = B00000001;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00100001)
        {
          if (bytes == 1)
          {
            _requestError = 0;
            pingResult_us = micros() - pingResultTimer;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    pingResult_us = 0;
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::deviceName(uint8_t target_ip, char *nameChar)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00000101;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00100101)
        {
          if (bytes <= 7) // max 6 char name
          {
            for (uint8_t x = 0; x < (bytes - 1); x++)
              nameChar[x] = dataArray[x + 1];
            nameChar[(bytes - 1)] = '\0';
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::version(uint8_t target_ip, uint8_t *versionArray, uint8_t &versionLength)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00001001;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00101001)
        {
          if (bytes <= 7) // max 6 version digits
          {
            for (uint8_t x = 0; x < (bytes - 1); x++)
              versionArray[x] = dataArray[x + 1];
            versionLength = (bytes - 1);
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::date(uint8_t target_ip, uint8_t *dateArray)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00001101;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00101101)
        {
          if (bytes == 4) // date should be 4 bytes(CMD, day, month, year)
          {
            if (dataArray[1] >= 1 && dataArray[1] <= 31 &&
                dataArray[2] >= 1 && dataArray[2] <= 12)
            {
              for (uint8_t x = 0; x < (bytes - 1); x++)
                dateArray[x] = dataArray[x + 1];

              _requestError = 0;
              return true;
            }
            else
            {
              _requestError = 5;
              return true;
            }
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::time(uint8_t target_ip, uint8_t *timeArray)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00010001;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00110001)
        {
          if (bytes == 4) // date should be 4 bytes(CMD, hours, minutes, seconds)
          {
            if (dataArray[1] <= 23 &&
                dataArray[2] >= 0 && dataArray[2] <= 59 &&
                dataArray[3] >= 0 && dataArray[3] <= 59)
            {
              for (uint8_t x = 0; x < (bytes - 1); x++)
                timeArray[x] = dataArray[x + 1];

              _requestError = 0;
              return true;
            }
            else
            {
              _requestError = 5;
              return true;
            }
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::dateTime(uint8_t target_ip, uint8_t *dateArray, uint8_t *timeArray)
{
  if (this->date(target_ip, dateArray) && this->time(target_ip, timeArray))
    return true;
  else
    return false;
}

bool RTX_RequestProtocol::configRead(uint8_t target_ip, uint8_t address, uint8_t &dataByte)
{
  _bufferAddressError = false;
  {
    uint8_t dataArray[2];
    dataArray[0] = B00000011; // request read from config buff
    dataArray[1] = address;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 2);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B01111111) == B00100011)
        {
          if (bytes >= 2)
          {
            if (dataArray[1] == address)
            {
              if (!(dataArray[0] & B10000000))
              {
                if (bytes == 3)
                {
                  dataByte = dataArray[2];
                  _requestError = 0;
                  return true;
                }
                else
                {
                  _requestError = 5;
                  return false;
                }
              }
              else
              {
                // address out of range
                _requestError = 5;
                _bufferAddressError = true;
                return false;
              }
            }
            else
            {
              // _requestError = 5; // << is it really an error at this point?
              // keep looking? maybe multiple response packets conflicting?
            }
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::configWrite(uint8_t target_ip, uint8_t address, uint8_t dataByte)
{
  _bufferAddressError = false;
  {
    uint8_t dataArray[3];
    dataArray[0] = B00000111; // request write from config buff
    dataArray[1] = address;
    dataArray[2] = dataByte;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 3);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B01111111) == B00100111)
        {
          if (bytes >= 2)
          {
            if (dataArray[1] == address)
            {
              if (!(dataArray[0] & B10000000))
              {
                if (bytes == 3)
                {
                  _requestError = 0;
                  return true;
                }
                else
                {
                  _requestError = 5;
                  return false;
                }
              }
              else
              {
                // address out of range
                _requestError = 5;
                _bufferAddressError = true;
                return false;
              }
            }
            else
            {
              // _requestError = 5; // << is it really an error at this point?
              // keep looking? maybe multiple response packets conflicting?
            }
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}

bool RTX_RequestProtocol::debugRead(uint8_t target_ip, uint8_t address, uint8_t &dataByte)
{
  _bufferAddressError = false;
  {
    uint8_t dataArray[2];
    dataArray[0] = B01000011; // request read from config buff
    dataArray[1] = address;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 2);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B01111111) == B01100011)
        {
          if (bytes >= 2)
          {
            if (dataArray[1] == address)
            {
              if (!(dataArray[0] & B10000000))
              {
                if (bytes == 3)
                {
                  dataByte = dataArray[2];
                  _requestError = 0;
                  return true;
                }
                else
                {
                  _requestError = 5;
                  return false;
                }
              }
              else
              {
                // address out of range
                _requestError = 5;
                _bufferAddressError = true;
                return false;
              }
            }
            else
            {
              // _requestError = 5; // << is it really an error at this point?
              // keep looking? maybe multiple response packets conflicting?
            }
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::debugWrite(uint8_t target_ip, uint8_t address, uint8_t dataByte)
{
  _bufferAddressError = false;
  {
    uint8_t dataArray[3];
    dataArray[0] = B01000111; // request write from config buff
    dataArray[1] = address;
    dataArray[2] = dataByte;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 3);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B01111111) == B01100111)
        {
          if (bytes >= 2)
          {
            if (dataArray[1] == address)
            {
              if (!(dataArray[0] & B10000000))
              {
                if (bytes == 3)
                {
                  _requestError = 0;
                  return true;
                }
                else
                {
                  _requestError = 5;
                  return false;
                }
              }
              else
              {
                // address out of range
                _requestError = 5;
                _bufferAddressError = true;
                return false;
              }
            }
            else
            {
              // _requestError = 5; // << is it really an error at this point?
              // keep looking? maybe multiple response packets conflicting?
            }
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}

bool RTX_RequestProtocol::bufferAddressError()
{
  return _bufferAddressError;
}

bool RTX_RequestProtocol::EEPROM_load(uint8_t target_ip)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00001011;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00101011)
        {
          if (bytes == 1)
          {
            _EEPROM_error = (dataArray[0] & B10000000);
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::EEPROM_burn(uint8_t target_ip)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00001111;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00101111)
        {
          if (bytes == 3)
          {
            _EEPROM_writing = (dataArray[0] & B10000000);
            _EEPROM_burnProgress = 0;
            _EEPROM_burnProgress = dataArray[1];
            _EEPROM_burnProgress |= (dataArray[2] << 8);
            _requestError = 0;
            return true;
          }
          else if (bytes == 1)
          {
            _EEPROM_writing = (dataArray[0] & B10000000);
            _EEPROM_burnProgress = 0;
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}

bool RTX_RequestProtocol::EEPROM_updateStatus(uint8_t target_ip)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00010011;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00110011)
        {
          if (bytes == 1)
          {
            _EEPROM_writing = (dataArray[0] & B01000000);
            _EEPROM_error = (dataArray[0] & B10000000);

            _EEPROM_burnProgress = 0;

            _requestError = 0;
            return true;
          }
          else if (bytes == 3)
          {
            _EEPROM_writing = (dataArray[0] & B01000000);
            _EEPROM_error = (dataArray[0] & B10000000);

            _EEPROM_burnProgress = 0;
            _EEPROM_burnProgress = dataArray[1];
            _EEPROM_burnProgress |= (dataArray[2] << 8);

            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::EEPROM_error()
{
  return _EEPROM_error;
}
bool RTX_RequestProtocol::EEPROM_writing()
{
  return _EEPROM_writing;
}
uint16_t RTX_RequestProtocol::EEPROM_burnProgress()
{
  return _EEPROM_burnProgress;
}

bool RTX_RequestProtocol::EEPROM_setError(uint8_t target_ip, bool setError)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00010111;
    if (setError)
      dataArray[0] |= B10000000;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00110111)
        {
          if (bytes == 1)
          {
            if ((dataArray[0] & B10000000) == setError)
            {
              _requestError = 0;
              return true;
            }
            else
            {
              _requestError = 5;
              return false;
            }
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}

bool RTX_RequestProtocol::configBytes(uint8_t target_ip, uint16_t &returnBytes)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00011011;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B11111111) == B00111011)
        {
          if (bytes == 3)
          {
            returnBytes = 0;
            returnBytes = dataArray[1];
            returnBytes |= (dataArray[2] << 8);
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::debugBytes(uint8_t target_ip, uint16_t &returnBytes)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B01011011;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B11111111) == B01111011)
        {
          if (bytes == 3)
          {
            returnBytes = 0;
            returnBytes = dataArray[1];
            returnBytes |= (dataArray[2] << 8);
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}
bool RTX_RequestProtocol::EEPROM_bytes(uint8_t target_ip, uint16_t &returnBytes)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B10011011;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B11111111) == B10111011)
        {
          if (bytes == 3)
          {
            returnBytes = 0;
            returnBytes = dataArray[1];
            returnBytes |= (dataArray[2] << 8);
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}

bool RTX_RequestProtocol::EEPROM_writeCount(uint8_t target_ip, uint32_t &returnBytes)
{
  {
    uint8_t dataArray[1];
    dataArray[0] = B00011111;
    _RTX_Protocol_pointer->write(target_ip, dataArray, 1);
  }

  uint32_t timeOutTimer = millis();

  if (_RTX_Protocol_pointer->writeError() == 0) // request sent successfully
  {
    uint8_t sender_ip;
    uint8_t dataArray[10];
    uint8_t bytes;
    while (millis() - timeOutTimer < RTX_RequestProtocol_MessageTimeOut_ms)
    {
      if (_RTX_Protocol_pointer->read(sender_ip, dataArray, bytes, true))
      {
        if (sender_ip == target_ip && bytes && (dataArray[0] & B00111111) == B00111111)
        {
          if (bytes == 5)
          {
            returnBytes = 0;
            returnBytes = dataArray[1];
            returnBytes |= (dataArray[2] << (8));
            returnBytes |= (dataArray[3] << (8 * 2));
            returnBytes |= (dataArray[4] << (8 * 3));
            _requestError = 0;
            return true;
          }
          else
          {
            _requestError = 5;
            return false;
          }
        }
      }
    }
    _requestError = 4;
  }
  else // request not sent successfully
  {
    _rtx_errorCodeSet(); // set error codes to 1, 2, or 3
  }
  return false;
}

void RTX_RequestProtocol::_rtx_errorCodeSet()
{
  if (_RTX_Protocol_pointer->writeError() == 1) // receiver_ip not connected
  {
    _requestError = 2;
  }
  else if (_RTX_Protocol_pointer->writeError() == 2) // receiver_ip buffer full
  {
    _requestError = 3;
  }
  else // rtx bus/error
  {
    _requestError = 1;
  }
}
#endif
