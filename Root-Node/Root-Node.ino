#include <Arduino.h>
#include <painlessMesh.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define MESH_SSID "SeizeTheClass"
#define MESH_PASSWORD "runincircles"
#define MESH_PORT 5007

#define BLYNK_TEMPLATE_ID "[not yet]"
#define BLYNK_TEMPLATE_NAME "[not yet]"
#define BLYNK_AUTH_TOKEN "[not yet]"

painlessMesh mesh;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "[YourWiFiSSID]";
char pass[] = "[YourWiFiPassword]";

String sensorData = "";
int manualTimerDuration = 30000;

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
}

void onNewConnection(uint32_t nodeId) {
    String timerMessage = "TIMER:" + String(manualTimerDuration);
    mesh.sendSingle(nodeId, timerMessage);
    Serial.println("Timer sync sent to Node ID: " + String(nodeId));
}

void handleReceivedMessage(String &msg) {
    if (msg.startsWith("TEMP")) {
        sensorData = msg;
        Serial.println("Received data: " + sensorData);
        String temp = msg.substring(5, msg.indexOf(","));
        String hum = msg.substring(msg.indexOf("HUM:") + 4);
        Blynk.virtualWrite(V1, temp.toFloat());
        Blynk.virtualWrite(V2, hum.toFloat());
    }
}

BLYNK_WRITE(V0) {
    int mode = param.asInt();
    if (mode == 1) {
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
    manualTimerDuration = timerValue * 1000; 
    String timerMessage = "TIMER:" + String(manualTimerDuration);
    mesh.sendBroadcast(timerMessage); 
    Serial.println("Manual timer set to: " + String(manualTimerDuration / 1000) + " seconds");
}
