#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\rertired\ButtonInterface\ButtonInterface_1.0.0.h"

ButtonInterface button(3); // pullup mode automatically set on
// ButtonInterface button(3, false);  // << do this to turn off pullupMode
// ButtonInterface button[] = {(3), (4)}; //  do this to declare multiple buttons

void setup()
{
  Serial.begin(115200);
  Serial.println("init");
}

void loop()
{
  if (button.run()) // << state change and button run/loop
  {
    if (button.state())
    {
      Serial.print("|>> : ");
      Serial.println(button.prevInterval());
    }
    else
    {
      Serial.print("|<< : ");
      Serial.println(button.prevInterval());
    }
  }
  if (Serial.available())
  {
    Serial.print("|| : ");
    Serial.println(button.interval());
    while (Serial.available())
      Serial.read();
  }
}