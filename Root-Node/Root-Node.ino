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
char ssid[] = "Gumara_indihome";
char pass[] = "ammar0209";

String sensorData = "";
int manualTimerDuration = 30 * 60 * 1000; 
String classStartTime = "08:00";          
String classEndTime = "09:40";            
bool lampStatus = false;
bool acStatus = false;

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing Root Node...");

    Blynk.begin(auth, ssid, pass);
    Serial.println("Connected to Blynk.");

    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION | DEBUG);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    Serial.println("Mesh initialized.");

    mesh.onNewConnection(onNewConnection);
    mesh.onReceive([](uint32_t from, String &msg) {
        Serial.println("Message received: " + msg);
        handleReceivedMessage(msg);
    });

    String initialTimerMessage = "TIMER:" + String(manualTimerDuration);
    Serial.println("Broadcasting initial timer message: " + initialTimerMessage);
    mesh.sendBroadcast(initialTimerMessage);

    Serial.println("Setup complete.");
}

void loop() {
    static unsigned long lastBroadcast = 0;
    mesh.update();
    Blynk.run();

    if (millis() - lastBroadcast > 5000) { 
        String statusMessage = "Lamp: " + String(lampStatus ? "ON" : "OFF") + ", AC: " + String(acStatus ? "ON" : "OFF");
        if (mesh.sendBroadcast(statusMessage)) {
            Serial.println("Broadcasted: " + statusMessage);
        } else {
            Serial.println("Failed to broadcast status.");
        }
        lastBroadcast = millis();
    }
    updateClassActiveStatus();
}

void onNewConnection(uint32_t nodeId) {
    Serial.println("New connection established with Node ID: " + String(nodeId));

    String timerMessage = "TIMER:" + String(manualTimerDuration);
    if (mesh.sendSingle(nodeId, timerMessage)) {
        Serial.println("Sent timer message to Node ID: " + String(nodeId));
    } else {
        Serial.println("Failed to send timer message to Node ID: " + String(nodeId));
    }

    String startTimeMessage = "START_TIME:" + classStartTime;
    if (mesh.sendSingle(nodeId, startTimeMessage)) {
        Serial.println("Sent class start time to Node ID: " + String(nodeId));
    }

    String endTimeMessage = "END_TIME:" + classEndTime;
    if (mesh.sendSingle(nodeId, endTimeMessage)) {
        Serial.println("Sent class end time to Node ID: " + String(nodeId));
    }
}

void handleReceivedMessage(String &msg) {
    if (msg.startsWith("Temperature: ")) {
        int tempIndex = msg.indexOf(":") + 2;
        int humIndex = msg.indexOf(", Humidity: ") + 12;

        String temp = msg.substring(tempIndex, humIndex - 12);
        String hum = msg.substring(humIndex);

        Serial.println("Received DHT data: " + msg);
        Blynk.virtualWrite(V1, temp.toDouble());
        Blynk.virtualWrite(V2, hum.toDouble());
        Serial.println("Temperature: " + temp + " | Humidity: " + hum);

    } else if (msg.startsWith("RELAY_STATUS:")) {
    
        lampStatus = msg.indexOf("LAMP_ON") > 0;
        acStatus = msg.indexOf("AC_ON") > 0;

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
