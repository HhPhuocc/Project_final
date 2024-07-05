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

LiquidCrystal_I2C lcd(0x27, 20, 4); // Địa chỉ I2C của màn hình LCD và kích thước là 20x4
HardwareSerial mySerial(1); // Sử dụng UART1
HardwareSerial myHardwareSerial(2); // Sử dụng UART2
DFRobotDFPlayerMini myDFPlayer;

unsigned long previousMillis = 0;
volatile bool dataAvailable = false;
volatile bool flag = false;
String count = "";
String sharedCount = ""; // Biến trung gian để chia sẻ giữa các task
String dataa = "";
String uid = ""; // Biến lưu trữ UID
SemaphoreHandle_t countMutex;
SemaphoreHandle_t lcdMutex;
unsigned long lastInterruptTime = 0; // Biến để lưu thời gian ngắt cuối cùng
const unsigned long debounceDelay = 200; // Thời gian debounce (200ms)

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
  myHardwareSerial.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  if (!myDFPlayer.begin(myHardwareSerial)) {
    while (true); // Dừng lại nếu không kết nối được
  }

  myDFPlayer.volume(30);
  lcd.init();
  lcd.backlight();

  pinMode(buzzerPin, OUTPUT);

  // Tạo mutex cho biến count và lcd
  countMutex = xSemaphoreCreateMutex();
  lcdMutex = xSemaphoreCreateMutex();

  // Tạo các task và gán chúng cho các core
  xTaskCreatePinnedToCore(
    TaskSerial,     // Function to be called
    "TaskSerial",   // Name of the task
    10000,          // Stack size (bytes)
    NULL,           // Parameter to pass
    1,              // Task priority
    NULL,           // Task handle
    0);             // Core to run the task

  xTaskCreatePinnedToCore(
    TaskMySerial,   // Function to be called
    "TaskMySerial", // Name of the task
    10000,          // Stack size (bytes)
    NULL,           // Parameter to pass
    1,              // Task priority
    NULL,           // Task handle
    1);             // Core to run the task
}

void TaskSerial(void *pvParameters) {
  while (true) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 8000) {
      previousMillis = currentMillis;
      Serial.print('a');
      delay(10); // Cho một chút thời gian để dữ liệu được gửi đi
      if (Serial.available()) {
        xSemaphoreTake(countMutex, portMAX_DELAY); // Lấy mutex trước khi truy cập biến count
        count = Serial.readStringUntil('\n');
        sharedCount = count; // Gán giá trị của count cho sharedCount
        xSemaphoreGive(countMutex); // Trả mutex sau khi truy cập biến count
      }
    }
    delay(10); // Small delay to yield
  }
}

void TaskMySerial(void *pvParameters) {
  while (true) {
    if (mySerial.available() > 0) {
      dataa = mySerial.readStringUntil('\n'); // Đọc dữ liệu từ UART đến ký tự kết thúc dòng (\n)
      flag = true;
      dataa.trim();
      delay(10);

      // Kiểm tra ký tự cuối cùng của data
      if (dataa.endsWith("T")) {
        xSemaphoreTake(lcdMutex, portMAX_DELAY); // Lấy mutex trước khi truy cập LCD
        lcd.setCursor(0, 2);
        lcd.print("  THE HOP LE        ");
        myDFPlayer.play(2);
        xSemaphoreGive(lcdMutex); // Trả mutex sau khi truy cập LCD
        flag = false;
        delay(1000);
      } else if (dataa.endsWith("F")) {
        xSemaphoreTake(lcdMutex, portMAX_DELAY); // Lấy mutex trước khi truy cập LCD
        lcd.setCursor(0, 2);
        lcd.print("  THE KHONG HOP LE  ");
        myDFPlayer.play(1);
        xSemaphoreGive(lcdMutex); // Trả mutex sau khi truy cập LCD
        flag = false;
        delay(1000);
      } else if (dataa.endsWith("C")) {
        xSemaphoreTake(countMutex, portMAX_DELAY); // Lấy mutex trước khi truy cập biến sharedCount
        mySerial.print(sharedCount); // Sử dụng biến trung gian sharedCount
        xSemaphoreGive(countMutex); // Trả mutex sau khi truy cập biến sharedCount
      } else {
        // Giả sử đây là UID
        uid = dataa;
        xSemaphoreTake(lcdMutex, portMAX_DELAY); // Lấy mutex trước khi truy cập LCD
        lcd.setCursor(0, 0);
        lcd.print("UID: ");
        lcd.setCursor(5, 0);
        lcd.print(uid);
        xSemaphoreGive(lcdMutex); // Trả mutex sau khi truy cập LCD
        digitalWrite(buzzerPin, HIGH);
        delay(100);
        digitalWrite(buzzerPin, LOW);
        delay(100);
      }
    }

    if (flag == false) {
      xSemaphoreTake(lcdMutex, portMAX_DELAY); // Lấy mutex trước khi truy cập LCD
      lcd.setCursor(0, 0);
      lcd.print("   Moi quet the     ");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      xSemaphoreGive(lcdMutex); // Trả mutex sau khi truy cập LCD
      delay(1000); // Thêm độ trễ để tránh nhấp nháy
    }
    delay(10); // Small delay to yield
  }
}

void loop() {
  // Empty loop since we're using tasks
}
