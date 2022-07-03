#include "ATEM_UIP.h"

ATEM_UIP ATEM;

byte ARD_MAC[] = {
    0x90, 0xA2, 0xDA, 0x00, 0xE8, 0xE9}; // <= SETUP
IPAddress ARD_IP(192, 168, 86, 44);

void setup()
{
  Serial.begin(115200);
  Serial.print("Init\n");
  Ethernet.begin(ARD_MAC, ARD_IP);
  ATEM.Setup(IPAddress(192, 168, 86, 68), 56417);
}

void loop()
{
  ATEM.Run();
  // if (Serial.available())
  // {
  //   // uint8_t CMD_Packet[] = {0, 0xCA, 0xE3, 0x82};
  //   // ATEM.SendCommand("DAut", CMD_Packet, 4);
  //   {
  //     Serial.print("ACK start: ");
  //     Serial.println(4);
  //     byte Answer[12] = {
  //         ((12 / 256) | 0x80), (12 % 256), 0x80,
  //         _SessionID, ((4) / 256),
  //         ((4) % 256), 0, 0, 0,
  //         0, 0, 0};
  //     ATEM_UDP.beginPacket(_ATEM_IP, 9910);
  //     ATEM_UDP.write(Answer, 12);
  //     ATEM_UDP.endPacket();
  //   }
  //   while (Serial.available())
  //     Serial.read();
  // }
  // static uint32_t timer = millis();
  // if (millis() - timer >= 200)
  // {
  //   timer = millis();
  //   Serial.print(ATEM.Run());
  //   Serial.print(": ");
  //   for (uint8_t X = 0; X < 8; X++)
  //   {
  //     Serial.print(ATEM.TallyInput[X]);
  //   }
  //   Serial.println();
  // }
}
