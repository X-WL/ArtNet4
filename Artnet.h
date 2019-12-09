#ifndef Artnet_h
#define Artnet_h
//pragma message "Artnet v4 by X-WL version 0.3"
#define VERSION_H 0
#define VERSION 3

#include <Arduino.h>
#include "artnet_node.h"
#include "common.h"
#include "packets.h"
//#include <WiFi.h>
//#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
#include <Udp.h>

#define UNICAST 0
#define BROADCAST 1
#define ARNET_HEADER_SIZE 17
#define MAX_BUFFER_UDP 1650
#define short_get_high_byte(x) ((HIGH_BYTE & x) >> 8)
#define short_get_low_byte(x) (LOW_BYTE & x)
#define bytes_to_short(h, l) (((h << 8) & 0xff00) | (l & 0x00FF));

class Artnet
{
  private:
    UDP* udp;
    uint8_t mac_address[6];
    uint8_t ip_local[4] = {192, 168, 0, 41};     // the IP address of node
    uint8_t mask_subnet[4] = {255, 255, 255, 0}; // network mask (art-net use 'A' network type)
  
    uint8_t num_in_port = 0;
    uint8_t num_out_port = 0;
    uint16_t in_map[8];
    uint16_t out_map[8];
    bool is_Started = false;
  public:
    Artnet(UDP& udp);
    void setNetwork(uint8_t mac[], uint8_t ip[], uint8_t subnet[]);
    bool begin(uint16_t in[], uint8_t num_in, uint16_t out[], uint8_t num_out);

    void vRecieverTask(void * pvParameters);

  char *shortName = "ARTNET BOX 4U\0";
  char *longName = "ArtNet v4 by X-WL\0";
  char *report = "This controller is OK. Work with WiFi.\0";

public:


  typedef void (*ArtnetRecvCallback)(uint8_t *data, uint16_t port);
  ArtnetRecvCallback artnetDmxRecvCallback;

  typedef void (*ArtnetTransmCallback)(uint8_t *data, uint16_t port);
  ArtnetTransmCallback artnetDmxTransmCallback;

  typedef void (*ArtnetIpProgCallback)(artnet_ipprog_t *packet);
  ArtnetIpProgCallback artnetIpProgCallback;
  ///
  long lastSendTime = 0;
  uint8_t artnetSendDelay = 1000 / 25;
  uint8_t ip_send[4] = {192, 168, 0, 10};

  artnet_node_t ArtnetNode;
  artnet_reply_t ArtPollReply;
  artnet_dmx_t sendPacket;
  artnet_ipprog_reply_t ArtIpprogReply; //not implemented yet
  artnet_packet_type_t packet_type;
  uint8_t packetBuffer[MAX_BUFFER_UDP];

  
  boolean isNewNetworkConfig = false;

  void setArtnetRecvCallback(ArtnetRecvCallback callback);
  void setArtnetTransmCallback(ArtnetTransmCallback callback);
  void setArtnetIpProgCallback(ArtnetIpProgCallback callback);

  bool handle();

  void sendPackage(uint8_t *packet, uint16_t size, const uint8_t mode);

  void handle_packet();
  uint16_t get_packet_type(uint8_t *packet);
  int handle_dmx(artnet_dmx_t *packet);
  int handle_poll(artnet_poll_t *packet);
  int handle_address(artnet_address_t *packet);
  int handle_ipprog(artnet_ipprog_t *packet);
  
  void fill_art_node(artnet_node_t *node);
  void fill_art_poll_reply(artnet_reply_t *poll_reply, artnet_node_t *node);
  void fill_art_ipprog_reply(artnet_ipprog_reply_t *ipprog_reply, artnet_node_t *node);

  void fill_art_dmx(artnet_dmx_t *art_dmx);

  //System
  void printPacket(uint8_t *packet);
  void printPacketByte(uint8_t *packet);
};
#endif
