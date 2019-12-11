#include <FastLED.h>

FASTLED_USING_NAMESPACE
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Artnet.h>
WiFiUDP udp;

#define DATA_PIN1   3
#define DATA_PIN2   4
#define DATA_PIN3   5
#define DATA_PIN4   6
//#define CLK_PIN   4

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_STRIPS 4
#define NUM_LEDS_PER_STRIP 12
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

#define BRIGHTNESS          70
//#define FRAMES_PER_SECOND  60

const char* ssid     = "ssid";
const char* password = "pswd";

Artnet artnet(udp);

  uint8_t ip_local[4] = {192, 168, 0, 31};     // the IP address of node
  uint8_t mask_subnet[4] = {255, 255, 255, 0}; // network mask (art-net use 'A' network type)
  uint8_t ip_gateway[4] = {192, 168, 0, 1}; 
  uint8_t factory_dns[4] = {192, 168, 0, 1}; 

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  //Set Manual IP!
  WiFi.config(ip_local, ip_gateway, mask_subnet, factory_dns);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  FastLED.addLeds<LED_TYPE,DATA_PIN1,COLOR_ORDER>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE,DATA_PIN2,COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE,DATA_PIN3,COLOR_ORDER>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE,DATA_PIN4,COLOR_ORDER>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(BRIGHTNESS);

  uint16_t in_map[4] = {0,1,2,3};
  uint16_t out_map[1] = {9}; //TODO
  artnet.init(mac, ip_local, mask_subnet);
  artnet.setArtnetRecvCallback(recvCallback);
  // input map, input map size, output map, output map size
  artnet.begin(in_map, 4, out_map, 1);  
}

int packetCounter = 0;

void loop() {
  EVERY_N_MILLISECONDS( 30 ) { FastLED.show();  }
  artnet.handle();
  EVERY_N_MILLISECONDS( 1000 ) { 
    char *report = "";
    sprintf(report, "Recieved packet: %d per second", packetCounter);
    artnet.setReport(report);
    packetCounter = 0;
  }
}

void recvCallback(uint8_t *data, uint16_t port){
  packetCounter++;
  switch (port) {
    case 0: {
      uint16_t pointer=0;
      for (int i=0; i<NUM_LEDS_PER_STRIP;i++){
        leds[0*NUM_LEDS_PER_STRIP+i] = CRGB(data[pointer++],data[pointer++],data[pointer++]);
      }
    } break;
    case 1: {
      uint16_t pointer=0;
      for (int i=0; i<NUM_LEDS_PER_STRIP;i++){
        leds[1*NUM_LEDS_PER_STRIP+i] = CRGB(data[pointer++],data[pointer++],data[pointer++]);
      }
    } break;
    case 2: {
      uint16_t pointer=0;
      for (int i=0; i<NUM_LEDS_PER_STRIP;i++){
        leds[2*NUM_LEDS_PER_STRIP+i] = CRGB(data[pointer++],data[pointer++],data[pointer++]);
      }
    } break;
    case 3: {
      uint16_t pointer=0;
      for (int i=0; i<NUM_LEDS_PER_STRIP;i++){
        leds[3*NUM_LEDS_PER_STRIP+i] = CRGB(data[pointer++],data[pointer++],data[pointer++]);
      }
    } break;
    default: break;
  }
}
