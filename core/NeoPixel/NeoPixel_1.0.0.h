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
