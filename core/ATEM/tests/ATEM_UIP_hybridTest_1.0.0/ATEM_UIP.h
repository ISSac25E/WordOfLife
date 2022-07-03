// ATEM_UIP REV 1.0.3
//.h
#ifndef ATEM_UIP_h
#define ATEM_UIP_h
#include "Arduino.h"

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
// #include <UIPEthernet.h>
// #include <UIPUdp.h>

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\Ethernet\core\ATEM\PacketBuffer\PacketBuffer_1.0.1.h"

EthernetUDP ATEM_UDP;

class ATEM_UIP
{
public:
  bool Run();
  void Setup(IPAddress IP, uint16_t Port)
  {
    _ATEM_IP = IP;
    _UDP_Port = Port;
    _Init = true;
    _Connected = false;
    ATEM_UDP.stop();
    _ConnectTimer = millis();
  }
  void Connect()
  {
    _Connected = false;
    _Init = true;
    ATEM_UDP.stop();
    _ConnectTimer = millis();
  }
  void Disconnect()
  {
    _Connected = false;
    _Init = false;
    ATEM_UDP.stop();
  }
  void SendCommand(char *CMD, uint8_t *CMD_Packet, uint8_t CMD_Bytes);
  byte TallyInput[8];

private:
  IPAddress _ATEM_IP = IPAddress(0, 0, 0, 0);
  uint16_t _UDP_Port = 0;
  bool _Connected = false;
  char _Buffer[96];
  uint16_t _SessionID;
  uint16_t _LastRemotePacketID;
  uint16_t _SendRemotePacketID;
  uint16_t _lastTlInPacket;
  uint32_t _Timer;
  uint32_t _ConnectTimer;
  bool _PacketCount = false;
  bool _Init = false;
};

//.cpp
//#include "ATEM_UIP.h"
//#include "Arduino.h"

