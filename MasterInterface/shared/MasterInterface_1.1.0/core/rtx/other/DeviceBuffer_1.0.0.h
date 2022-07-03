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
