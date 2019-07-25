#include "Artnet.h"

void Artnet::setArtnetRecvCallback(ArtnetRecvCallback callback) {
    Serial.println("set DMX callback!");
    artnetDmxRecvCallback = callback;
}

void Artnet::setArtnetTransmCallback(ArtnetTransmCallback callback) {
    artnetDmxTransmCallback = callback;
}

void Artnet::setArtnetIpProgCallback(ArtnetIpProgCallback callback) {
    artnetIpProgCallback = callback;
}

int* Artnet::getData(uint8_t port, uint8_t first, uint8_t last) {
  int* outData = new int[last - first + 1];
   // uint8_t outData = new uint8_t[last - first + 1];
  int p = 0;
  for (int i = first; i <= last; i++) {
    //outData[p++] = data[port][i];
  }
  return outData;
}

void Artnet::setData(uint8_t *newdata[], uint8_t port, uint8_t first, uint8_t last) {
  int p = 0;
  for (int i = first; i <= last; i++) {
    data[port][i] = *newdata[p++];
  }
}

void Artnet::historyNext() {
  if (pointerHistory++ == HISTORY_SIZE) {
    pointerHistory = 0;
  }
  for (int i = 0; i < 8; i++) {
    history[pointerHistory][i] = 0;
  }
}

uint8_t Artnet::getNumActivityPort() {
  uint8_t numPort = 0;
  uint8_t pointer = 0;
  if ((pointerHistory - 1) >= 0) {
    pointer = pointerHistory - 1;
  } else {
    pointer = HISTORY_SIZE - 1;
  }
  for (int i = 0; i < 8; i++) {
    if (history[pointer][i] > 0) {
      numPort++;
    }
  }
  return numPort;
}

void Artnet::printHistory() {
  for (int i = 0; i < 8; i++) {
    uint16_t sumPackage = 0;
    for (int j = 0; j < HISTORY_SIZE; j++) {
      sumPackage += history[j][i];
    }
    Serial.print(' ');
    Serial.print((float)sumPackage / (float)HISTORY_SIZE);
  }
  Serial.println();
  historyNext();
}

uint8_t Artnet::getAveragePPS() {
  uint16_t sumPackage = 0;
  for (int i = 0; i < 8; i++) {
    sumPackage += history[pointerHistory][i];
  }
  return sumPackage / 8;
}

