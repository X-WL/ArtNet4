#include "Artnet.h"

Artnet::Artnet(UDP& udp){
  this->udp = &udp;
}

void Artnet::setNetwork(uint8_t mac[], uint8_t ip[], uint8_t subnet[]){
  memcpy(this->mac_address, mac, 6);
  memcpy(this->ip_local, ip, 4);
  memcpy(this->mask_subnet, subnet, 4);
}

bool Artnet::begin(uint16_t in[], uint8_t num_in, uint16_t out[], uint8_t num_out){
  num_in_port = num_in;
  num_out_port = num_out;
  Serial.printf("Num in port = %d\n", num_in_port);
  Serial.printf("Num out port = %d\n", num_out_port);
  memcpy(in_map, in, num_in*2);
  memcpy(out_map, out, num_out*2);
  Serial.print("Artnet started!");
  udp->begin(6454);

  fill_art_node(&ArtnetNode);
  fill_art_poll_reply(&ArtPollReply, &ArtnetNode);
  fill_art_ipprog_reply(&ArtIpprogReply, &ArtnetNode);
  //printPacket((uint8_t*)&ArtPollReply);
  //printPacketByte((uint8_t*)&ArtPollReply);
  sendPackage((uint8_t *)&ArtPollReply, sizeof(ArtPollReply),0);
  
  is_Started = true;
  return true;
}

/* Main Task. */
void Artnet::vRecieverTask(void * pvParameters)
{
    for( ;; )
    {
      if (udp->parsePacket() > 0) {
        handle_packet();
      }
    }
}

bool Artnet::handle()
{
  int l = udp->parsePacket();
  if (l > 0) {
    handle_packet();
  }
}

void Artnet::handle_packet()
{
  udp->read((uint8_t *)&packetBuffer, MAX_BUFFER_UDP);
  packet_type = (artnet_packet_type_t)get_packet_type((uint8_t *)&packetBuffer);
  Serial.println("PT" + packet_type);
  if (packet_type == 0) // bad packet
  {
    return;
  }
  if (packet_type == ARTNET_DMX)
  {
    if (sizeof(packetBuffer) < sizeof(artnet_dmx_t))
      return;
    else
      handle_dmx((artnet_dmx_t *)&packetBuffer);
  }
  else if (packet_type == ARTNET_POLL)
  {
    if (sizeof(packetBuffer) < sizeof(artnet_poll_t))
      return;
    else
      handle_poll((artnet_poll_t *)&packetBuffer);
  }
  else if (packet_type == ARTNET_IPPROG)
  {
    if (sizeof(packetBuffer) < sizeof(artnet_ipprog_t))
      return;
    else
    {
      //handle_ipprog((artnet_ipprog_t *)&packetBuffer);
    }
  }
  else if (packet_type == ARTNET_ADDRESS)
  {
    if (sizeof(packetBuffer) < sizeof(artnet_address_t))
      return;
    else
      handle_address((artnet_address_t *)&packetBuffer);
  }
}

uint16_t Artnet::get_packet_type(uint8_t *packet) //this get artnet packet type
{
  if (!memcmp(packet, ArtnetNode.id, 8))
  {
    return bytes_to_short(packet[9], packet[8]);
  }
  return 0; // bad packet
}

int Artnet::handle_dmx(artnet_dmx_t *packet)
{
  Serial.print("dmx");
  for (int port = 0; port < num_in_port; port++)
  {
    if (packet->universe == in_map[port])
    {
      if (artnetDmxRecvCallback != NULL) {
        artnetDmxRecvCallback(packet->data, port);
      }
      // transmitPacket++;
      break;
    }
  }
}

int Artnet::handle_poll(artnet_poll_t *packet)
{
  if ((packet->ttm & 1) == 1) // controller say: send unicast reply
  {
    Serial.println("Unicast Reply");
    sendPackage((uint8_t *)&ArtPollReply, sizeof(ArtPollReply),1);
  }
  else // controller say: send broadcast reply
  {
    Serial.println("Broadcast Reply");
    sendPackage((uint8_t *)&ArtPollReply, sizeof(ArtPollReply),0);
  }
}

int Artnet::handle_ipprog(artnet_ipprog_t *packet)
{
  //TODO: CHANGE PORT & Settings
  Serial.println("IpProg package!");
  Serial.print("IP: ");
  Serial.print(packet->ProgIpHi);
  Serial.print(packet->ProgIp2);
  Serial.print(packet->ProgIp1);
  Serial.println(packet->ProgIpLo);
  Serial.print("Subnet Mask: ");
  Serial.print(packet->ProgSmHi);
  Serial.print(packet->ProgSm2);
  Serial.print(packet->ProgSm1);
  Serial.println(packet->ProgSmLo);
  Serial.print("Port: ");
  Serial.print(packet->ProgPortHi);
  Serial.println(packet->ProgPortLo); 
  
  // fill_art_node(&ArtNode);
  // fill_art_poll_reply(&ArtPollReply, &ArtNode);
  // fill_art_ipprog_reply(&ArtIpprogReply, &ArtNode);
  // send_reply(UNICAST, (uint8_t *)&ArtIpprogReply, sizeof(ArtIpprogReply));
  if (artnetIpProgCallback != NULL)
  {
    artnetIpProgCallback(packet);
  }
}

