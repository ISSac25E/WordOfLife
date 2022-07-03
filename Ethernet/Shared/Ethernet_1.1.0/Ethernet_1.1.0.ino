// OneWire Slave, RTX Protocol will be used in the future:
#include "core\rtx\OneWire_Slave_1.0.0.h"
// use atem lib:
#include "core\atem\AtemControl_1.0.0.h"
// include VarPar, useful for state changes:
#include "core\other\VarPar_1.0.1.h"

// set network credentials:
// mac address of arduino:
byte network_mac[] = {0x90, 0xA2, 0xDA, 0x00, 0xE8, 0xE9};
// ip address of arduino:
IPAddress network_ip(192, 168, 7, 69);
// set atem ip location:
IPAddress atem_network_ip(192, 168, 7, 75);

// set atem instance:
AtemControl atem(atem_network_ip); // set ip

// set OneWire rtx:
OneWire_Slave rtx(8, 1); // set pin(8) and ip(1), 1 is default for ethernet
const uint8_t pedal_ip = 3;

// set connection states and tally:
Par_bool atem_connected = false;
Par_uint8_t atem_programTally = 0;
Par_uint8_t atem_previewTally = 0;

void setup()
{
  Ethernet.begin(network_mac, network_ip);
}

void loop()
{
  rtxHandle();
  atemHandle();
}

void rtxHandle()
{
  uint8_t sender_ip;
  uint8_t dataArray[10]; // prepare for max possible bytes
  uint8_t dataBytes;
  if (rtx.read(sender_ip, dataArray, dataBytes))
  {
    if (dataBytes && sender_ip == pedal_ip)
    {
      switch (dataArray[0])
      {
        // command 0: null
      case 1:
        if (dataBytes == 1)
          atem.doCut();
        break;
      case 2:
        if (dataBytes == 1)
          atem.doAuto();
        break;
      case 3:
        if (dataBytes == 2)
          atem.previewInput(dataArray[1]);
        break;
      case 4:
        if (dataBytes == 2)
          atem.programInput(dataArray[1]);
        break;
      case 5:
        if (dataBytes == 1)
          atem.ftb();
        break;
      }
    }
  }
  static uint32_t sendTimer = millis();
  bool change = false;
  if (atem_connected.change())
    change = true;
  if (atem_previewTally.change())
    change = true;
  if (atem_programTally.change())
    change = true;

  if (change || millis() - sendTimer >= 500)
  {
    sendTimer = millis();
    uint8_t dataArray[3];
    dataArray[0] = 0;
    dataArray[0] = atem_connected;
    dataArray[1] = atem_programTally;
    dataArray[2] = atem_previewTally;
    rtx.write(pedal_ip, dataArray, 3);
  }
}

void atemHandle()
{
  atem_connected = atem.run();

  uint8_t atem_programTally_newVal = 0;
  uint8_t atem_previewTally_newVal = 0;

  for (uint8_t x = 0; x < 8; x++)
  {
    bitWrite(atem_programTally_newVal, x, bitRead(atem.tally(x), 0));
    bitWrite(atem_previewTally_newVal, x, bitRead(atem.tally(x), 1));
  }

  atem_programTally = atem_programTally_newVal;
  atem_previewTally = atem_previewTally_newVal;
}