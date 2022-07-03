#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\PinDriver\PinDriver_1.0.1.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\InputMacro\InputMacro_1.0.0.h"

// set pin and inputMacros:
PinDriver pin[3] = {PinDriver(2), PinDriver(3), PinDriver(4)}; // pin goes from left-most button(pin[0]) to right-most button(pin[2])
InputMacro pinMacro[3] = {InputMacro(true), InputMacro(true), InputMacro(true)};

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  if (pinMacro[0](pin[0]))
  {
    if (pinMacro[0])
    {
      Serial.print("0|>> : ");
      Serial.println(pinMacro[0].prevInterval());
    }
    else
    {
      Serial.print("0|<< : ");
      Serial.println(pinMacro[0].prevInterval());
    }
  }
  if (pinMacro[1](pin[1]))
  {
    if (pinMacro[1])
    {
      Serial.print("1|>> : ");
      Serial.println(pinMacro[1].prevInterval());
    }
    else
    {
      Serial.print("1|<< : ");
      Serial.println(pinMacro[1].prevInterval());
    }
  }
  if (pinMacro[2](pin[2]))
  {
    if (pinMacro[2])
    {
      Serial.print("2|>> : ");
      Serial.println(pinMacro[2].prevInterval());
    }
    else
    {
      Serial.print("2|<< : ");
      Serial.println(pinMacro[2].prevInterval());
    }
  }

  if (Serial.available())
  {
    while (Serial.available())
      Serial.read();
    Serial.print("0 ||| : ");
    Serial.println(pinMacro[0].interval());

    Serial.print("1 ||| : ");
    Serial.println(pinMacro[1].interval());

    Serial.print("2 ||| : ");
    Serial.println(pinMacro[2].interval());
  }
}