bool ATEM_UIP::Run()
{
  bool sendACK = false;
  if (_Init)
  {
    if (_Connected)
    {
      // Check for new Information and connection
      if ((millis() - _Timer) >= 1000)
      {
        if (!_PacketCount)
        {
          _Connected = false;
          Serial.println("Disconnected: timeout!");
          return false;
        }
        _PacketCount = false;
        _Timer = millis();
      }
      uint16_t PacketSize = ATEM_UDP.parsePacket();
      if (ATEM_UDP.available() && PacketSize)
      {
        ATEM_UDP.read(_Buffer, 12);
        uint16_t PacketLength = word(_Buffer[0] & B00000111, _Buffer[1]);
        uint16_t _PacketID = word(_Buffer[10], _Buffer[11]);
        Serial.print("rPKD ");
        Serial.print(_PacketID);
        Serial.print(" ePKD ");
        Serial.println(_LastRemotePacketID + 1);
        // I don't actually know what Packet ID 0 means yet
        // TODO: Figure out whether Packet ID 0 is why we are not always getting a stable connection:
        // if (_PacketID == 0 && PacketSize == 20 && _LastRemotePacketID != 0)
        // {
        //   _Connected = false;
        //   Serial.println("Disconnected: Packet 0");
        //   // byte Answer[12] = {
        //   //     ((12 / 256) | 0x80), (12 % 256), 0x00,
        //   //     _SessionID, ((0) / 256),
        //   //     ((0) % 256), 0, 0, 0,
        //   //     0, 0, 0};
        //   // ATEM_UDP.beginPacket(_ATEM_IP, 9910);
        //   // ATEM_UDP.write(Answer, 12);
        //   // ATEM_UDP.endPacket();
        //   _ConnectTimer = millis();
        //   return false;
        // }
        if (_PacketID == _LastRemotePacketID + 1)
        {
          _LastRemotePacketID++;
          sendACK = true;
        }
        uint8_t Command = _Buffer[0] & B11111000;
        bool CommandACK = Command & B00001000;
        if (PacketLength == PacketSize)
        {
          _PacketCount = true;
          if (PacketLength > 12)
          {
            bool ParsePacketDone = false;
            uint16_t IndexPointer = 12;
            while (IndexPointer < PacketLength && !ParsePacketDone)
            {
              ATEM_UDP.read(_Buffer, 2);
              uint16_t CommandLength = word(0, _Buffer[1]);
              if (CommandLength > 2 && CommandLength <= sizeof(_Buffer))
              {
                ATEM_UDP.read(_Buffer, CommandLength - 2);
                char CommandString[] = {
                    _Buffer[2],
                    _Buffer[3],
                    _Buffer[4],
                    _Buffer[5], '\0'};
                Serial.println(CommandString);
                if (!strcmp(CommandString, "TlIn"))
                {
                  if (_PacketID > _lastTlInPacket)
                  {
                    sendACK = true;
                    _lastTlInPacket = _PacketID;
                    _LastRemotePacketID = _PacketID;
                    TallyInput[0] = _Buffer[8];
                    TallyInput[1] = _Buffer[9];
                    TallyInput[2] = _Buffer[10];
                    TallyInput[3] = _Buffer[11];
                    TallyInput[4] = _Buffer[12];
                    TallyInput[5] = _Buffer[13];
                    TallyInput[6] = _Buffer[14];
                    TallyInput[7] = _Buffer[15];
                    Serial.print("Tally:");
                    for (uint8_t X = 0; X < 8; X++)
                    {
                      Serial.print(TallyInput[X]);
                    }
                    Serial.println();
                  }
                }
                IndexPointer += CommandLength;
              }
              else
              {
                ParsePacketDone = true;
                while (ATEM_UDP.available())
                  ATEM_UDP.read(_Buffer, sizeof(_Buffer));
              }
            }
          }
          if (CommandACK)
          {
            static bool send = true;
            // if (Serial.available())
            // {
            //   send = !send;
            //   // uint8_t CMD_Packet[] = {0, 0xCA, 0xE3, 0x82};
            //   // ATEM.SendCommand("DAut", CMD_Packet, 4);
            //   // {
            //   //   Serial.print("ACK start: ");
            //   //   Serial.println(0);
            //   //   byte Answer[12] = {
            //   //       ((12 / 256) | 0x80), (12 % 256), 0x00,
            //   //       _SessionID, ((0) / 256),
            //   //       ((0) % 256), 0, 0, 0,
            //   //       0, 0, 0};
            //   //   ATEM_UDP.beginPacket(_ATEM_IP, 9910);
            //   //   ATEM_UDP.write(Answer, 12);
            //   //   ATEM_UDP.endPacket();
            //   // }
            //   while (Serial.available())
            //     Serial.read();
            // }
            if (send && sendACK)
            {
              Serial.print("ACK: ");
              Serial.println(_LastRemotePacketID);
              byte Answer[12] = {
                  ((12 / 256) | 0x80), (12 % 256), 0x80,
                  _SessionID, ((_LastRemotePacketID) / 256),
                  ((_LastRemotePacketID) % 256), 0, 0, 0,
                  0, 0, 0};
              ATEM_UDP.beginPacket(_ATEM_IP, 9910);
              ATEM_UDP.write(Answer, 12);
              ATEM_UDP.endPacket();
            }
          }
        }
        else
        {
          while (ATEM_UDP.available())
            ATEM_UDP.read(_Buffer, sizeof(_Buffer));
        }
      }
    }
    else
    {
        ATEM_UDP.stop();
      // Connect
      if ((millis() - _ConnectTimer) >= 500)
      {
        Serial.println("Connect Attempt");
        ATEM_UDP.begin(_UDP_Port);
        {
          const byte ConnectByte[] = {
              0x10, 0x14, 0x53, 0xAB, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x3A,
              0x00, 0x00, 0x01, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00};
          while (ATEM_UDP.available())
            ATEM_UDP.read(_Buffer, sizeof(_Buffer));
          ATEM_UDP.beginPacket(_ATEM_IP, 9910);
          ATEM_UDP.write(ConnectByte, 20);
          ATEM_UDP.endPacket();
        }

        //  For some reason this Method works better But I don't like it:

        //  int B = 0;
        //  uint32_t Timer = millis();
        //  while (B != 20 && (millis() - Timer) < 10) {
        //    B = ATEM_UDP.parsePacket();
        //  }

        // Maybe this won't work:
        //        delay(5);

        uint8_t UDP_ParsePacket = ATEM_UDP.parsePacket();
        uint32_t ConnectTimer = micros();
        while (micros() - ConnectTimer < 10000 && UDP_ParsePacket != 20) // Try waiting 5ms for return Packet
        {
          UDP_ParsePacket = ATEM_UDP.parsePacket();
          // if(UDP_ParsePacket) Serial.println(UDP_ParsePacket);
        }
        if (UDP_ParsePacket == 20)
        {
          Serial.println("Connected");
          // ATEM Found and Responding:
          _Connected = true;
          _SendRemotePacketID = 1;
          _LastRemotePacketID = 0;
          _lastTlInPacket = 0;
          //      while (!ATEM_UDP.available());
          ATEM_UDP.read(_Buffer, 20);
          _SessionID = _Buffer[15];
          {
            const byte ConnectByte[] = {
                0x80, 0x0c, 0x53, 0xab, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x03, 0x00, 0x00};
            ATEM_UDP.beginPacket(_ATEM_IP, 9910);
            ATEM_UDP.write(ConnectByte, 12);
            ATEM_UDP.endPacket();
          }
          _Timer = millis();
          _PacketCount = true;
        }
        else
        {
          // Serial.println("Connecct Fail!");
          ATEM_UDP.stop();
        }
        _ConnectTimer = millis();
      }
    }
    return _Connected;
  }
  return false;
}

