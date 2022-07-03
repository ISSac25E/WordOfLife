#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\ATEM\W5500\AtemControl\AtemControl_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Slave\OneWire_Slave_1.0.0.h"

byte mac[] =
    {0x90, 0xA2, 0xDA, 0x00, 0xE8, 0xE9};
IPAddress ip(192, 168, 86, 44);

OneWire_Slave rtx(8, 1);
AtemControl atem(IPAddress(192, 168, 86, 68));

void setup()
{
  Ethernet.begin(mac, ip);
}

void loop()
{
  uint8_t rtx_ip;
  uint8_t rtx_packet[10];
  uint8_t rtx_bytes;
  if (rtx.read(rtx_ip, rtx_packet, rtx_bytes))
  {
  }
  {
    static uint32_t sendTimer = millis();
    static bool prevAtem = false;
    static uint8_t prevTally[8];
    bool Change = false;

    if (atem.run() != prevAtem)
    {
      prevAtem = !prevAtem;
      Change = true;
    }
    for (uint8_t x = 0; x < 8; x++)
    {
      if (atem.tally(x) != prevTally[x])
      {
        Change = true;
      }
      prevTally[x] = atem.tally(x);
    }

    if (Change || millis() - sendTimer >= 500)
    {
      sendTimer = millis();
      uint8_t packet[3] = {prevAtem, 0, 0};
      for (uint8_t x = 0; x < 8; x++)
      {
        bitWrite(packet[1], x, bitRead(atem.tally(x), 0));
        bitWrite(packet[2], x, bitRead(atem.tally(x), 1));
      }

      rtx.write(4, packet, 3);
    }
  }
}