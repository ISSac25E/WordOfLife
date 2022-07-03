// AtemPacketBuffer
//.h

#ifndef AtemPacketBuffer_h
#define AtemPacketBuffer_h

#include "Arduino.h"

class AtemPacketBuffer
{
public:
  AtemPacketBuffer(bool *, uint8_t); // provide with bit array and number of bytes in array) default min will be set to 0 (0 considered to not be found/added yet)

  void set(uint16_t);   // set the lowest/min number and reset buffer
  void shift(uint16_t); // shift "minNumber" to new number while retaining as many numbers from buffer as possible. you can only shift foward. To shift backwar, you need to call "set()"
  void add(uint16_t);   // add a number to buffer(cannot be lower than min or higher than min + provided buffer)
  bool check(uint16_t); // input number and check if it already exists. return true = exists, false = does not exist/was not added yet. All number lower than min will be considered as added already
  uint16_t minNumber(); // returns the "_minNumber" var
  uint16_t maxNumber(); // returns maxNumber found

private:
  uint8_t _index(uint8_t); // give you absolute index from relative index. Input relative index from 0: eg input - 0 = minIndex
  void _min_inc();         // increment min-index by 1

  bool *_buffer;
  uint8_t _bufferSize;

  uint8_t _minIndex = 0;
  uint8_t _indexBytes = 0;

  uint16_t _minNumber = 0;
};

//.cpp

AtemPacketBuffer::AtemPacketBuffer(bool *buffer, uint8_t bufferSize)
{
  // set pointers and sizes:
  _buffer = buffer;
  _bufferSize = bufferSize;

  // set the first index to false becuase buffer was just initialized. minNumber always represents the smallest number yet "to be found"
  _buffer[_minIndex] = false;
}

void AtemPacketBuffer::set(uint16_t setMin)
{
  // set "_indexBytes" to 0, because we are starting with a new buffer and new number
  _indexBytes = 0;
  _minNumber = setMin;
  _buffer[_minIndex] = false;
}
void AtemPacketBuffer::shift(uint16_t shiftMinNum)
{
  if (shiftMinNum > _minNumber)
  {
    while (shiftMinNum > _minNumber && _indexBytes)
    {
      _indexBytes--;
      _min_inc();
      _minNumber++;
    }
    if (shiftMinNum != _minNumber)
      _minNumber = shiftMinNum;
      
    while (_buffer[_index(0)] && _indexBytes)
    {
      _indexBytes--;
      _minNumber++;
      _min_inc();
    }
    if (_buffer[_index(0)] && !_indexBytes)
    {
      _minNumber++;
      _buffer[_index(0)] = false;
    }
  }
}
void AtemPacketBuffer::add(uint16_t addNum)
{
  if (addNum >= _minNumber) // make sure "addNum" is greater than, or equal to minNumber, otherwise the number has already been added and nothing is needed to be done
  {
    uint8_t numRelativeIndex = (addNum - _minNumber); // relative index to "minNummber"

    if (numRelativeIndex < _bufferSize) // all new added numbers cannot be greater than what the buffer can fit ie. diference between the minNumber and addNum cannot exceed "bufferSize"
    {
      if (numRelativeIndex > _indexBytes) // two different proccesses when relative index goes outside of the currect buffer range(not buffer size, buffer range/"indexBytes")
      {
        uint8_t indexBytesHold = _indexBytes;                      // save the previous index value, it will be needed
        _indexBytes = numRelativeIndex;                            // set the new range/indexbytes to relativeIndex of "addNum"
        for (uint8_t x = 0; x < _indexBytes - indexBytesHold; x++) // set all the new values in the range to false.
          _buffer[_index(indexBytesHold + 1 + x)] = false;
        _buffer[_index(numRelativeIndex)] = true; // only leave the very last value as true

        // all of this may be very unnecessary(if anything begins to break in the future, refer to this) :
        /*
        while (_buffer[_index(0)] && _indexBytes)
        {
          _indexBytes--;
          _minNumber++;
          _min_inc();
        }
        if (_buffer[_index(0)] && !_indexBytes)
        {
          _minNumber++;
          _buffer[_index(0)] = false;
        }
        */
      }
      else
      {
        if (!_buffer[_index(numRelativeIndex)]) // we only need to run and check if this is false, otherwise, we alreaddy checked before at somepoint
        {
          _buffer[_index(numRelativeIndex)] = true; // set it to true, because it was just added
          while (_buffer[_index(0)] && _indexBytes) // now check if(and how much) we can advance the minNumber(and minIndex)
          {
            _minNumber++;
            _indexBytes--;
            _min_inc();
          }
          if (_buffer[_index(0)] && !_indexBytes) // Check the last byte if we can advance
          {
            _minNumber++;
            _buffer[_index(0)] = false;
          }
        }
      }
    }
  }
}
bool AtemPacketBuffer::check(uint16_t checkNum)
{
  if (checkNum >= _minNumber && checkNum - _minNumber <= _indexBytes) // check if "checkNum" is within greater than "minNumber" and within the buffer range("indexBytes")
  {
    return _buffer[_index(checkNum - _minNumber)]; // return the boolean value
  }
  if (checkNum < _minNumber) // if the value is less than the "minNumber" then it has been added before, return true
    return true;
  return false; // Other wise, requested number is too large and has not been added yet.
}
uint16_t AtemPacketBuffer::minNumber()
{
  return _minNumber; // return minNumber
}
uint16_t AtemPacketBuffer::maxNumber()
{
  if (_indexBytes)
    return _minNumber + _indexBytes; // if there are indexBytes, return the last value from the buffer range
  return _minNumber - 1;             // if there are no index bytes, the return the number before "minNumber". if "minNumber" is 0, then -1 will be returned because no number has been added to the buffer yet
}

uint8_t AtemPacketBuffer::_index(uint8_t relativeIndex)
{
  return (_minIndex + relativeIndex) % _bufferSize; // return absolute index from the inputed index relative to "minNumber"
}
void AtemPacketBuffer::_min_inc()
{
  _minIndex = (_minIndex + 1) % _bufferSize; // input the minIndex by one and loop it around the buffer if needed
}
#endif
