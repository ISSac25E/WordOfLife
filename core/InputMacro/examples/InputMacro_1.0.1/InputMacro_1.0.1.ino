#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\InputMacro\InputMacro_1.0.1.h"
// include pinDriver 1.0.1 as well for example:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\PinDriver\PinDriver_1.0.1.h"

PinDriver pin(3);          // set digital pin 3. pullup Mode automatic
InputMacro pinMacro(true); // set start state as high because pin is pullup

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  if (pinMacro(pin))
  {
    if (pinMacro && !pinMacro.prevTriggered())
    {
      Serial.print("|>> : ");
      Serial.println(pinMacro.prevInterval());
    }
    else if (!pinMacro)
    {
      Serial.print("|<< : ");
      Serial.println(pinMacro.prevInterval());
    }
  }

  if (pinMacro.interval() > 300 && !pinMacro && !pinMacro.triggered())
  {
    Serial.print("|<| : ");
    Serial.println(pinMacro.interval());
    pinMacro.trigger();
  }
  
  if (Serial.available())
  {
    while (Serial.available())
      Serial.read();
    Serial.print("||| : ");
    Serial.println(pinMacro.interval());
  }
}
