#ifndef ARTNET_H
#define ARTNET_H
//pragma message "Artnet v4 by X-WL version 0.1"
#define VERSION_H 0
#define VERSION 1

#include <Arduino.h>
#include "artnet_node.h"
#include "common.h"          
#include "packets.h"
#include <WiFi.h>
#ifndef ARTNET_NUM_PORT
  #define ARTNET_NUM_PORT 8
#endif

#define UNICAST               0
#define BROADCAST             1
#define ARNET_HEADER_SIZE     17
#define MAX_BUFFER_UDP 1650
#define short_get_high_byte(x)((HIGH_BYTE & x) >> 8)
#define short_get_low_byte(x)(LOW_BYTE & x)
#define bytes_to_short(h,l)( ((h << 8) & 0xff00) | (l & 0x00FF) );

#define HISTORY_SIZE 5

struct NetworkConfiguration
{
  uint8_t mac[6];
  uint8_t ip_local[4] = {192, 168, 0, 41};        // the IP address of node
  uint8_t mask_subnet[4] = {255, 255, 255, 0};        // network mask (art-net use 'A' network type)
  uint16_t port = 6454;
};

class Artnet {
  public:
    //system
    NetworkConfiguration netConf;
    WiFiUDP udp;
    char * shortName = "Magic BOX 8U\0";
    char * longName = "Magic BOX (8 PORT EDITION) by X-WL\0";
    char * report = "This controller is OK. Work with WiFi.\0";
    
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
    
    artnet_node_t             ArtnetNode;
    artnet_reply_t            ArtPollReply;
    artnet_dmx_t              sendPacket;
    artnet_ipprog_reply_t     ArtIpprogReply; //not implemented yet
    artnet_packet_type_t      packet_type;
    uint8_t packetBuffer[MAX_BUFFER_UDP];  
    byte Remote_IP[4] = { 192, 168, 0, 255 };
    uint8_t stat[ARTNET_NUM_PORT];
    uint8_t history[HISTORY_SIZE][ARTNET_NUM_PORT];
    uint8_t pointerHistory = 0;
    #define WAIT_PORT 8
    uint16_t transmitPacket = 0; 
    uint16_t in_map[ARTNET_NUM_PORT];
    uint16_t out_map[ARTNET_NUM_PORT];
    boolean isNewNetworkConfig = false;

    uint8_t data[ARTNET_NUM_PORT][512];
    
    void setArtnetRecvCallback(ArtnetRecvCallback callback);
    void setArtnetTransmCallback(ArtnetTransmCallback callback);
    void setArtnetIpProgCallback(ArtnetIpProgCallback callback);
    
    int* getData(uint8_t port, uint8_t first, uint8_t last);
    void setData(uint8_t *newdata[], uint8_t port, uint8_t first, uint8_t last);

    void historyNext();
    uint8_t getNumActivityPort();
    void printHistory();
    uint8_t getAveragePPS();
    
    boolean init(uint8_t mac[], uint8_t ip[], uint8_t subnet[]);
    void configIO(uint16_t in_mapping[], uint16_t out_mapping[]);
    void configNetwork(uint8_t mac[],uint8_t ip[], uint8_t subnet[]);
    void configNetwork(uint8_t mac[],uint8_t ip[], uint8_t subnet[], uint16_t port);
    
    bool handle();

    void handle_packet();
    uint16_t get_packet_type(uint8_t *packet);
    int handle_dmx(artnet_dmx_t *packet);
    int handle_poll(artnet_poll_t *packet);
    int handle_address(artnet_address_t *packet);
    int handle_ipprog(artnet_ipprog_t *packet);
    void send_reply(uint8_t mode_broadcast, uint8_t *packet, uint16_t size);
    void fill_art_node(artnet_node_t *node);
    void fill_art_poll_reply(artnet_reply_t *poll_reply, artnet_node_t *node);
    void fill_art_ipprog_reply(artnet_ipprog_reply_t *ipprog_reply, artnet_node_t *node);
    
    void fill_art_dmx(artnet_dmx_t *art_dmx);
    
    
    //System
    void printPacket(uint8_t *packet);
    void printPacketByte(uint8_t *packet);
};
#endif
