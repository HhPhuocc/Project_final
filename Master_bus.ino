/*
  Master Lora Node
  05/05/2024 - vnq - i

*/      
#include <Arduino.h>  
#include <WiFi.h>               //using the ESP32
#include <Firebase_ESP_Client.h>   
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
//----------------------------------FIREBASE 
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
//----------------------------------
#define NSS 4
#define RST 5
#define DI0 2

#define WIFI_SSID "tak" //<<< WIFI DEFINE
#define WIFI_PASSWORD "impossible111"


#define API_KEY "AIzaSyDPyQdcKG217hzWMXTL4WseOOyPD9ovMPA" //<<FIBASE DEFINE
#define DATABASE_URL "https://da33-44ee4-default-rtdb.firebaseio.com/" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

byte MasterNode = 0xFF;
byte Node1 = 0xBB;
byte Node2 = 0xCC;
char serialData;
String SenderNode = "";
String outgoing;              // outgoing message/ tin nhan gui di

//byte msgCount = 0;            // count of outgoing messages - dem
String incoming = "";  //-----du lieu den
volatile bool flag = false; 
volatile bool flag_UID = false; 
// Tracks the time since last event fired
unsigned long previousMillis = 0;
unsigned long int previoussecs = 0;
unsigned long int currentsecs = 0;
unsigned long currentMillis = 0;
int interval = 1 ; // updated every 1 second
int Secs = 0;

bool signupOK = false;                     //since we are doing an anonymous sign in 

// int temperature;
// int humidity;
// int soilmoisturepercent;
// int soilMoistureValue;

//String lat;
//String lon;
// double lat = 0;
// double lon = 0;
void sendMessage(String outgoing, byte MasterNode, byte otherNode) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(otherNode);              // add destination address/ dia chi đích
  LoRa.write(MasterNode);            // add sender address
  //LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  //msgCount++;
}

void Plz_run(){
  int commaIndex = incoming.indexOf(',');
  String latTR = incoming.substring(0, commaIndex);
  String lonTR = incoming.substring(commaIndex + 1);
  double lat = latTR.toDouble();
  double lon = lonTR.toDouble();
//  Serial.print("Latitude: " );
//  Serial.println(lat, 6);
//  Serial.print("Longitude: " );
//  Serial.println(lon, 6);
  String pathLat1 = "/GPS_Value_1/Latitude";
  String pathLon1 = "/GPS_Value_1/Longitude";
  Firebase.RTDB.setDouble(&fbdo, (pathLat1), lat);
  Firebase.RTDB.setDouble(&fbdo, (pathLon1), lon);

}