int Artnet::handle_address(artnet_address_t *packet) //not implemented yet
{
  // send_reply(UNICAST, (uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
}



void Artnet::fill_art_node(artnet_node_t *node)
{
  //fill to 0's
  memset(node, 0, sizeof(node));
  //  2c f4 32 5e 24 fe

  //fill data
   node->mac[0] = 44;
   node->mac[1] = 244;
   node->mac[2] = 32;
   node->mac[3] = 94;
   node->mac[4] = 24;
   node->mac[5] = 254;
  //  memcpy(node->mac, {44,244,32,94,24,254}, 6);                   // the mac address of node
  //memcpy(node->localIp, netConf.ip_local, 4);           // the IP address of node
  //memcpy(node->subnetMask, netConf.mask_subnet, 4);     // network mask (art-net use 'A' network type)

  sprintf((char *)node->id, "Art-Net\0"); // *** don't change never ***
  sprintf((char *)node->shortname, shortName);
  sprintf((char *)node->longname, longName);
  sprintf((char *)node->nodereport, report);

  //memset(node->porttypes, 0x80, ARTNET_MAX_PORTS);
  //memset(node->goodinput, 0x08, ARTNET_MAX_PORTS);
  node->numbports = 4;

  memset(node->porttypes, 0x45, 4);
  memset(node->goodinput, 0x80, 4);
  memset(node->goodoutput, 0x00, 4);

  node->subH = 0x00; // high byte of the Node Subnet Address (This field is currently unused and set to zero. It is
  // provided to allow future expansion.) (art-net III)
  node->sub = 0x00; // low byte of the Node Subnet Address

  // **************************** art-net address of universes **********************************
  // not implemented yet
  node->swin[0] = 0x00; // This array defines the 8 bit Universe address of the available input channels.
  node->swin[1] = 0x01; // values from 0x00 to 0xFF
  node->swin[2] = 0x02;
  node->swin[3] = 0x03;
  //node->swin[0] = 0x08;        // This array defines the 8 bit Universe address of the available input channels.
  //node->swin[1] = 0x09;        // values from 0x00 to 0xFF
  //node->swin[2] = 0x0A;
  //node->swin[3] = 0x0B;

  node->swout[0] = 0x00; // This array defines the 8 bit Universe address of the available output channels.
  node->swout[1] = 0x01; // values from 0x00 to 0xFF
  node->swout[2] = 0x02;
  node->swout[3] = 0x03;
  //node->swout[4] = 0x04;

  node->goodoutput[0] = 0x80;
  node->goodoutput[1] = 0x80;
  node->goodoutput[2] = 0x80;
  node->goodoutput[3] = 0x80;

  node->etsaman[0] = 0;     // The ESTA manufacturer code.
  node->etsaman[1] = 0;     // The ESTA manufacturer code.
  node->localPort = 0x1936; // artnet UDP port is by default 6454 (0x1936)
  node->verH = 0;           // high byte of Node firmware revision number.
  node->ver = 3;            // low byte of Node firmware revision number.
  node->ProVerH = 0;        // high byte of the Art-Net protocol revision number.
  node->ProVer = 14;        // low byte of the Art-Net protocol revision number.
  node->oemH = 0;           // high byte of the oem value.
  node->oem = 0xFF;         // low byte of the oem value. (0x00FF = developer code)
  node->ubea = 0;           // This field contains the firmware version of the User Bios Extension Area (UBEA). 0 if not used
  node->status = 0x08;
  node->swvideo = 0;
  node->swmacro = 0;
  node->swremote = 0;
  node->style = 0; // StNode style - A DMX to/from Art-Net device
}

