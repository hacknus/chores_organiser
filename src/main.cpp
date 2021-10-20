#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "MD_Parola.h"
#include "MD_MAX72xx.h"
#include <WiFiUdp.h>
#include "NTPClient.h"

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

const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const char *ssid = "Martin Router King";
const char *password = "ihaveastream5%";

ESP8266WebServer server(80); //Server on port 80

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

}

void loop() {

    if (millis() % 3600000 <= 30000) {
        if (update_time) {
            timeClient.update();
            update_time = false;
        }
    } else {
        update_time = true;
    }

    if (old_day != timeClient.getDay()) {
        for (int i = 0; i < 6; i++) {
            if (Linus[i].name != "gate closed") Linus[i].update();
            if (Jana[i].name != "gate closed") Jana[i].update();
        }
        old_day = timeClient.getDay();
        changed = 1;
        Serial.print("new day: ");
        Serial.println(old_day);
    }


    int j = 0;
    for (int i = 0; i < NUM_CHORES; i++) {
        if ((Jana[i].who == LINUS) and (Jana[i].name != "gate closed")) {
            for (j = 0; j < NUM_CHORES; j++) {
                if (Linus[j].name == "gate closed") break;
            }
            //Serial.print("need to shift ");
            //Serial.println(Jana[i].name);
            changed = 1;
            Linus[j] = Jana[i];
            Jana[i] = EmptyChore;
        }
    }

    j = 0;
    for (int i = 0; i < NUM_CHORES; i++) {
        if ((Linus[i].who == JANA) and (Linus[i].name != "gate closed")) {
            for (j = 0; j < NUM_CHORES; j++) {
                if (Jana[j].name == "gate closed") break;
            }
            //Serial.print("need to shift ");
            //Serial.println(Linus[i].name);
            changed = 1;
            Jana[j] = Linus[i];
            Linus[i] = EmptyChore;
        }
    }

    if (changed != 0) {
        sort_chores(Jana);
        sort_chores(Linus);

        SPIFFS.remove("/countdowns.csv");  //Lösche Datei
        myfile = SPIFFS.open("/countdowns.csv", "w");  // Öffne Datei um Daten zu überschreiben ! (a - append)
        output = "";
        for (int i = 0; i < NUM_CHORES; i++) {
            for (int j = 0; j < NUM_CHORES; j++) {
                if ((Linus[j].name == chores[i].name) and (Linus[j].name != "gate closed")) {
                    output += Linus[j].countdown;
                    output += ",";
                } else if ((Jana[j].name == chores[i].name) and (Jana[j].name != "gate closed")) {
                    output += Jana[j].countdown;
                    output += ",";
                }
            }
        }
        Serial.print("writing to countdowns-file:");
        Serial.println(output);
        myfile.println(output);
        myfile.close();

        SPIFFS.remove("/linus_log.csv");  //Lösche Datei
        myfile = SPIFFS.open("/linus_log.csv", "w");  // Öffne Datei um Daten zu überschreiben ! (a - append)
        output = "";
        for (int i = 0; i < NUM_CHORES; i++) {
            output += Linus[i].name;
            output += ",";
        }
        Serial.print("writing to linus-file:");
        Serial.println(output);
        myfile.println(output);
        myfile.close();

        SPIFFS.remove("/jana_log.csv");  //Lösche Datei
        myfile = SPIFFS.open("/jana_log.csv", "w");  // Öffne Datei um Daten zu überschreiben ! (a - append)
        output = "";
        for (int i = 0; i < NUM_CHORES; i++) {
            output += Jana[i].name;
            output += ",";
        }
        Serial.print("writing to jana-file:");
        Serial.println(output);
        myfile.println(output);
        myfile.close();

        SPIFFS.remove("/scores_log.csv");  //Lösche Datei
        myfile = SPIFFS.open("/scores_log.csv", "w");  // Öffne Datei um Daten zu überschreiben ! (a - append)
        output = String(linus_score) + "," + String(jana_score);
        Serial.print("writing to scores-file:");
        Serial.println(output);
        myfile.println(output);
        myfile.close();

        changed = 0;


        //update messages for LED matrix
        int str_len;

        // jana:
        int counter_jana = 0;
        for (int i = 0; i < NUM_CHORES; i++) {
            if ((Jana[i].name != "gate closed") && (Jana[i].countdown <= 0)) {
                counter_jana += 1;
            }
        }
        add = " ";
        if (Jana[0].countdown < 0) {
            add += "- DELAYED ";
        }
        for (int i = 1; i < counter_jana; i++) {
            add += ".";
        }

        if (Jana[0].countdown <= 0) {
            buf = Jana[0].name;
        } else {
            buf = "gate closed";
        }
        buf.concat(add);
        str_len = buf.length() + 1;
        buf.toCharArray(newMessage, str_len);
        newMessageAvailable = true;

        // linus:
        int counter_linus = 0;
        for (int i = 0; i < NUM_CHORES; i++) {
            if ((Linus[i].name != "gate closed") && (Linus[i].countdown <= 0)) {
                counter_linus += 1;
            }
        }
        add = " ";
        if (Linus[0].countdown < 0) {
            add += "- DELAYED ";
        }
        for (int i = 1; i < counter_linus; i++) {
            add += ".";
        }

        if (Linus[0].countdown <= 0) {
            buf = Linus[0].name;
        } else {
            buf = "gate closed";
        }
        buf.concat(add);
        str_len = buf.length() + 1;
        buf.toCharArray(newMessage2, str_len);
        newMessageAvailable2 = true;

    }

    if ((digitalRead(D2) == HIGH) && (Linus[0].countdown <= 0)) {
        linus_score += 1 + Linus[0].countdown;
        Linus[0].done();
        delay(400);
    }

    if ((digitalRead(D1) == HIGH) && (Jana[0].countdown <= 0)) {
        jana_score += 1 + Jana[0].countdown;
        Jana[0].done();
        delay(400);
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

    server.handleClient();
}
