#include "heltec.h"
#include <set>

using namespace std;

#define BAND 868E6 //you can se\t band here directly,e.g. 868E6,915E6

uint8_t id[6];
String lastMsgSent;
String lastMsgRcvd;
unsigned long lastSendTime = 0;
int interval = 1000;

set<uint8_t*> myFriends;

void updateLocalAddress();
void setupScreen();
void renderDashboard();
void loraStuff();
void onReceive(int packetSize);

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  getLocalAddress();  
  setupScreen();
  LoRa.onReceive(onReceive);
}

void loop() {
  renderDashboard();
  loraStuff();
}

void updateLocalAddress() {
  uint8_t out[6];
  esp_efuse_mac_get_default(out);
  return out;
}

void setupScreen() {
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->display();
}

void renderDashboard() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawStringMaxWidth(0, 0, 128, "id: " + id);

  String friendsBrag = "";

  for (auto f : myFriends) {
    friendsBrag += ", " + f;
  }

  Heltec.display->drawStringMaxWidth(0, 10, 128, "friends: " + friendsBrag);
  Heltec.display->display();
}

void sendMessage(String outgoing) {
  Serial.println("Imma bout to yell about myself");
  auto beginSend = millis();
  LoRa.beginPacket();
  LoRa.print(id);
  LoRa.endPacket();
  auto elapsed = millis() - beginSend;
  Serial.printf("Sending message took: %i millis", (int)elapsed);
}

void loraStuff()
{
  if (millis() - lastSendTime < interval) return;
  sendMessage(id);
  Serial.println("Sending " + id);
  lastSendTime = millis();            // timestamp the message
  interval = random(2000) + 1000;    // 2-3 seconds
}

void printScreen(String msg) {
  Serial.println(msg);
}

void onReceive(int packetSize) {
  digitalWrite(LED, HIGH);
  if (packetSize == 0) return;

  byte incomingLength = LoRa.read();
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    printScreen("error: message length does not match length\n" + incoming);
    return;
  }
  Serial.println("Heard from a friend: %s" + (char*)incoming);
  myFriends.insert(incoming);
}