boolean Artnet::init(uint8_t mac[], uint8_t ip[], uint8_t subnet[]) {
  udp.begin(6454);

  uint16_t in_mapping[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  uint16_t out_mapping[8] = { -1, -1, -1, -1, -1, -1, -1, -1};

  configIO(in_mapping, out_mapping);
  configNetwork(mac, ip, subnet);
  fill_art_node(&ArtnetNode);
  fill_art_poll_reply(&ArtPollReply, &ArtnetNode);
  fill_art_ipprog_reply  (&ArtIpprogReply, &ArtnetNode);
  //printPacket((uint8_t*)&ArtPollReply);
  //printPacketByte((uint8_t*)&ArtPollReply);
  send_reply(BROADCAST, (uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
}

/**
   This method inizialize In-Out mapping.
   Send uint16_t pointer for array [8] : {0,1,2,3,4,5,6,7}.
   Set port "-1" from unusing port.
*/
void Artnet::configIO(uint16_t in_mapping[], uint16_t out_mapping[]) {
  for (int i = 0; i < ARTNET_NUM_PORT; i++) {
    in_map[i] = in_mapping[i];
    out_map[i] = out_mapping[i];
  }
}

void Artnet::configNetwork(uint8_t mac[], uint8_t ip[], uint8_t subnet[]) {
  memcpy(ArtnetNode.mac, mac, 6);        
  memcpy(ArtnetNode.localIp, ip, 4);          
  memcpy(ArtnetNode.subnetMask, subnet, 4);     
}

void Artnet::configNetwork(uint8_t mac[], uint8_t ip[], uint8_t subnet[], uint16_t port) {
  memcpy(ArtnetNode.mac, mac, 6);                   
  memcpy(ArtnetNode.localIp, ip, 4);           
  memcpy(ArtnetNode.subnetMask, subnet, 4);   
  ArtnetNode.localPort = port;
}

uint8_t replyC= 0;

bool Artnet::handle() {
    /*if (replyC++==0){
    //EVERY_N_MILLISECONDS( 2000 ) {
    send_reply(BROADCAST, (uint8_t *)&ArtPollReply, sizeof(ArtPollReply));  //Need FIX Art_Poll, Art_Replay
  }*/
  
  int l = udp.parsePacket();
  if (l > 0) {
      Serial.println('h');
    handle_packet();
  }
    /*
    if (artnetDmxTransmCallback != NULL) {
        if (((float)(ESP.getCycleCount() - lastSendTime) / 240) >= artnetSendDelay){
        //EVERY_N_MILLISECONDS( artnetSendDelay ) {
            for (int i = 0; i < ARTNET_NUM_PORT; i++) {
                if (out_map[i] > 0) {
                    //artnet_dmx_t sendPacket;
                    //sendPacket
                    sendPacket.universe = out_map[i];
                    artnetDmxTransmCallback(sendPacket.data, i);
                }
                
            }
        //}
            lastSendTime = ESP.getCycleCount();
        }
    }*/
    /*EVERY_N_MILLISECONDS( 1000 ) {
#if (DEBUG_STAT == 1)
    printHistory();
#endif
  }*/
}

void Artnet::handle_packet()
{
  udp.read((uint8_t *)&packetBuffer, MAX_BUFFER_UDP);
  packet_type = (artnet_packet_type_t)get_packet_type((uint8_t *)&packetBuffer);
  Serial.println("PT" + packet_type);
  if (packet_type == 0)  // bad packet
  {
    return;
  }
  if (packet_type == ARTNET_DMX)
  {
    if (sizeof(packetBuffer) < sizeof(artnet_dmx_t))
      return;
    else
      handle_dmx((artnet_dmx_t *)&packetBuffer);
  } else if (packet_type == ARTNET_POLL)
  {
#ifdef ARTNETDEBUG
    Serial.print("ArtPoll package! IP: ");
    Serial.print(udp.remoteIP());
    Serial.print(" Port: ");
    Serial.println(udp.remotePort());
#endif
    if (sizeof(packetBuffer) < sizeof(artnet_poll_t))
      return;
    else
      handle_poll((artnet_poll_t *)&packetBuffer);
  } else if (packet_type == ARTNET_IPPROG)
  {
    if (sizeof(packetBuffer) < sizeof(artnet_ipprog_t))
      return;
    else  {
#if BETA_FUNCTION
      handle_ipprog((artnet_ipprog_t *)&packetBuffer);
#endif
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
  return 0;  // bad packet
}

int Artnet::handle_dmx(artnet_dmx_t *packet) {
    Serial.print("dmx");
    
    for (int port = 0; port < ARTNET_NUM_PORT; port++) {
        if (packet->universe == in_map[port]) {
#ifdef ARTNETDEBUG
            Serial.print("U-");
            Serial.print(packet->universe);
#endif
            //if (artnetDmxRecvCallback != NULL) {
                //artnetDmxRecvCallback(packet->data, port);
           // }
#ifdef ARTNETDEBUG
            //history[pointerHistory][i]++;
            transmitPacket++;
#endif
            break;
        }
    }
}

int Artnet::handle_poll(artnet_poll_t *packet)
{
  if ((packet->ttm & 1) == 1) // controller say: send unicast reply
  {
    Serial.println("Unicast Reply");
    send_reply(UNICAST, (uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
  }
  else // controller say: send broadcast reply
  {
    Serial.println("Broadcast Reply");
    send_reply(BROADCAST, (uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
  }
}

int Artnet::handle_ipprog(artnet_ipprog_t *packet)
{
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
  Serial.println(packet->ProgPortLo); /*
  settings->NetConf.ip_Local[0] = packet->ProgIpHi;
  settings->NetConf.ip_Local[1] = packet->ProgIp2;
  settings->NetConf.ip_Local[2] = packet->ProgIp1;
  settings->NetConf.ip_Local[3] = packet->ProgIpLo;
  settings->NetConf.subnet_Mask[0] = packet->ProgSmHi;
  settings->NetConf.subnet_Mask[1] = packet->ProgSm2;
  settings->NetConf.subnet_Mask[2] = packet->ProgSm1;
  settings->NetConf.subnet_Mask[3] = packet->ProgSmLo;
  //TODO: CHANGE PORT
  fill_art_node(&ArtNode, settings);
  fill_art_poll_reply(&ArtPollReply, &ArtNode);
  fill_art_ipprog_reply(&ArtIpprogReply, &ArtNode);
  send_reply(UNICAST, (uint8_t *)&ArtIpprogReply, sizeof(ArtIpprogReply), settings);
*/
    if (artnetIpProgCallback != NULL) {
        artnetIpProgCallback(packet);
    }
}


int Artnet::handle_address(artnet_address_t *packet) //not implemented yet
{
  send_reply(UNICAST, (uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
}

void Artnet::send_reply(uint8_t mode_broadcast, uint8_t *packet, uint16_t size)
{
  if (mode_broadcast == 1) { // send broadcast packet
    udp.beginPacket();
    //udp.beginPacket(settings->NetConf.ip_Broadcast, 6454);
    udp.write(packet, size); //Bad length Packet
    //udp.write(packet, 281);
    udp.endPacket();
  }
  else {// send unicast packet to controller
    //Udp.beginPacket(Remote_IP, ArtNode.remotePort);
    //udp.RemoteIP
    //udp.beginPacket(Remote_IP, 6454);
    udp.beginPacket();
    udp.write(packet, size);
    //udp.write(packet, 281);
    udp.endPacket();
  }
}

void Artnet::fill_art_node(artnet_node_t *node)
{
  //fill to 0's
  memset(node, 0, sizeof(node));

  //fill data
  //memcpy(node->mac, netConf.mac, 6);                   // the mac address of node
  //memcpy(node->localIp, netConf.ip_local, 4);           // the IP address of node
  //memcpy(node->subnetMask, netConf.mask_subnet, 4);     // network mask (art-net use 'A' network type)

  sprintf((char *)node->id, "Art-Net\0"); // *** don't change never ***
  sprintf((char *)node->shortname, shortName);
  sprintf((char *)node->longname, longName);
  sprintf((char *)node->nodereport, report);
  

  //memset(node->porttypes, 0x80, ARTNET_MAX_PORTS);
  //memset(node->goodinput, 0x08, ARTNET_MAX_PORTS);
  node->numbports = 8;

  memset(node->porttypes, 0x45, 4);
  memset(node->goodinput, 0x80, 4);
  memset (node->goodoutput, 0x00, 4);


  node->subH = 0x00;        // high byte of the Node Subnet Address (This field is currently unused and set to zero. It is
  // provided to allow future expansion.) (art-net III)
  node->sub = 0x00;        // low byte of the Node Subnet Address

  // **************************** art-net address of universes **********************************
  // not implemented yet
  node->swin[0] = 0x00;        // This array defines the 8 bit Universe address of the available input channels.
  node->swin[1] = 0x01;        // values from 0x00 to 0xFF
  node->swin[2] = 0x02;
  node->swin[3] = 0x03;
  //node->swin[0] = 0x08;        // This array defines the 8 bit Universe address of the available input channels.
  //node->swin[1] = 0x09;        // values from 0x00 to 0xFF
  //node->swin[2] = 0x0A;
  //node->swin[3] = 0x0B;

  node->swout[0] = 0x00;        // This array defines the 8 bit Universe address of the available output channels.
  node->swout[1] = 0x01;        // values from 0x00 to 0xFF
  node->swout[2] = 0x02;
  node->swout[3] = 0x03;
  //node->swout[4] = 0x04;

  node->goodoutput[0] = 0x80;
  node->goodoutput[1] = 0x80;
  node->goodoutput[2] = 0x80;
  node->goodoutput[3] = 0x80;

  node->etsaman[0] = 0;        // The ESTA manufacturer code.
  node->etsaman[1] = 0;        // The ESTA manufacturer code.
  node->localPort = 0x1936;   // artnet UDP port is by default 6454 (0x1936)
  node->verH = 0;        // high byte of Node firmware revision number.
  node->ver = 2;        // low byte of Node firmware revision number.
  node->ProVerH = 0;        // high byte of the Art-Net protocol revision number.
  node->ProVer = 14;       // low byte of the Art-Net protocol revision number.
  node->oemH = 0;        // high byte of the oem value.
  node->oem = 0xFF;     // low byte of the oem value. (0x00FF = developer code)
  node->ubea = 0;        // This field contains the firmware version of the User Bios Extension Area (UBEA). 0 if not used
  node->status = 0x08;
  node->swvideo = 0;
  node->swmacro = 0;
  node->swremote = 0;
  node->style = 0;        // StNode style - A DMX to/from Art-Net device
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

  sprintf((char *)poll_reply->nodereport, "%i LED output universes active.\0", 8);

  poll_reply->opCode = 0x2100;  // ARTNET_REPLY
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
  memset (ipprog_reply, 0, sizeof(ipprog_reply));

  //copy data from node
  memcpy (ipprog_reply->id, node->id, sizeof(ipprog_reply->id));

  ipprog_reply->ProgIpHi  = node->localIp[0];
  ipprog_reply->ProgIp2   = node->localIp[1];
  ipprog_reply->ProgIp1   = node->localIp[2];
  ipprog_reply->ProgIpLo  = node->localIp[3];

  ipprog_reply->ProgSmHi  = node->subnetMask[0];
  ipprog_reply->ProgSm2   = node->subnetMask[1];
  ipprog_reply->ProgSm1   = node->subnetMask[2];
  ipprog_reply->ProgSmLo  = node->subnetMask[3];

  ipprog_reply->OpCode        = 0xF900; //ARTNET_IPREPLY
  ipprog_reply->ProVerH       = node->ProVerH;
  ipprog_reply->ProVer        = node->ProVer;
  ipprog_reply->ProgPortHi    = node->localPort >> 8;
  ipprog_reply->ProgPortLo    = (node->localPort & 0xFF);
}

void Artnet::fill_art_dmx(artnet_dmx_t *art_dmx){
    sprintf((char *)art_dmx->id, "Art-Net\0");
    art_dmx->opCode = 0x5000;   //ArtDmx package
    art_dmx->verH = 0;        // high byte of Node firmware revision number.
    art_dmx->ver = 2;        // low byte of Node firmware revision number.
    art_dmx->sequence = 0x00; //sequence off
    art_dmx->physical = 0x00; //TODO
    art_dmx->universe = 0x00;
    art_dmx->lengthHi = 0x00;
    art_dmx->length = 0x00;
    art_dmx->data[ARTNET_DMX_LENGTH];
}


void Artnet::printPacket(uint8_t *packet) {
  for (int i = 0; i < sizeof(artnet_reply_t); i++) { //TODO FIX SIZEOF
    Serial.print((char)packet[i]);
    Serial.print('.');
  }
  Serial.println();

}

void Artnet::printPacketByte(uint8_t *packet) {
  for (int i = 0; i < sizeof(artnet_reply_t); i++) { //TODO FIX SIZEOF
    Serial.print(packet[i]);
    Serial.print('.');
  }
  Serial.println();

}