void Artnet::fill_art_poll_reply(artnet_reply_t *poll_reply, artnet_node_t *node)
{
  //fill to 0's
  memset(poll_reply, 0, sizeof(poll_reply));

  //copy data from node
  memcpy(poll_reply->id, node->id, sizeof(poll_reply->id));
  memcpy(poll_reply->ip, node->localIp, sizeof(poll_reply->ip));
  memcpy(poll_reply->mac, node->mac, sizeof(poll_reply->mac));
  memcpy(poll_reply->shortname, node->shortname, sizeof(poll_reply->shortname));
  memcpy(poll_reply->longname, node->longname, sizeof(poll_reply->longname));
  memcpy(poll_reply->nodereport, node->nodereport, sizeof(poll_reply->mac));
  memcpy(poll_reply->porttypes, node->porttypes, sizeof(poll_reply->porttypes));
  memcpy(poll_reply->goodinput, node->goodinput, sizeof(poll_reply->goodinput));
  memcpy(poll_reply->goodoutput, node->goodoutput, sizeof(poll_reply->goodoutput));
  memcpy(poll_reply->swin, node->swin, sizeof(poll_reply->swin));
  memcpy(poll_reply->swout, node->swout, sizeof(poll_reply->swout));
  memcpy(poll_reply->etsaman, node->etsaman, sizeof(poll_reply->etsaman));

  poll_reply->opCode = 0x2100; // ARTNET_REPLY
  poll_reply->port = node->localPort;
  poll_reply->verH = node->verH;
  poll_reply->ver = node->ver;
  poll_reply->subH = node->subH;
  poll_reply->sub = node->sub;
  poll_reply->oemH = node->oemH;
  poll_reply->oem = node->oem;
  poll_reply->status = node->status;
  poll_reply->numbportsH = node->numbportsH;
  poll_reply->numbports = node->numbports;
  poll_reply->swvideo = node->swvideo;
  poll_reply->swmacro = node->swmacro;
  poll_reply->swremote = node->swremote;
  poll_reply->style = node->style;
}

void Artnet::fill_art_ipprog_reply(artnet_ipprog_reply_t *ipprog_reply, artnet_node_t *node)
{
  //fill to 0's
  memset(ipprog_reply, 0, sizeof(ipprog_reply));

  //copy data from node
  memcpy(ipprog_reply->id, node->id, sizeof(ipprog_reply->id));

  ipprog_reply->ProgIpHi = node->localIp[0];
  ipprog_reply->ProgIp2 = node->localIp[1];
  ipprog_reply->ProgIp1 = node->localIp[2];
  ipprog_reply->ProgIpLo = node->localIp[3];

  ipprog_reply->ProgSmHi = node->subnetMask[0];
  ipprog_reply->ProgSm2 = node->subnetMask[1];
  ipprog_reply->ProgSm1 = node->subnetMask[2];
  ipprog_reply->ProgSmLo = node->subnetMask[3];

  ipprog_reply->OpCode = 0xF900; //ARTNET_IPREPLY
  ipprog_reply->ProVerH = node->ProVerH;
  ipprog_reply->ProVer = node->ProVer;
  ipprog_reply->ProgPortHi = node->localPort >> 8;
  ipprog_reply->ProgPortLo = (node->localPort & 0xFF);
}

void Artnet::fill_art_dmx(artnet_dmx_t *art_dmx)
{
  sprintf((char *)art_dmx->id, "Art-Net\0");
  art_dmx->opCode = 0x5000; //ArtDmx package
  art_dmx->verH = 0;        // high byte of Node firmware revision number.
  art_dmx->ver = 2;         // low byte of Node firmware revision number.
  art_dmx->sequence = 0x00; //sequence off
  art_dmx->physical = 0x00; //TODO
  art_dmx->universe = 0x00;
  art_dmx->lengthHi = 0x00;
  art_dmx->length = 0x00;
  art_dmx->data[ARTNET_DMX_LENGTH];
}

void Artnet::sendPackage(uint8_t *packet, uint16_t size, const uint8_t mode){
  if (mode){// send unicast packet to controller 
    udp->beginPacket({192,168,0,109}, 6454);
    udp->write(packet, size); 
    udp->endPacket();
  } else {  
    udp->beginPacket({192,168,0,255}, 6454);
    udp->write(packet, size);
    udp->endPacket();
  }
}

void Artnet::printPacket(uint8_t *packet)
{
  for (int i = 0; i < sizeof(artnet_reply_t); i++)
  { //TODO FIX SIZEOF
    Serial.print((char)packet[i]);
    Serial.print('.');
  }
  Serial.println();
}

void Artnet::printPacketByte(uint8_t *packet)
{
  for (int i = 0; i < sizeof(artnet_reply_t); i++)
  { //TODO FIX SIZEOF
    Serial.print(packet[i]);
    Serial.print('.');
  }
  Serial.println();
}

void Artnet::setArtnetRecvCallback(ArtnetRecvCallback callback)
{
  Serial.println("set DMX callback!");
  artnetDmxRecvCallback = callback;
}

void Artnet::setArtnetTransmCallback(ArtnetTransmCallback callback)
{
  artnetDmxTransmCallback = callback;
}

void Artnet::setArtnetIpProgCallback(ArtnetIpProgCallback callback)
{
  artnetIpProgCallback = callback;
}