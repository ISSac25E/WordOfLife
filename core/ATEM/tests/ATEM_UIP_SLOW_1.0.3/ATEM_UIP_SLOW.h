//ATEM_UIP_SLOW REV 1.0.3
//.h
#ifndef ATEM_UIP_SLOW_h
#define ATEM_UIP_SLOW_h
#include "Arduino.h"

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

class ATEM_UIP_SLOW {
  public:
    bool Run();
    void Setup(IPAddress IP, uint16_t Port) {
      _ATEM_IP = IP;
      _UDP_Port = Port;
      _Init = true;
      _Connected = false;
      _ATEM_UDP.stop();
      _ConnectTimer = millis();
    }
    void Connect() {
      _Connected = false;
      _Init = true;
      _ATEM_UDP.stop();
      _ConnectTimer = millis();
    }
    void Disconnect() {
      _Connected = false;
      _Init = false;
      _ATEM_UDP.stop();
    }
    void SendCommand(char *CMD, uint8_t *CMD_Packet, uint8_t CMD_Bytes);
    byte TallyInput[8];

  private:
    EthernetUDP _ATEM_UDP;
    IPAddress _ATEM_IP = IPAddress(0, 0, 0, 0);
    uint16_t _UDP_Port = 0;
    bool _Connected = false;
    char _Buffer[96];
    uint16_t _SessionID;
    uint16_t _LastRemotePacketID;
    uint16_t _SendRemotePacketID;
    uint32_t _Timer;
    uint32_t _ConnectTimer;
    bool _PacketCount = false;
    bool _Init = false;
};

//.cpp
//#include "ATEM_UIP_SLOW.h"
//#include "Arduino.h"

bool ATEM_UIP_SLOW::Run() {
  if (_Init) {
    if (_Connected) {
      //Check for new Information and connection
      if ((millis() - _Timer) >= 1000) {
        if (!_PacketCount) {
          _Connected = false;
        }
        _PacketCount = false;
        _Timer = millis();
      }
      uint16_t PacketSize = _ATEM_UDP.parsePacket();
      if (_ATEM_UDP.available() && PacketSize) {
        _ATEM_UDP.read(_Buffer, 12);
        uint16_t PacketLength = word(_Buffer[0] & B00000111, _Buffer[1]);
        uint16_t _PacketID = word(_Buffer[10], _Buffer[11]);
        //I don't actually know what Packet ID 0 means yet
        //TODO: Figure out whether Packet ID 0 is why we are not always getting a stable connection:
        if (_PacketID == 0 && PacketSize == 20 && _LastRemotePacketID != 0) {
          _Connected = false;
        }
        if (_PacketID == _LastRemotePacketID + 1)
          _LastRemotePacketID++;
        else if (_PacketID < _LastRemotePacketID && _PacketID != 0) _LastRemotePacketID = _PacketID;
        uint8_t Command = _Buffer[0] & B11111000;
        bool CommandACK = Command & B00001000;
        if (PacketLength == PacketSize) {
          _PacketCount = true;
          if (PacketLength > 12) {
            bool ParsePacketDone = false;
            uint16_t IndexPointer = 12;
            while (IndexPointer < PacketLength && !ParsePacketDone) {
              _ATEM_UDP.read(_Buffer, 2);
              uint16_t CommandLength = word(0, _Buffer[1]);
              if (CommandLength > 2 && CommandLength <= sizeof(_Buffer)) {
                _ATEM_UDP.read(_Buffer, CommandLength - 2);
                char CommandString[] = {
                  _Buffer[2],
                  _Buffer[3],
                  _Buffer[4],
                  _Buffer[5], '\0'
                };
                if (!strcmp(CommandString, "TlIn")) {
                  TallyInput[0] = _Buffer[8];
                  TallyInput[1] = _Buffer[9];
                  TallyInput[2] = _Buffer[10];
                  TallyInput[3] = _Buffer[11];
                  TallyInput[4] = _Buffer[12];
                  TallyInput[5] = _Buffer[13];
                  TallyInput[6] = _Buffer[14];
                  TallyInput[7] = _Buffer[15];
                }
                IndexPointer += CommandLength;
              }
              else {
                ParsePacketDone = true;
                while (_ATEM_UDP.available()) _ATEM_UDP.read(_Buffer, sizeof(_Buffer));
              }
            }
          }
          if (CommandACK) {
            byte Answer[12] = {
              ((12 / 256) | 0x80), (12 % 256), 0x80,
              _SessionID, ((_LastRemotePacketID) / 256),
              ((_LastRemotePacketID) % 256), 0, 0, 0,
              0, 0, 0
            };
            _ATEM_UDP.beginPacket(_ATEM_IP, 9910);
            _ATEM_UDP.write(Answer, 12);
            _ATEM_UDP.endPacket();
          }
        }
        else {
          while (_ATEM_UDP.available()) _ATEM_UDP.read(_Buffer, sizeof(_Buffer));
        }
      }
    }
    else {
      //Connect
      if ((millis() - _ConnectTimer) >= 500) {
        _ATEM_UDP.begin(_UDP_Port);
        {
          const byte ConnectByte[] = {
            0x10, 0x14, 0x53, 0xAB, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x3A,
            0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00
          };
          while (_ATEM_UDP.available()) _ATEM_UDP.read(_Buffer, sizeof(_Buffer));
          _ATEM_UDP.beginPacket(_ATEM_IP, 9910);
          _ATEM_UDP.write(ConnectByte, 20);
          _ATEM_UDP.endPacket();
        }
        //    int B = 0;
        //    uint32_t Timer = millis();
        //    while (B != 20 && (millis() - Timer) < 10) {
        //      B = _ATEM_UDP.parsePacket();
        //    }

        //        //Maybe this won't work:
        //        delay(5);

        uint32_t ConnectTimer = micros();
        uint8_t UDP_ParsePacket = _ATEM_UDP.parsePacket();
        while (micros() - ConnectTimer < 5000 && UDP_ParsePacket != 20) //Try waiting 5ms for return Packet
          uint8_t UDP_ParsePacket = _ATEM_UDP.parsePacket();
        if (UDP_ParsePacket == 20) {
          //ATEM Found and Responding:
          _Connected = true;
          _SendRemotePacketID = 1;
          _LastRemotePacketID = 0;
          //      while (!_ATEM_UDP.available());
          _ATEM_UDP.read(_Buffer, 20);
          _SessionID = _Buffer[15];
          {
            const byte ConnectByte[] = {
              0x80, 0x0c, 0x53, 0xab, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x03, 0x00, 0x00
            };
            _ATEM_UDP.beginPacket(_ATEM_IP, 9910);
            _ATEM_UDP.write(ConnectByte, 12);
            _ATEM_UDP.endPacket();
          }
          _Timer = millis();
          _PacketCount = true;
        }
        else _ATEM_UDP.stop();
        _ConnectTimer = millis();
      }
    }
    return _Connected;
  }
  return false;
}

