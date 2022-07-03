// ConfigEEPROM
//.h

/*
  changes from previous version:
    - now you can input 0 sectors and it will not change it to 1 sector
    - when writing, it will still write entire EE if "WriteEntireEE" is set(even if there are 0 secotrs set). Otherwise it will only increment writecount each time
    - when reading, ed(error detection) will always return true and you will also return an empty buffer(because 0 sectors to read)
*/

#ifndef ConfigEEPROM_h
#define ConfigEEPROM_h

#ifndef ConfigEEPROM_Size
#define ConfigEEPROM_Size 1024 // Atmega328p default
#endif

#ifndef ConfigEEPROM_MaxSector
#define ConfigEEPROM_MaxSector ((ConfigEEPROM_Size - 4) / 11)
#endif

#include "Arduino.h"
#include "EEPROM.h"

class ConfigEEPROM
{
public:
  ConfigEEPROM(uint8_t, bool); // set number of sectors and weather you want the entire eeprom flashed every time(true)(active sectors and inactive) or just the active sectors(false). input(numberOfSectors, writeEntireEE)
  bool read(uint8_t *);        // read the entire configured eeprom and check ed(error detection). input(dataArray(return)) dataArray must be the size of (sectors * 8). output true = ed successful/no faults found, false = ed failed/bad eeprom/corrupted
  void write(uint8_t *);       // SET write to eeprom. input(dataArray). dataArray must be the size of (sectors * 8). the actual writing will be handled in "run()"
  bool write();                // returns weather or not the eeprom is currently writing. return true = writeing, false = not writing(standby)
  bool run();                  // handles write proccess to eeprom. Does ed and writes only one byte each run until writing is completed. returns true if writing, false if done writing.
  uint16_t progressBytes();    // returns how many bytes have been writen so far. returns 0 if not writing or just beginning
  uint16_t totalBytes();       // return total number of byte to be written/active bytes
  uint32_t writeCount();       // reads how many times eeprom was written to. Refreshes each time called from eeprom

private:
  bool _writeEntireEE;
  bool _writing = false;
  uint8_t _sectors = 0;
  uint8_t *_writeBuffer;
  uint16_t _writeProgress = 0;
  uint16_t _totalBytes = ConfigEEPROM_Size;
  void _readED(uint8_t *, uint8_t);
  void _writeED(uint8_t *, uint8_t, uint8_t);
};

//.cpp
//#include "ConfigEEPROM.h"
//#include "Arduino.h"

ConfigEEPROM::ConfigEEPROM(uint8_t sectors, bool writeEntireEE = true)
{
  if (sectors <= ConfigEEPROM_MaxSector)
    _sectors = sectors;
  else
    _sectors = ConfigEEPROM_MaxSector;
  _writeEntireEE = writeEntireEE;
  if (_writeEntireEE)
    _totalBytes = ConfigEEPROM_Size;
  else
    _totalBytes = (_sectors * 11) + 4;
}

bool ConfigEEPROM::read(uint8_t *buff)
{
  if (!_writing)
  {
    // Fill Entire Buffer first:
    for (uint16_t x = 0; x < (_sectors * 8); x++)
      buff[x] = EEPROM.read(x);
    for (uint8_t x = 0; x < _sectors; x++)
    {
      // ed(error detection) buffer[] {H_Parity, V_Parity, CheckSum}:
      uint8_t edBuff[3];
      uint8_t testParity[2] = {0, 0};
      _readED(edBuff, x);

      // H Parity:
      for (uint8_t _byte = 0; _byte < 8; _byte++)
      {
        for (uint8_t _bit = 0; _bit < 8; _bit++)
        {
          if (bitRead(buff[(8 * x) + _byte], _bit))
            testParity[0] & (1 << _byte) ? testParity[0] &= ~(1 << _byte) : testParity[0] |= (1 << _byte);
        }
      }
      // V Parity:
      for (uint8_t _bit = 0; _bit < 8; _bit++)
      {
        for (uint8_t _byte = 0; _byte < 8; _byte++)
        {
          if (bitRead(buff[(8 * x) + _byte], _bit))
            testParity[1] & (1 << _bit) ? testParity[1] &= ~(1 << _bit) : testParity[1] |= (1 << _bit);
        }
      }
      // CheckSum:
      uint16_t testCheckSum = 85; // Set ChckSum to 85(B01010101) as a control
      for (uint8_t _byte = 0; _byte < 8; _byte++)
        testCheckSum += buff[(8 * x) + _byte];
      testCheckSum += testParity[0];
      testCheckSum += testParity[1];
      testCheckSum += (testCheckSum >> 8);
      testCheckSum &= B11111111;

      if (testParity[0] != edBuff[0] ||
          testParity[1] != edBuff[1] ||
          testCheckSum != edBuff[2])
        return false;
    }
    return true;
  }
  else
    return false;
}

