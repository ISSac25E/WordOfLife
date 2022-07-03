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
