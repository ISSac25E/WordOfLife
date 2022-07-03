// ButtonInterface
//.h
#ifndef ButtonInterface_h
#define ButtonInterface_h

#ifndef ButtonInterfaceBootDelay_ms
#define ButtonInterfaceBootDelay_ms 100 // default startup delay
#endif

#ifndef ButtonInterfaceDebounceDelay_ms
#define ButtonInterfaceDebounceDelay_ms 10 // default debounce delay is 10(ms)
#endif

#include "Arduino.h"

class ButtonInterface
{
public:
  ButtonInterface(uint8_t, bool); // setup button. inputs(pin(0-19, pullupMode(default true)))

  bool run();              // runs button and debounce. returns true if stateChange on button.
  bool state();            // return state of button
  bool stateChange();      // returns true if button changed state from last "run()" call. does not run button
  uint32_t interval();     // returns current interval of button(ms). how long the button was in the state that it currently is in
  uint32_t prevInterval(); // returns the last interval of the button since the last stateChange(ms).

private:
  bool _pinRead(); // directly read from pinport and return result

  bool _pinBoot = false; // used for button cooldown during bootup
  bool _buttonState;     // keep track of buttonState
  bool _stateChange = false;

  uint32_t _pinInterval;
  uint32_t _prevPinInterval;

  volatile uint8_t *_pinPort_PIN;  // Pin read/state register
  volatile uint8_t *_pinPort_DDR;  // Pin set register (INPUT/OUTPUT)
  volatile uint8_t *_pinPort_PORT; // Pin write register (HIGH/LOW)

  uint8_t _pinMask;    // eg. B00100000
  uint8_t _pinMaskNot; // eg. B11011111
};

//.cpp

ButtonInterface::ButtonInterface(uint8_t pin, bool pullupMode = true)
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
    *_pinPort_PORT |= _pinMask; // set to pullup
  else
    *_pinPort_PORT = _pinMaskNot; // set to floating pin

  _buttonState = _pinRead();
}

bool ButtonInterface::run()
{
  static bool pinTest = false;
  static uint32_t pinTimer = micros(); // micros should return faster then millis

  _stateChange = false;
  if (_pinBoot) // small cooldown during boot(to prevent button glitches during brownout)
  {
    if (pinTest)
    {
      if (micros() - pinTimer >= ((uint32_t)ButtonInterfaceDebounceDelay_ms * (uint32_t)1000))
      {
        if (_pinRead() != _buttonState)
        {
          _buttonState = !_buttonState;
          pinTest = false;
          _stateChange = true;
          _prevPinInterval = (millis() - _pinInterval);
          _pinInterval = millis();
        }
      }
    }
    else if (_pinRead() != _buttonState)
    {
      pinTest = true;
      pinTimer = micros();
    }
  }
  else
  {
    _buttonState = _pinRead();
    if (micros() - pinTimer >= ((uint32_t)ButtonInterfaceBootDelay_ms * (uint32_t)1000))
    {
      _pinBoot = true;
      _pinInterval = millis();
      _prevPinInterval = millis();
    }
  }
  return _stateChange;
}
bool ButtonInterface::state()
{
  return _buttonState;
}
bool ButtonInterface::stateChange()
{
  return _stateChange;
}
uint32_t ButtonInterface::interval()
{
  if (_pinBoot)
    return (millis() - _pinInterval);
  else
    return 0;
}
uint32_t ButtonInterface::prevInterval()
{
  if (_pinBoot)
    return _prevPinInterval;
  else
    return 0;
}

bool ButtonInterface::_pinRead()
{
  return (*_pinPort_PIN & _pinMask);
}

#endif
