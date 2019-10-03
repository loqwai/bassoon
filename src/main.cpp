#include "heltec.h"
#include <set>
#include <string>
#include <stdio.h>
using namespace std;

#define BAND 868E6 //you can se\t band here directly,e.g. 868E6,915E6

uint8_t id[6];
String lastMsgSent;
String lastMsgRcvd;
unsigned long lastSendTime = 0;
int interval = 1000;

set<String> myFriends;

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
  esp_efuse_mac_get_default(id);
}

String addressToString(uint8_t* addr) {
  char str[18] = {}; 
  sprintf(str, "%X:%X:%X:%X:%X:%X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
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
  Heltec.display->drawStringMaxWidth(0, 0, 128, "id: " + addressToString(id));

  String friendsBrag = "";

  for (auto f : myFriends) {
    friendsBrag += ", " + f;
  }

  Heltec.display->drawStringMaxWidth(0, 10, 128, "friends: " + friendsBrag);
  Heltec.display->display();
}

void sendMessage(String outgoing) {
  Serial.println("Imma bout to yell about myself");
  LoRa.beginPacket();
  LoRa.print(addressToString(id));
  LoRa.endPacket();
}

void loraStuff() {
  if (millis() - lastSendTime < interval) return;
  auto beginSend = millis();
  sendMessage(String((char*) id));
  Serial.println("Sending " + addressToString(id));
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

  byte incomingLength = LoRa.read();
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    printScreen("error: message length does not match length\n" + incoming);
    return;
  }
  Serial.println("Heard from a friend: " + incoming);
  myFriends.insert(incoming);
}