void ATEM_UIP_SLOW::SendCommand(char *CMD, uint8_t *CMD_Packet, uint8_t CMD_Bytes) {
  //Create Packet to Control ATEM. API different for each Command:
  //For now no more that 16 byte Command, More than enough for most Command Packets:
  if (CMD_Bytes <= 16) {
    uint16_t PacketSize = 20 + CMD_Bytes;
    uint8_t AnswerPacket[PacketSize];
    for (uint8_t X = 0; X < PacketSize; X++) AnswerPacket[X] = 0;

    //Put the Size of the entire packet into Answer
    AnswerPacket[0] = PacketSize / 256; //Top 8 MSB Bits First;
    AnswerPacket[1] = PacketSize % 256; //Bottom 8 LSB Bits Next;
    AnswerPacket[0] |= B00001000;

    AnswerPacket[2] = 0x80; //API
    AnswerPacket[3] = _SessionID; //So ATEM Can Verify
    AnswerPacket[10] = _SendRemotePacketID / 256; //Top 8 MSB Bits First;
    AnswerPacket[11] = _SendRemotePacketID % 256; //Bottom 8 LSB Bits Next;

    //Put the Size of the CMD_Packet into Answer:
    AnswerPacket[12] = (8 + CMD_Bytes) / 256; //Top 8 MSB Bits First;
    AnswerPacket[13] = (8 + CMD_Bytes) % 256; //Bottom 8 LSB Bits Next;

    //Rest of the 16 bytes are zeros

    //Comand Identifier "CMD":
    for (uint8_t X = 0; X < 4; X++) AnswerPacket[16 + X] = CMD[X];
    //Next is the Command Packet bytes. This is for the rest of the Command:
    for (uint8_t X = 0; X < CMD_Bytes; X++) AnswerPacket[20 + X] = CMD_Packet[X];

    //Send Packet Via UDP:
    _ATEM_UDP.beginPacket(_ATEM_IP, 9910);
    _ATEM_UDP.write(AnswerPacket, PacketSize);
    _ATEM_UDP.endPacket();

    //Increment "_SendRemotePacketID" for the next time we want to send a Command Packet:
    _SendRemotePacketID++;
  }
}

#endif
