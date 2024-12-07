#define BLYNK_PRINT Serial 
#define BLYNK_TEMPLATE_ID "TMPL6B0UuhKtb"
#define BLYNK_TEMPLATE_NAME "Finpro IoT"
#define BLYNK_AUTH_TOKEN "T7Lzogew03q6tNpsgTLZ2J_ddthYA7MG"

#include <Arduino.h>
#include <painlessMesh.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define MESH_SSID "SeizeTheClass"
#define MESH_PASSWORD "runincircles"
#define MESH_PORT 5007

painlessMesh mesh;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Arifa";
char pass[] = "aliaskosonq";

String sensorData = "";
int manualTimerDuration = 30 * 60 * 1000; 
String classStartTime = "08:00";          
String classEndTime = "09:40";            
bool lampStatus = false;
bool acStatus = false;

void setup() {
    Serial.begin(115200);

    Blynk.begin(auth, ssid, pass);

    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    mesh.onNewConnection(onNewConnection);
    mesh.onReceive([](uint32_t from, String &msg) {
        handleReceivedMessage(msg);
    });

    String initialTimerMessage = "TIMER:" + String(manualTimerDuration);
    mesh.sendBroadcast(initialTimerMessage);
}

void loop() {
    mesh.update();
    Blynk.run();

    updateClassActiveStatus();
    delay(1000);
}

void onNewConnection(uint32_t nodeId) {
    String timerMessage = "TIMER:" + String(manualTimerDuration);
    mesh.sendSingle(nodeId, timerMessage);

    String startTimeMessage = "START_TIME:" + classStartTime;
    String endTimeMessage = "END_TIME:" + classEndTime;
    mesh.sendSingle(nodeId, startTimeMessage);
    mesh.sendSingle(nodeId, endTimeMessage);

    Serial.println("Timer and class schedule synced to Node ID: " + String(nodeId));
}

void handleReceivedMessage(String &msg) {
    if (msg.startsWith("TEMP")) {

        sensorData = msg;
        Serial.println("Received data: " + sensorData);
        String temp = msg.substring(5, msg.indexOf(","));
        String hum = msg.substring(msg.indexOf("HUM:") + 4);
        Blynk.virtualWrite(V1, temp.toDouble());
        Blynk.virtualWrite(V2, hum.toDouble());

    } else if (msg.startsWith("RELAY_STATUS:")) {
       
        if (msg.indexOf("LAMP_ON") > 0) {
            lampStatus = true;
        } else if (msg.indexOf("LAMP_OFF") > 0) {
            lampStatus = false;
        }

        if (msg.indexOf("AC_ON") > 0) {
            acStatus = true;
        } else if (msg.indexOf("AC_OFF") > 0) {
            acStatus = false;
        }

        Serial.print("Lamp Status: ");
        Serial.print(lampStatus ? "ON" : "OFF");
        Serial.print(" | AC Status: ");
        Serial.println(acStatus ? "ON" : "OFF");
    }
}

BLYNK_WRITE(V0) {  
    int overrideMode = param.asInt(); 
    if (overrideMode == 1) {
        mesh.sendBroadcast("OVERRIDE_ON");
    } else {
        mesh.sendBroadcast("OVERRIDE_OFF");
    }
}

BLYNK_WRITE(V3) {
    mesh.sendBroadcast("READ_SENSORS");
}

BLYNK_WRITE(V4) {
    int timerValue = param.asInt(); 
    manualTimerDuration = timerValue * 60 * 1000; 
    String timerMessage = "TIMER:" + String(manualTimerDuration);
    mesh.sendBroadcast(timerMessage); 
    Serial.println("Timer updated to: " + String(timerValue) + " minutes");
}

BLYNK_WRITE(V7) {
    classStartTime = param.asStr();
    String startTimeMessage = "START_TIME:" + classStartTime;
    mesh.sendBroadcast(startTimeMessage);
    Serial.println("Class start time updated: " + classStartTime);
}

BLYNK_WRITE(V8) {
    classEndTime = param.asStr(); 
    String endTimeMessage = "END_TIME:" + classEndTime;
    mesh.sendBroadcast(endTimeMessage); 
    Serial.println("Class end time updated: " + classEndTime);
}

void updateClassActiveStatus() {
    bool isActive = lampStatus || acStatus;

    Blynk.virtualWrite(V9, isActive ? 1 : 0);

    Serial.print("Lamp Status: "); Serial.print(lampStatus);
    Serial.print(" | AC Status: "); Serial.print(acStatus);
    Serial.print(" | Class Active Status: "); Serial.println(isActive);
}
