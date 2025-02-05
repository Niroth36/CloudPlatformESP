#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <base64.h> 
#include "minio_config.h"

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200, 60000);

unsigned long previousMillis = 0;
const unsigned long interval = 6000; // 1 minute - 6sec

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

        // Create JSON payload
        StaticJsonDocument<200> doc;
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["timestamp"] = timestamp;
        String jsonPayload;
        serializeJson(doc, jsonPayload);

        sendToMinIO(jsonPayload);
    }
}

void sendToMinIO(String jsonPayload) {
    HTTPClient http;
    // Use the correct MinIO service URL (ensure this is properly set in minio_config.h)
    String url = MINIO_SERVICE "/" MINIO_BUCKET "/sensor_data.json";  // Ensure MINIO_SERVICE is correct

    WiFiClient client;  // Create a WiFiClient instance
    http.begin(client, url);  // Use the updated API

    http.addHeader("Content-Type", "application/json");  // Set content-type to JSON

    // Since authentication is disabled, we no longer need the "Authorization" header
    // Remove the line that adds the authorization header

    int httpResponseCode = http.PUT(jsonPayload);  // Use PUT to upload JSON data
    if (httpResponseCode > 0) {
        Serial.println("Data sent to MinIO successfully");
        Serial.println("Response: " + http.getString());
    } else {
        Serial.print("Error sending data to MinIO: ");
        Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();  // Close the HTTP connection
}