void ATEM_UIP::SendCommand(char *CMD, uint8_t *CMD_Packet, uint8_t CMD_Bytes)
{
  if (_Connected)
  { // Create Packet to Control ATEM. API different for each Command:
    // For now no more that 16 byte Command, More than enough for most Command Packets:
    if (CMD_Bytes <= 16)
    {
      Serial.print("SendCMD: ");
      Serial.print(CMD);
      Serial.print(" Bytes: ");
      for(uint8_t x = 0; x < CMD_Bytes; x++) {
        Serial.print(CMD_Packet[x]);
        Serial.print(" ");
      } 
      Serial.println();
      uint16_t PacketSize = 20 + CMD_Bytes;
      uint8_t AnswerPacket[PacketSize];
      for (uint8_t X = 0; X < PacketSize; X++)
        AnswerPacket[X] = 0;

      // Put the Size of the entire packet into Answer
      AnswerPacket[0] = PacketSize / 256; // Top 8 MSB Bits First;
      AnswerPacket[1] = PacketSize % 256; // Bottom 8 LSB Bits Next;
      AnswerPacket[0] |= B00001000;

      AnswerPacket[2] = 0x80;                       // API
      AnswerPacket[3] = _SessionID;                 // So ATEM Can Verify
      AnswerPacket[10] = _SendRemotePacketID / 256; // Top 8 MSB Bits First;
      AnswerPacket[11] = _SendRemotePacketID % 256; // Bottom 8 LSB Bits Next;

      // Put the Size of the CMD_Packet into Answer:
      AnswerPacket[12] = (8 + CMD_Bytes) / 256; // Top 8 MSB Bits First;
      AnswerPacket[13] = (8 + CMD_Bytes) % 256; // Bottom 8 LSB Bits Next;

      // Rest of the 16 bytes are zeros

      // Comand Identifier "CMD":
      for (uint8_t X = 0; X < 4; X++)
        AnswerPacket[16 + X] = CMD[X];
      // Next is the Command Packet bytes. This is for the rest of the Command:
      for (uint8_t X = 0; X < CMD_Bytes; X++)
        AnswerPacket[20 + X] = CMD_Packet[X];

      // Send Packet Via UDP:
      ATEM_UDP.beginPacket(_ATEM_IP, 9910);
      ATEM_UDP.write(AnswerPacket, PacketSize);
      ATEM_UDP.endPacket();

      // Increment "_SendRemotePacketID" for the next time we want to send a Command Packet:
      _SendRemotePacketID++;
    }
  }
}

#endif
