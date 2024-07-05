#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <MFRC522.h>

#define NSS_PIN 10
#define RST_PIN 9
#define RSTT_PIN 7
#define SS_PIN_RFID 8
#define DIO0_PIN 6
#define GPS_BAUDRATE 9600
#define S_RX_PIN 4
#define S_TX_PIN 3

TinyGPSPlus gps;
SoftwareSerial SoftSerial(S_RX_PIN, S_TX_PIN);
MFRC522 mfrc522(SS_PIN_RFID, RSTT_PIN);

byte MasterNode = 0xFF;
byte Node2 = 0xCC;
byte Node_rfid = 0xC1;
byte Node_count = 0xC2;
volatile bool cardPresent = false;
volatile bool flag = false;
unsigned long previousMillis = 0;
volatile bool f1 = false;
String message;
void sendMessage(String outgoing, byte MasterNode, byte Node2) {
  LoRa.beginPacket();
  LoRa.write(MasterNode);
  LoRa.write(Node2);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket();
}

//void handleInterrupt() {
//  cardPresent = true;
//}

void onReceive(int packetSize) {
  if (packetSize == 0) return;
  //Serial.println("2");
  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingLength = LoRa.read();

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    return;
  }

  if (recipient != Node2 && sender != MasterNode) {
    return;
  }

  String Val = incoming;

  if ((Val == "20")&&(flag == false)) {
   
    sendMessage(message, MasterNode, Node2);
    delay(100);
  }
  else if (Val == "30") {
    while (Serial.available() > 0) {
    Serial.read();
  }
    Serial.println("T");
    flag = false;
  }
  else if (Val == "40") {
    while (Serial.available() > 0) {
    Serial.read();
  }
    Serial.println("F");
    flag = false;
  }
  else if ((Val == "50")&&(flag == false)) {
    while (Serial.available() > 0) {
    Serial.read();
  }
    Serial.println("C");
    delay(10);
    unsigned long startTime = millis();
    while (Serial.available() == 0) {
      if (millis() - startTime > 2000) {
        return;
      }
    }
    if (Serial.available()) {
      String count = Serial.readStringUntil('\n');
      sendMessage(count, MasterNode, Node_count);
    }
  } 
}

void readRFID() {
  digitalWrite(SS_PIN_RFID, LOW);
  if ((mfrc522.PICC_IsNewCardPresent()) && (mfrc522.PICC_ReadCardSerial())) {
    flag = true;
    
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content += String(mfrc522.uid.uidByte[i]);
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    Serial.println(content);
    sendMessage(content, MasterNode, Node_rfid);
  }
  digitalWrite(SS_PIN_RFID, HIGH);
}

void setup() {
  Serial.begin(9600);
  SoftSerial.begin(GPS_BAUDRATE);
  SPI.begin();
  
  LoRa.setPins(NSS_PIN, RST_PIN, DIO0_PIN);
  LoRa.begin(433E6);
  mfrc522.PCD_Init();
}

void loop() {
  if (SoftSerial.available()) {
    gps.encode(SoftSerial.read());
  }
  if (gps.location.isUpdated()) {
      message = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
     }
//  
 RFID();
  onReceive(LoRa.parsePacket());

}
void RFID(){
   unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 2500) {
    previousMillis = currentMillis;
     readRFID();
  }
}
