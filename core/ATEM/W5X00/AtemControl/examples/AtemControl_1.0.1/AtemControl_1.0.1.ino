#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\ATEM\W5X00\AtemControl\AtemControl_1.0.1.h"
AtemControl atem;

void setup()
{
  Serial.begin(115200);
  Serial.println("init\n");

  byte mac[] =
      {0x90, 0xA2, 0xDA, 0x00, 0xE8, 0xE9};
  IPAddress ip(192, 168, 86, 44);

  atem.config(IPAddress(192, 168, 86, 68));
  // atem.config(IPAddress(192, 168, 86, 68), 4532); // arduino port optional
  Ethernet.begin(mac, ip);
}

void loop()
{
  if (Serial.available())
    {
      while (Serial.available())
        Serial.read();
      atem.doCut();
    }
    atem.run();
}