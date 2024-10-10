#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Định nghĩa các chân cho ESP32
#define buzzerPin 5
#define RX1_PIN 16 // RX cho giao tiếp UART đầu tiên
#define TX1_PIN 17 // TX cho giao tiếp UART đầu tiên
#define RX2_PIN 4  // RX cho giao tiếp UART thứ hai
#define TX2_PIN 2  // TX cho giao tiếp UART thứ hai

LiquidCrystal_I2C lcd(0x27, 20, 4); 
HardwareSerial mySerial(1); 
HardwareSerial myHardwareSerial(2); 
DFRobotDFPlayerMini myDFPlayer;

unsigned long previousMillis = 0;
volatile bool dataAvailable = false;
volatile bool flag = false;
String count = "";
String sharedCount = ""; 
String dataa = "";
String uid = "";
SemaphoreHandle_t countMutex;
SemaphoreHandle_t lcdMutex;
unsigned long lastInterruptTime = 0; 
const unsigned long debounceDelay = 200; 

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
  myHardwareSerial.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  if (!myDFPlayer.begin(myHardwareSerial)) {
    while (true); 
  }

  myDFPlayer.volume(30);
  lcd.init();
  lcd.backlight();

  pinMode(buzzerPin, OUTPUT);

  
  countMutex = xSemaphoreCreateMutex();
  lcdMutex = xSemaphoreCreateMutex();


  xTaskCreatePinnedToCore(
    TaskSerial,     
    "TaskSerial",  
    10000,          
    NULL,           
    1,              
    NULL,           
    0);            

  xTaskCreatePinnedToCore(
    TaskMySerial,   
    "TaskMySerial", 
    10000,          
    NULL,           
    1,             
    NULL,           
    1);             
}

void TaskSerial(void *pvParameters) {
  while (true) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 8000) {
      previousMillis = currentMillis;
      Serial.print('a');
      delay(10); 
      if (Serial.available()) {
        xSemaphoreTake(countMutex, portMAX_DELAY); 
        count = Serial.readStringUntil('\n');
        sharedCount = count; 
        xSemaphoreGive(countMutex); 
      }
    }
    delay(10); /
  }
}

void TaskMySerial(void *pvParameters) {
  while (true) {
    if (mySerial.available() > 0) {
      dataa = mySerial.readStringUntil('\n'); 
      flag = true;
      dataa.trim();
      delay(10);

     
      if (dataa.endsWith("T")) {
        xSemaphoreTake(lcdMutex, portMAX_DELAY); 
        lcd.setCursor(0, 2);
        lcd.print("  THE HOP LE        ");
        myDFPlayer.play(2);
        xSemaphoreGive(lcdMutex); 
        flag = false;
        delay(1000);
      } else if (dataa.endsWith("F")) {
        xSemaphoreTake(lcdMutex, portMAX_DELAY); 
        lcd.setCursor(0, 2);
        lcd.print("  THE KHONG HOP LE  ");
        myDFPlayer.play(1);
        xSemaphoreGive(lcdMutex); 
        flag = false;
        delay(1000);
      } else if (dataa.endsWith("C")) {
        xSemaphoreTake(countMutex, portMAX_DELAY); 
        mySerial.print(sharedCount); 
        xSemaphoreGive(countMutex); 
      } else {
        uid = dataa;
        xSemaphoreTake(lcdMutex, portMAX_DELAY); 
        lcd.setCursor(0, 0);
        lcd.print("UID: ");
        lcd.setCursor(5, 0);
        lcd.print(uid);
        xSemaphoreGive(lcdMutex); 
        digitalWrite(buzzerPin, HIGH);
        delay(100);
        digitalWrite(buzzerPin, LOW);
        delay(100);
      }
    }

    if (flag == false) {
      xSemaphoreTake(lcdMutex, portMAX_DELAY); 
      lcd.setCursor(0, 0);
      lcd.print("   Moi quet the     ");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      xSemaphoreGive(lcdMutex); // 
      delay(1000); 
    }
    delay(10); 
  }
}

void loop() {
 
}
