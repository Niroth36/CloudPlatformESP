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

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7200;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000);

unsigned long previousMillis = 0;
const unsigned long interval = 60000; // 5 minutes

FirebaseJson json;

void setup() {
    Serial.begin(9600);

    dht.begin();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    timeClient.begin();
    while (!timeClient.update()) {
        timeClient.forceUpdate();
    }
    Serial.println("NTP Time synced successfully");

    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_HOST;
    auth.user.email = FIREBASE_USER_EMAIL;
    auth.user.password = FIREBASE_USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.println("Firebase initialized successfully");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
        }
        Serial.println("Reconnected to WiFi");
    }

    if (!Firebase.ready()) {
        Serial.println("Firebase not ready. Reinitializing...");
        Firebase.begin(&config, &auth);
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        timeClient.update();
        setTime(timeClient.getEpochTime());

        String timestamp = String(year()) + "-" +
                           (month() < 10 ? "0" : "") + String(month()) + "-" +
                           (day() < 10 ? "0" : "") + String(day()) + "T" +
                           (hour() < 10 ? "0" : "") + String(hour()) + ":" +
                           (minute() < 10 ? "0" : "") + String(minute()) + ":" +
                           (second() < 10 ? "0" : "") + String(second()) + "Z";

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
