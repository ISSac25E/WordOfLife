// PacketBuffer
//.h
//fail, code has bugs. refer to newer versions. keep this version for example)

#ifndef PacketBuffer_h
#define PacketBuffer_h

#include "Arduino.h"

class PacketBuffer
{
public:
  PacketBuffer(bool *, uint8_t); // provide with bit array and number of bytes in array) default min will be set to 0 (0 considered to not be found/added yet)

  void set(uint16_t);   // set the lowest/min number and reset buffer
  void add(uint16_t);   // add a number to buffer(cannot be lower than min or higher than min + provided buffer)
  bool check(uint16_t); // input number and check if it already exists. return true = exists, false = does not exist/was not added yet. All number lower than min will be considered as added already
  uint16_t minNumber(); // returns the "_minNumber" var

private:
  uint8_t _index(uint8_t); // give you absolute index from relative index. Input relative index from 0: eg input - 0 = minIndex
  uint8_t _spread();       // give you spread in bytes from min to max. eg if min and max are the same, spread will be 0
  void _min_inc(uint8_t);  // increment min by input value
  void _min_dec(uint8_t);  // decrement min by input value
  void _max_inc(uint8_t);  // increment max by input value
  void _max_dec(uint8_t);  // decrement max by input value

  bool *_buffer;
  uint8_t _bufferSize;

  uint8_t _minIndex = 0;
  uint8_t _maxIndex = 0;

  uint16_t _minNumber = 0;
};

//.cpp

PacketBuffer::PacketBuffer(bool *buffer, uint8_t bufferSize)
{
  _buffer = buffer;
  _bufferSize = bufferSize;

  _buffer[0] = false;
}

void PacketBuffer::set(uint16_t setMin)
{
  _maxIndex = _minIndex;
  _minNumber = setMin;
  _buffer[_minIndex] = false;
}
void PacketBuffer::add(uint16_t setNum)
{
  if (setNum >= _minNumber)
    if (setNum - _minNumber < _bufferSize)
    {
      if (setNum - _minNumber > _spread())
      {
        uint8_t spreadVal = _spread();
        _max_inc((setNum - _minNumber) - spreadVal);
        for (uint8_t x = 0; x < (setNum - _minNumber) - spreadVal; x++)
          _buffer[_index(spreadVal + 1 + x)] = false;
        _buffer[_index((setNum - _minNumber))] = true;

        uint8_t index = 0;
        while (_buffer[_index(index)] && _spread())
        {
          index++;
          _minNumber++;
          _min_inc(1);
        }
      }
      else
      {
        _buffer[_index(setNum - _minNumber)] = true;
        uint8_t index = 0;
        while (_buffer[_index(index)] && _spread())
        {
          index++;
          _minNumber++;
          _min_inc(1);
        }
      }
    }
}
bool PacketBuffer::check(uint16_t checkNum)
{
  if (checkNum >= _minNumber && checkNum - _minNumber <= _spread())
  {
    return _buffer[_index(checkNum - _minNumber)];
  }
  if (checkNum < _minNumber)
    return true;
  return false;
}
uint16_t PacketBuffer::minNumber()
{
  return _minNumber;
}

uint8_t PacketBuffer::_index(uint8_t relativeIndex)
{
  return (_minIndex + relativeIndex) % _bufferSize;
}
uint8_t PacketBuffer::_spread()
{
  uint8_t maxIndex = _maxIndex;
  if (_minIndex > _maxIndex)
    maxIndex += _bufferSize;
  return maxIndex - _minIndex;
}
void PacketBuffer::_min_inc(uint8_t inc)
{
  _minIndex = (_minIndex + inc) % _bufferSize;
}
void PacketBuffer::_min_dec(uint8_t dec)
{
  dec %= _bufferSize;
  if (dec > _minIndex)
    _minIndex += _bufferSize;
  _minIndex -= dec;
}
void PacketBuffer::_max_inc(uint8_t inc)
{
  _maxIndex = (_maxIndex + inc) % _bufferSize;
}
void PacketBuffer::_max_dec(uint8_t dec)
{
  dec %= _bufferSize;
  if (dec > _maxIndex)
    _maxIndex += _bufferSize;
  _maxIndex -= dec;
}
#endif
