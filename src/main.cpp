#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "MD_Parola.h"
#include "MD_MAX72xx.h"
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define HARDWARE_TYPE2 MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 8
#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D4
#define CS_PIN2    D8

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
MD_Parola P2 = MD_Parola(HARDWARE_TYPE2, CS_PIN2, MAX_DEVICES);

uint8_t frameDelay = 30;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;

// Global message buffers shared by Serial and Scrolling functions
#define  BUF_SIZE  225
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;
char curMessage2[BUF_SIZE];
char newMessage2[BUF_SIZE];
bool newMessageAvailable2 = false;
String buf;
int str_len = 0;
int linus_done = 0;
int johannes_done = 0;
const long utcOffsetInSeconds = 3600;

const char *ssid = "Martin Router King";
const char *password = "ihaveastream5%";

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    char* pch = NULL;

    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED: {
            Serial.printf("[WSc] Connected to url: %s\n", payload);
            // send message to server when Connected
            // webSocket.sendTXT("Connected");
        }
            break;
        case WStype_TEXT:
            Serial.printf("[WSc] get text: %s\n", payload);

            pch = strtok((char*)payload,"\n");
            buf = pch;
            str_len = buf.length() + 1;
            buf.toCharArray(newMessage, str_len);
            newMessageAvailable = true;


            pch = strtok(NULL,"\n");
            buf = pch;
            str_len = buf.length() + 1;
            buf.toCharArray(newMessage2, str_len);
            newMessageAvailable2 = true;
            // send message to server
            // webSocket.sendTXT("message here");
            Serial.printf("new message: %s\n", newMessage);
            Serial.printf("new message2: %s\n", newMessage2);

            break;
        case WStype_BIN:
            Serial.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
        case WStype_PING:
            // pong will be send automatically
            Serial.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            Serial.printf("[WSc] get pong\n");
            break;
    }

}

void setup() {
    Serial.begin(115200);
    delay(10);

    delay(1000);
    Serial.println("booted up!");

    P.begin();
    P.displayScroll(curMessage, PA_LEFT, scrollEffect, frameDelay);
    P2.begin();
    P2.displayScroll(curMessage2, PA_LEFT, scrollEffect, frameDelay);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    WiFi.persistent(false); //workaround to buffer overflow of the wifi lib

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    webSocket.begin("192.168.178.69", 13254);
    webSocket.onEvent(webSocketEvent);
    // try ever 5000 again if connection has failed
    webSocket.setReconnectInterval(5000);
}

void loop() {

    webSocket.loop();
    String message;

    if (digitalRead(D2) == HIGH) {
        DynamicJsonDocument doc(200); // fixed size

        JsonObject root = doc.to<JsonObject>();
        root["client"] = "display";
        root["linus"] = "0";
        root["johannes"] = "1";
        char   buffer[200]; // create temp buffer
        size_t len = serializeJson(root, buffer);  // serialize to buffer
        webSocket.sendTXT(buffer);
        delay(100);
    }

    if (digitalRead(D1) == HIGH) {
        DynamicJsonDocument doc(200); // fixed size

        JsonObject root = doc.to<JsonObject>();
        root["client"] = "display";
        root["linus"] = "1";
        root["johannes"] = "0";
        char   buffer[200]; // create temp buffer
        size_t len = serializeJson(root, buffer);  // serialize to buffer
        webSocket.sendTXT(buffer);
        delay(100);
    }

    if (P2.displayAnimate()) {
        if (newMessageAvailable2) {
            strcpy(curMessage2, newMessage2);
            newMessageAvailable2 = false;
        }
        P2.displayReset();
    }
    if (P.displayAnimate()) {
        if (newMessageAvailable) {
            strcpy(curMessage, newMessage);
            newMessageAvailable = false;
        }
        P.displayReset();
    }

}
