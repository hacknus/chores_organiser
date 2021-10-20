#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "MD_Parola.h"
#include "MD_MAX72xx.h"
#include <WiFiUdp.h>
#include "NTPClient.h"
#include <WebSocketsClient.h>

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
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const char *ssid = "Martin Router King";
const char *password = "ihaveastream5%";

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {


    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
        {
            Serial.print("[WSc] Connected to url: ");
            Serial.println((char *)payload);
            // send message to server when Connected
            webSocket.sendTXT("Connected");
        }
            break;
        case WStype_TEXT:
            Serial.print("[WSc] get text: ");
            Serial.println((char *)payload);
            // send message to server
            // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            Serial.print("[WSc] get binary length: ");
            Serial.println(length);
            // hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
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
    //WiFi.mode(WIFI_STA);
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

    webSocket.begin("192.168.178.1", 13254);
    webSocket.onEvent(webSocketEvent);

}

void loop() {

    webSocket.loop();

    if (digitalRead(D2) == HIGH) {
        linus_done = 1;
        delay(100);
    }

    if (digitalRead(D1) == HIGH) {
        johannes_done = 1;
        delay(100);
    }

    buf = "gate closed";
    str_len = buf.length() + 1;
    buf.toCharArray(newMessage2, str_len);
    newMessageAvailable2 = true;

    buf = "gate closed";
    str_len = buf.length() + 1;
    buf.toCharArray(newMessage, str_len);
    newMessageAvailable = true;

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

    server.handleClient();
}
