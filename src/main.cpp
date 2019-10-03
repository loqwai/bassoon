#include "heltec.h"
#include <set>
#include <list>
#include <string>
#include <stdio.h>
using namespace std;

#define BAND 868E6 //you can se\t band here directly,e.g. 868E6,915E6

list<uint8_t> id;
String lastMsgSent;
String lastMsgRcvd;
unsigned long lastSendTime = 0;
int interval = 1000;

set<list<uint8_t>> myFriends;

void updateLocalAddress();
void setupScreen();
void renderDashboard();
void loraStuff();
void onReceive(int packetSize);

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  updateLocalAddress();  
  setupScreen();
  LoRa.onReceive(onReceive);
  LoRa.receive();
}

void loop() {
  renderDashboard();
  loraStuff();
}

void updateLocalAddress() {
  uint8_t rawId[6];
  esp_efuse_mac_get_default(rawId);
  
  for (auto c : rawId) {
    id.push_back(c);
  }
}

String addressToString(list<uint8_t> addr) {
  uint8_t chars[addr.size()];

  int i = 0;
  for (auto c : addr) {
    chars[i] = c;
    i++;
  }

  return String((char*) chars);
}

String addressToFormattedString(list<uint8_t> addr) {
  char str[18] = {}; 

  auto chars = addressToString(addr);

  sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", chars[0], chars[1], chars[2], chars[3], chars[4], chars[5]);
  return String(str);
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
  Heltec.display->drawStringMaxWidth(0, 0, 128, "id: " + addressToFormattedString(id));

  String friendsBrag = "";

  for (auto f : myFriends) {
    friendsBrag += " " + addressToFormattedString(f);
  }

  Heltec.display->drawStringMaxWidth(0, 10, 128, "friends: " + friendsBrag);
  Heltec.display->display();
}

void sendMessage(String outgoing) {
  Serial.println("Imma bout to yell about myself");
  LoRa.beginPacket();
  LoRa.print(addressToFormattedString(id));
  LoRa.endPacket();
}

void loraStuff() {
  if (millis() - lastSendTime < interval) return;
  auto beginSend = millis();
  sendMessage(addressToString(id));
  Serial.println("Sending " + addressToFormattedString(id));
  lastSendTime = millis();            // timestamp the message
  auto elapsed = millis() - beginSend;
  Serial.printf("Sending message took: %i millis\n", (int)elapsed);
  interval = elapsed + random(elapsed);
  LoRa.receive();
}

void printScreen(String msg) {
  Serial.println(msg);
}

void onReceive(int packetSize) {
  Serial.printf("onReceive(%i)\n", packetSize);
  digitalWrite(LED, HIGH);
  if (packetSize == 0) return;

  list<uint8_t> incoming = {};

  while (LoRa.available()) {
    incoming.push_back(LoRa.read());
  }
  digitalWrite(LED, LOW);

  Serial.println("Heard from a friend: " + addressToFormattedString(incoming));
  myFriends.insert(incoming);
}