void ConfigEEPROM::write(uint8_t *buff)
{
  if (!_writing)
  {
    _writing = true;
    _writeProgress = 0;
    _writeBuffer = buff;
  }
}

bool ConfigEEPROM::write()
{
  return _writing;
}

bool ConfigEEPROM::run()
{
  if (_writing)
  {
    if (_writeEntireEE)
    {
      if (_writeProgress < (ConfigEEPROM_MaxSector * 11))
      {
        if (_writeProgress < (_sectors * 11))
        {
          static uint8_t sectorData[8];
          static uint8_t sectorED[3]; // {H_Parity, V_Parity, CheckSum}
          // Write to active sectors
          if (!(_writeProgress % 11))
          {
            // Start of a new sector
            for (uint8_t _byte = 0; _byte < 8; _byte++)
              sectorData[_byte] = _writeBuffer[((_writeProgress / 11) * 8) + _byte];

            // H Parity:
            sectorED[0] = 0;
            for (uint8_t _byte = 0; _byte < 8; _byte++)
            {
              for (uint8_t _bit = 0; _bit < 8; _bit++)
              {
                if (bitRead(sectorData[_byte], _bit))
                  sectorED[0] & (1 << _byte) ? sectorED[0] &= ~(1 << _byte) : sectorED[0] |= (1 << _byte);
              }
            }
            // V Parity:
            sectorED[1] = 0;
            for (uint8_t _bit = 0; _bit < 8; _bit++)
            {
              for (uint8_t _byte = 0; _byte < 8; _byte++)
              {
                if (bitRead(sectorData[_byte], _bit))
                  sectorED[1] & (1 << _bit) ? sectorED[1] &= ~(1 << _bit) : sectorED[1] |= (1 << _bit);
              }
            }
            // CheckSum:
            uint16_t checkSum = 85; // Set ChckSum to 85(B01010101) as a control
            for (uint8_t _byte = 0; _byte < 8; _byte++)
              checkSum += sectorData[_byte];
            checkSum += sectorED[0];
            checkSum += sectorED[1];
            checkSum += (checkSum >> 8);
            sectorED[2] = checkSum;
          }
          if ((_writeProgress % 11) < 8)
          {
            // write data
            EEPROM.write(((_writeProgress / 11) * 8) + (_writeProgress % 11), sectorData[(_writeProgress % 11)]);
            _writeProgress++;
          }
          else
          {
            // write ed
            _writeED(sectorED, (_writeProgress / 11), (_writeProgress % 11) - 8);
            _writeProgress++;
          }
        }
        else
        {
          // write to inactive sectors for consistancy
          if ((_writeProgress % 11) < 8)
          {
            uint16_t EEPROM_address = ((_writeProgress / 11) * 8) + (_writeProgress % 11);
            EEPROM.write(EEPROM_address, EEPROM.read(EEPROM_address));
            _writeProgress++;
          }
          else
          {
            uint16_t EEPROM_address = (ConfigEEPROM_MaxSector * 8) + ((_writeProgress / 11) * 3) + (_writeProgress % 11) - 8;
            EEPROM.write(EEPROM_address, EEPROM.read(EEPROM_address));
            _writeProgress++;
          }
        }
      }
      else if (_writeProgress >= (ConfigEEPROM_Size - 4))
      {
        // write "writeCount" Bytes, and exit EEPROM write
        static uint32_t writeCountHold = 0;
        if (_writeProgress == (ConfigEEPROM_Size - 4))
        {
          writeCountHold = writeCount();
          writeCountHold++;
        }
        EEPROM.write(_writeProgress, (writeCountHold >> (8 * ((ConfigEEPROM_Size - 1) - _writeProgress))));
        _writeProgress++;
        if (_writeProgress == totalBytes())
        {
          _writing = false;
          _writeProgress = 0;
        }
      }
      else
      {
        EEPROM.write(_writeProgress, EEPROM.read(_writeProgress));
        _writeProgress++;
      }
    }
    else
    {
      if (_writeProgress < (_sectors * 11))
      {
        static uint8_t sectorData[8];
        static uint8_t sectorED[3]; // {H_Parity, V_Parity, CheckSum}
        // Write to active sectors
        if (!(_writeProgress % 11))
        {
          // Start of a new sector
          for (uint8_t _byte = 0; _byte < 8; _byte++)
            sectorData[_byte] = _writeBuffer[((_writeProgress / 11) * 8) + _byte];

          // H Parity:
          sectorED[0] = 0;
          for (uint8_t _byte = 0; _byte < 8; _byte++)
          {
            for (uint8_t _bit = 0; _bit < 8; _bit++)
            {
              if (bitRead(sectorData[_byte], _bit))
                sectorED[0] & (1 << _byte) ? sectorED[0] &= ~(1 << _byte) : sectorED[0] |= (1 << _byte);
            }
          }
          // V Parity:
          sectorED[1] = 0;
          for (uint8_t _bit = 0; _bit < 8; _bit++)
          {
            for (uint8_t _byte = 0; _byte < 8; _byte++)
            {
              if (bitRead(sectorData[_byte], _bit))
                sectorED[1] & (1 << _bit) ? sectorED[1] &= ~(1 << _bit) : sectorED[1] |= (1 << _bit);
            }
          }
          // CheckSum:
          uint16_t checkSum = 85; // Set ChckSum to 85(B01010101) as a control
          for (uint8_t _byte = 0; _byte < 8; _byte++)
            checkSum += sectorData[_byte];
          checkSum += sectorED[0];
          checkSum += sectorED[1];
          checkSum += (checkSum >> 8);
          sectorED[2] = checkSum;
        }
        if ((_writeProgress % 11) < 8)
        {
          // write data
          EEPROM.write(((_writeProgress / 11) * 8) + (_writeProgress % 11), sectorData[(_writeProgress % 11)]);
          _writeProgress++;
        }
        else
        {
          // write ed
          _writeED(sectorED, (_writeProgress / 11), (_writeProgress % 11) - 8);
          _writeProgress++;
        }
      }
      else
      {
        // write "writeCount" Bytes, and exit EEPROM write
        static uint32_t writeCountHold = 0;
        if (_writeProgress == (_sectors * 11))
        {
          writeCountHold = writeCount();
          writeCountHold++;
        }
        EEPROM.write((ConfigEEPROM_Size - 4) + (_writeProgress - (_sectors * 11)), (writeCountHold >> (8 * ((((_sectors * 11) + 3) - _writeProgress)))));
        _writeProgress++;
        if (_writeProgress == totalBytes())
        {
          _writing = false;
          _writeProgress = 0;
        }
      }
    }
  }
  return _writing;
}

uint16_t ConfigEEPROM::progressBytes()
{
  return _writeProgress;
}

uint16_t ConfigEEPROM::totalBytes()
{
  return _totalBytes;
}

uint32_t ConfigEEPROM::writeCount()
{
  uint32_t writeCountHold = 0;
  for (uint8_t x = 0; x < 4; x++)
    writeCountHold += (EEPROM.read((ConfigEEPROM_Size - 1) - x) << (8 * x));
  return writeCountHold;
}

void ConfigEEPROM::_readED(uint8_t *buff, uint8_t sector)
{
  buff[0] = EEPROM.read((ConfigEEPROM_MaxSector * 8) + (sector * 3) + 0);
  buff[1] = EEPROM.read((ConfigEEPROM_MaxSector * 8) + (sector * 3) + 1);
  buff[2] = EEPROM.read((ConfigEEPROM_MaxSector * 8) + (sector * 3) + 2);
}

void ConfigEEPROM::_writeED(uint8_t *buff, uint8_t sector, uint8_t buffval)
{
  if (buffval < 3)
    EEPROM.write((ConfigEEPROM_MaxSector * 8) + (sector * 3) + buffval, buff[buffval]);
}
#endif
