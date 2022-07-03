#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Slave\OneWire_Slave_1.0.0.h"

OneWire_Slave rtxSlaveDevice(8, 1); // set pin to 8 and ip to 1

void setup()
{
  Serial.begin(2000000);
  Serial.println("init");
}
void loop()
{
  static uint32_t sendTimer = millis();
  uint8_t dataArrayRead[10];
  static uint8_t dataArraySend[10];

  uint8_t dataBytes;
  uint8_t ip;
  if (rtxSlaveDevice.read(ip, dataArrayRead, dataBytes))
  {
    Serial.print("new msg ");
    Serial.print("ip: ");
    Serial.print(ip);
    Serial.print(" data: ");
    for (uint8_t x = 0; x < dataBytes; x++)
    {
      Serial.print(dataArrayRead[x]);
      Serial.print(" ");
      if(dataArraySend[x] != dataArrayRead[x]) {
        Serial.println("error");
        while(1);
      }
    }
    Serial.println("\n");
    if(dataBytes != 10) {
      Serial.println("error");
      while(1);
    }
  }
  if (millis() - sendTimer >= 50)
  {
    sendTimer = millis();
    Serial.print("send msg ");
    Serial.print("ip: ");
    Serial.print(1);
    Serial.print(" data: ");
    for (uint8_t x = 0; x < 10; x++)
    {
      dataArraySend[x] = random();
      Serial.print(dataArraySend[x]);
      Serial.print(" ");
    }
    Serial.print("  result: ");
    Serial.print(rtxSlaveDevice.write(1, dataArraySend, 10) ? "sent, errorCode: " : "not sent, errorCode: ");
    Serial.println(rtxSlaveDevice.writeError());
    if(rtxSlaveDevice.writeError()){
      Serial.println("error");
      while(1);
    }
    Serial.println();
  }
  static uint8_t prevConnected = 1;
  if (prevConnected != rtxSlaveDevice.rtxError())
  {
    prevConnected = rtxSlaveDevice.rtxError();
    Serial.print("rtxError: ");
    Serial.print(prevConnected);
    Serial.println("<<<<<<<<<<<<<\n");
    if(rtxSlaveDevice.rtxError() != 0) while(1);
  }
}