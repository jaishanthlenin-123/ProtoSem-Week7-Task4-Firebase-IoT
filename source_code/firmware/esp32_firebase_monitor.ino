/*
 * Task 4: ESP32 Firebase IoT Multi-Sensor Monitoring & Control
 * Hardware:
 *   - DHT11 Temperature & Humidity Sensor -> GPIO 4
 *   - LDR Analog Light Sensor -> GPIO 34 (ADC1)
 *   - Relay Module / AC Bulb -> GPIO 5
 * Library: Firebase ESP Client by Mobizt
 */

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi Credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Firebase Project Credentials
#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL" // e.g. https://[PROJECT_ID]-default-rtdb.firebaseio.com/

// Exact Pin Configuration (Matches Pin Table and Schematic)
#define DHTPIN   4       // Digital One-Wire bus for DHT11
#define DHTTYPE  DHT11   // Sensor Model
#define LDRPIN   34      // ADC1 Channel 6 for Analog Light Voltage
#define BULBPIN  5       // Digital Output driving Relay Module

DHT dht(DHTPIN, DHTTYPE);

// Firebase Core Objects
FirebaseData fbdo;
FirebaseData streamData;
FirebaseAuth auth;
FirebaseConfig config;

// Timing variables for non-blocking sensor acquisition
unsigned long lastSensorPublish = 0;
const unsigned long publishInterval = 3000; // Publish every 3 seconds

// Stream callback: fires asynchronously when Firebase /control/bulb changes
void streamCallback(FirebaseStream data) {
  Serial.printf("[FIREBASE STREAM] Path: %s | Value: %s\n", data.dataPath().c_str(), data.stringData().c_str());
  if (data.dataType() == "boolean") {
    bool bulbCmd = data.boolData();
    digitalWrite(BULBPIN, bulbCmd ? HIGH : LOW);
    Serial.printf("[HARDWARE] Relay state updated -> GPIO 5 = %s\n", bulbCmd ? "HIGH (ON)" : "LOW (OFF)");
  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("[FIREBASE STREAM] Stream timeout, reconnecting...");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Configure Pin Modes
  pinMode(BULBPIN, OUTPUT);
  pinMode(LDRPIN, INPUT);
  digitalWrite(BULBPIN, LOW); // Start with bulb OFF

  dht.begin();
  Serial.println("\n=== Task 4: Firebase IoT Multi-Sensor Monitoring ===");

  // Connect to Local Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());

  // Configure Firebase Client
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up anonymously or use email credentials
  Firebase.signUp(&config, &auth, "", "");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Establish real-time WebSocket listener on /control/bulb
  if (!Firebase.RTDB.beginStream(&streamData, "/control/bulb")) {
    Serial.printf("[FIREBASE] Stream begin failed: %s\n", streamData.errorReason().c_str());
  }
  Firebase.RTDB.setStreamCallback(&streamData, streamCallback, streamTimeoutCallback);
  Serial.println("[FIREBASE] Realtime listener active on /control/bulb");
}

void loop() {
  // Non-blocking telemetry loop
  if (Firebase.ready() && (millis() - lastSensorPublish >= publishInterval || lastSensorPublish == 0)) {
    lastSensorPublish = millis();

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int ldrRaw = analogRead(LDRPIN);

    // Validate sensor readings
    if (isnan(temp) || isnan(hum)) {
      Serial.println("[ERROR] Failed to read from DHT11 sensor!");
      return;
    }

    Serial.printf("[TELEMETRY] Temp: %.1f C | Hum: %.1f %% | LDR: %d\n", temp, hum, ldrRaw);

    // Update Realtime Database Nodes
    Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", temp);
    Firebase.RTDB.setFloat(&fbdo, "/sensors/humidity", hum);
    Firebase.RTDB.setInt(&fbdo, "/sensors/ldr", ldrRaw);
  }
}