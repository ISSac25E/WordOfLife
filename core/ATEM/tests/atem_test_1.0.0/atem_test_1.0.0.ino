#include "atem.h"

byte mac[6] = {0x90, 0xA2, 0xDA, 0x5A, 0x25, 0x2D};
IPAddress atem_ip(192, 168, 86, 68);

atem_w55 atem(atem_ip, 1659);

void setup()
{
  Ethernet.begin(mac);
}
void loop()
{
}