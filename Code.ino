#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Pin definitions
#define RED_LED_PIN 25
#define GREEN_LED_PIN 26
#define YELLOW_LED_PIN 27
#define BUTTON_PIN 33
#define BUZZER_PIN 5

bool tagScanned = false; // Variable to track if tag has been scanned

// Define your WiFi credentials
const char* ssid = "METINDO-DOJO";
const char* password = "Metindo@Dojo";

// Google Sheets API endpoint
const char* googleSheetsUrl = "https://script.google.com/macros/s/AKfycbxVkL005hXYkldi6R3qIs-PROQiW_PdfyQrGz54IBapYqEqGKyNV2gaS6ddaQry7g2qCg/exec";

void setup() {
  Serial.begin(115200);
  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Check if a new card is present
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("Card UID:");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    
    // Blink the green LED
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay(500);
    digitalWrite(GREEN_LED_PIN, LOW);
    
    // Make a beep sound with the buzzer
    tone(BUZZER_PIN, 1000); // Send 1KHz sound signal
    delay(1000);            // for 1 sec
    noTone(BUZZER_PIN);     // Stop sound
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    
    // Set tag scanned flag to true
    tagScanned = true;
  }
  
  // Check if button is pressed and tag has been scanned
  if (digitalRead(BUTTON_PIN) == LOW && tagScanned) {
    Serial.println("Button pressed after tag scan!");
    // Turn on green LED
    digitalWrite(GREEN_LED_PIN, HIGH);
    // Turn off red LED if previously on
    digitalWrite(RED_LED_PIN, LOW);

    // Send data "1" to Google Sheets
    sendDataToGoogleSheets("1");
    delay(1000); // Avoid multiple submissions due to button bounce
  }
  // If button is pressed before tag is scanned or without a tag
  else if (digitalRead(BUTTON_PIN) == LOW && !tagScanned) {
    Serial.println("Button pressed before tag scan!");
    // Turn on red LED
    digitalWrite(RED_LED_PIN, HIGH);
    // Turn off green LED if previously on
    digitalWrite(GREEN_LED_PIN, LOW);
    // Make a long beep sound with the buzzer
    tone(BUZZER_PIN, 1000); // Send 1KHz sound signal
    delay(2000);            // for 2 sec
    noTone(BUZZER_PIN);     // Stop sound
  }
}

void sendDataToGoogleSheets(String data) {
  HTTPClient http;

  // Add data to the request payload
  String payload = "{\"value\":\"" + data + "\"}";

  // Send HTTP POST request
  http.begin(googleSheetsUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error in HTTP POST request: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
