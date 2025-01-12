#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// WiFi credentials
const char* ssid = "AxilleasRooter";
const char* password = "dekaeuro";

// Firebase configuration
#include "firebase_config.h"

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

#define DHTPIN D2         // DHT sensor pin
#define DHTTYPE DHT11     // DHT sensor type
#define GREEN_LED_PIN 14  // D5 for Green LED
#define RED_LED_PIN 12    // D6 for Red LED

DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7200;
const unsigned long interval = 60000; // 1-minute interval
unsigned long previousMillis = 0;
unsigned long previousEpochTime = 0;

FirebaseJson json;

// NTP Servers to check
const char* ntpServers[] = {
  "0.pool.ntp.org",
  "1.pool.ntp.org",
  "2.pool.ntp.org",
  "3.pool.ntp.org"
};
int currentServerIndex = 0; // Keep track of the current NTP server being used
const int maxRetries = 5; // Max retries for each server

// NTP Client object
NTPClient* timeClient;

void setup() {
    Serial.begin(9600);
    
    dht.begin();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Configure LED pins as outputs
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);

    // Initialize NTP Client with the first server
    timeClient = new NTPClient(ntpUDP, ntpServers[currentServerIndex], utcOffsetInSeconds, interval);
    timeClient->begin();

    // Attempt to sync time from NTP
    attemptToGetTime();

    // Firebase authentication and configuration
    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_HOST;
    auth.user.email = FIREBASE_USER_EMAIL;
    auth.user.password = FIREBASE_USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.println("Firebase initialized successfully");
}

// Function to attempt getting time from NTP server with retries
void attemptToGetTime() {
    int retries = 0;

    // Attempt to sync NTP time with multiple retries
    while (retries < maxRetries) {
        if (timeClient->update()) {
            Serial.println("NTP Time synced successfully");

            // Save the synced time to previousEpochTime
            previousEpochTime = timeClient->getEpochTime();
            setTime(previousEpochTime); // Update system time
            return; // Exit the function if successful
        } else {
            Serial.println("NTP update failed, retrying...");
            retries++;
            delay(1000); // Wait before retrying
        }
    }
    
    // If still failing after max retries, try the next server
    Serial.println("Switching to next NTP server...");
    currentServerIndex = (currentServerIndex + 1) % (sizeof(ntpServers) / sizeof(ntpServers[0]));
    
    // Recreate the NTPClient object with the new server
    timeClient->end();
    timeClient = new NTPClient(ntpUDP, ntpServers[currentServerIndex], utcOffsetInSeconds, interval);
    timeClient->begin();

    // Check if the NTP time is still not set after trying all servers
    if (previousEpochTime == 0) {
        Serial.println("Failed to sync time after multiple attempts, using fallback time.");
        previousEpochTime = millis() / 1000; // Set a fallback timestamp (current time since boot)
        setTime(previousEpochTime);  // Set fallback time to the system clock
    }

    attemptToGetTime();  // Recursively attempt to get time from the next server
}


void loop() {
    // Ensure WiFi remains connected
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
        }
        Serial.println("Reconnected to WiFi");
    }

    // Reinitialize Firebase if needed
    if (!Firebase.ready()) {
        Serial.println("Firebase not ready. Reinitializing...");
        Firebase.begin(&config, &auth);
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // Read temperature and humidity values
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        // LED Control Logic
        if (humidity >= 50 && humidity <= 60 && temperature >= 20 && temperature <= 22) {
            digitalWrite(GREEN_LED_PIN, HIGH);
            digitalWrite(RED_LED_PIN, LOW);
        } else {
            digitalWrite(GREEN_LED_PIN, LOW);
            digitalWrite(RED_LED_PIN, HIGH);
        }

        // Sync time
        if (timeClient->update()) {
            unsigned long currentEpochTime = timeClient->getEpochTime();
            setTime(currentEpochTime);  // Update system time
            previousEpochTime = currentEpochTime;
        } else {
            Serial.println("NTP time update failed, using previous time + 1 minute");
            previousEpochTime += 60;  // Add 1 minute (60 seconds) to the previous epoch time
            setTime(previousEpochTime);  // Update system time with the adjusted time
        }

        // Prepare the timestamp string
        String timestamp = String(year()) + "-" +
                           (month() < 10 ? "0" : "") + String(month()) + "-" +
                           (day() < 10 ? "0" : "") + String(day()) + "T" +
                           (hour() < 10 ? "0" : "") + String(hour()) + ":" +
                           (minute() < 10 ? "0" : "") + String(minute()) + ":" +
                           (second() < 10 ? "0" : "") + String(second()) + "Z";

        // Send data to Firebase
        json.clear();
        json.set("temperature", temperature);
        json.set("humidity", humidity);
        json.set("timestamp", timestamp);

        String path = "/sensor_data";

        if (Firebase.pushJSON(firebaseData, path, json)) {
            Serial.println("Data sent to Firebase");
        } else {
            Serial.print("Error sending data: ");
            Serial.println(firebaseData.errorReason());
        }
    }
}
