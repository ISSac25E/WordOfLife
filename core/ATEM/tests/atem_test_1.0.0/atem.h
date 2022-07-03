// atem_w55
//.h
#ifndef atem_w55_h
#define atem_w55_h

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

class atem_w55
{
public:
  atem_w55(IPAddress, uint16_t); // input target ip address, and port to be used by the arduino
  void run();

private:
  EthernetUDP _udp;
  IPAddress _atem_ip;
  uint16_t _udp_port;
  bool _connected = false;
};

//.cpp

atem_w55::atem_w55(IPAddress atem_ip, uint16_t port)
{
  
}

#endif