void Plz_run1(){
  int commaIndex = incoming.indexOf(',');
  String latTR = incoming.substring(0, commaIndex);
  String lonTR = incoming.substring(commaIndex + 1);
  double lat = latTR.toDouble();
  double lon = lonTR.toDouble();
//  Serial.print("Latitude: " );
//  Serial.println(lat, 6);
//  Serial.print("Longitude: " );
//  Serial.println(lon, 6);
  String pathLat1 = "/GPS_Value_2/Latitude";
  String pathLon1 = "/GPS_Value_2/Longitude";
  Firebase.RTDB.setDouble(&fbdo, (pathLat1), lat);
  Firebase.RTDB.setDouble(&fbdo, (pathLon1), lon);
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address/ dia chi nhận
  byte sender = LoRa.read();            // sender address
  byte incomingLength = LoRa.read();    // incoming msg length

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  //Serial.println(incoming);
  if (incomingLength != incoming.length()) {   // check length for error
    //Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }
  if (recipient != MasterNode) {
    //Serial.println("This message is not for me.");
    return;
  }
  if ( sender == 0XBB ){
    SenderNode = "Node1:";
    //Serial.println("  1 :-----");
    //Serial.println(incoming);
    Plz_run();
    incoming = "";
  }

  if ( sender == 0XCC ){
    SenderNode = "Node2:";
    //Serial.println("  2 :-----");
    //Serial.println(incoming);
    Plz_run1();
    incoming = "";
}
  if (sender == 0xC1) {
    flag = true;
    flag_UID = true;
    String UID = "";
    UID = incoming;
    Serial.println(UID);
    incoming = "";
    // Đợi nhận dữ liệu từ cổng Serial
    while(Serial.available() == 0) {
    
    }
    serialData = Serial.read();
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print(serialData);
    delay(1000);
    
    if ((serialData == '1')&&(flag_UID == true)) {
      String message = "30";
      sendMessage("30", MasterNode, Node2);
      lcd.setCursor(0, 1); 
      lcd.print(message);
      flag_UID = false;
      flag = false;
    } 
    else if ((serialData == '0')&&(flag_UID == true)){
      String message = "40";
      sendMessage("40", MasterNode, Node2);
      lcd.setCursor(0, 1); 
      lcd.print(message);
      flag_UID = false;
      flag = false;
    }
  //byte incomingMsgId = LoRa.read();     // incoming msg ID
  }
  if(sender == 0xC2)
{
    String count = "";
    count = incoming;
    int countInt = count.toInt();  // Chuyển đổi chuỗi count thành số nguyên

    String Count2 = "/GPS_Value_2/Count";
    Firebase.RTDB.setInt(&fbdo, Count2, countInt);  // Gửi giá trị số nguyên lên Firebase
    incoming = "";
}
  if (sender == 0xB1) {
    flag = true;
    flag_UID = true;
    String UID = "";
    UID = incoming;
    Serial.println(UID);
    incoming = "";
    // Đợi nhận dữ liệu từ cổng Serial
    while(Serial.available() == 0) {
    
    }
    serialData = Serial.read();
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print(serialData);
    delay(1000);
    
    if ((serialData == '1')&&(flag_UID == true)) {
      String message = "70";
      sendMessage("70", MasterNode, Node1);
      lcd.setCursor(0, 1); 
      lcd.print(message);
      flag_UID = false;
      flag = false;
    } 
    else if ((serialData == '0')&&(flag_UID == true)){
      String message = "80";
      sendMessage("80", MasterNode, Node1);
      lcd.setCursor(0, 1); 
      lcd.print(message);
      flag_UID = false;
      flag = false;
    }
  }
  if(sender == 0xB2)
  {
      String count = "";
      count = incoming;
      int countInt = count.toInt();  // Chuyển đổi chuỗi count thành số nguyên
  
      String Count1 = "/GPS_Value_1/Count";
      Firebase.RTDB.setInt(&fbdo, Count1, countInt);  // Gửi giá trị số nguyên lên Firebase
      incoming = "";
  }
}
void setup() {
  Serial.begin(115200);    // initialize serial
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   //--------------Wifi 
  lcd.init();           // Khởi tạo màn hình LCD
  lcd.backlight();      // Bật đèn nền
  //Serial.println(F("ESP32 - Firebase"));
  //Serial.println("LoRa Master Node");

  while (WiFi.status() != WL_CONNECTED){
    {
      //Serial.print(".");
    }
  }
  //Serial.println();
  //Serial.print("Connected with IP: ");
  //Serial.println(WiFi.localIP());
  //Serial.println();

  //------------------------LORA SET PINS

  //Serial.println("LoRa Receiver");
  LoRa.setPins(NSS, RST, DI0);///<<<<<<<<<<<<<<<<<<<<<<<<<<
  if (!LoRa.begin(433E6)) {
    //Serial.println("Starting LoRa failed!");
    while (1);
  }

  //----------------------------------------FIREBASE
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    //Serial.println("sign up ok");
    signupOK = true;
  }
  else{
    //Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  //--------------------------------------------------------------------
}
  
void loop() {

  currentMillis = millis();
  currentsecs = currentMillis / 1000;
  if ((unsigned long)(currentsecs - previoussecs) >= interval) {
    Secs = Secs + 1;
    //Serial.println(Secs);
    if ( Secs >= 30 ) ///-------------------------
    {
      Secs = 0;
    }
    if (( Secs == 10 )&&(flag == false))
    {

      String message = "10"; //---- thong bao
      sendMessage(message, MasterNode, Node1);
     
    }

    if (( Secs == 12)&&(flag == false))
    {

      String message = "20";
      sendMessage(message, MasterNode, Node2);
     
    }
    if (( Secs == 20)&&(flag == false))
    {

      String message = "60";
      //Serial.println(message);
      sendMessage(message, MasterNode, Node1);
    }

    if (( Secs == 22)&&(flag == false))
    {

      String message = "50";
      //Serial.println(message);
      sendMessage(message, MasterNode, Node2);
    }

    previoussecs = currentsecs;
  }
  onReceive(LoRa.parsePacket());
  
}



  
//                       oo0oo
//                      o8888888o
//                      88" . "88
//                      (| 😑 |)
//                      0\  =  /0
//                    __/`---'\__
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             _'. .'  /--.--\  `. .'_
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  - \.;`\ _ /`;.`/ - ` : | |
//         \  \ _.   \_ __\ /__ _/   .- /  /
//     =====`-.____.___ \_____/___.-___.-'=====
//                       `=---='
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//            Phật phù hộ, không bao giờ BUG
//     
