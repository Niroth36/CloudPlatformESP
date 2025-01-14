#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "minio_config.h"

#define DHTPIN D2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200, 60000);

unsigned long previousMillis = 0;
const unsigned long interval = 60000; // 1 minute

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

        String jsonPayload = "{\"temperature\": " + String(temperature) +
                             ", \"humidity\": " + String(humidity) +
                             ", \"timestamp\": \"" + timestamp + "\"}";

        sendToMinIO(jsonPayload);
    }
}

void sendToMinIO(String jsonPayload) {
    HTTPClient http;
    String url = "http://" MINIO_SERVER "/" MINIO_BUCKET "/sensor_data.json";

    // Basic Authentication header
    String authHeader = "Basic " + base64::encode(String(MINIO_ACCESS_KEY) + ":" + MINIO_SECRET_KEY);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", authHeader);

    int httpResponseCode = http.PUT(jsonPayload);  // Use PUT to upload JSON data
    if (httpResponseCode > 0) {
        Serial.println("Data sent to MinIO successfully");
        Serial.println("Response: " + http.getString());
    } else {
        Serial.print("Error sending data to MinIO: ");
        Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
}
