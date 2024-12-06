#include <Arduino.h>
#include <DHT.h>
#include <painlessMesh.h>

#define DHTPIN 15     
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// RCWL0516 pin
#define MOTION_PIN 16

// Relay pins
#define RELAY_LAMP 17
#define RELAY_AC 18

// Mesh network parameters
#define MESH_SSID "SeizeTheClass"
#define MESH_PASSWORD "runincircles"
#define MESH_PORT 5007

painlessMesh mesh;

bool autoMode = true;  // Default mode: Auto
bool timerUpdated = false;
unsigned long motionCounter = 0;
unsigned long motionThreshold = 5;  
unsigned long timerDuration = 30000; // 30 seconds
unsigned long lastMotionTime = 0;

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(MOTION_PIN, INPUT);
    pinMode(RELAY_LAMP, OUTPUT);
    pinMode(RELAY_AC, OUTPUT);

    // Initialize relays as OFF
    digitalWrite(RELAY_LAMP, LOW);
    digitalWrite(RELAY_AC, LOW);

    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive([](uint32_t from, String &msg) {
        handleReceivedMessage(msg);
    });
}

void loop() {
    mesh.update();

    if (autoMode) {
        handleAutoMode();
    }
}

// Handle motion detection in Auto Mode
void handleAutoMode() {
    if (digitalRead(MOTION_PIN) == HIGH) {
        motionCounter++;
        lastMotionTime = millis();
    }

    if (motionCounter >= motionThreshold) {
        digitalWrite(RELAY_LAMP, HIGH);
        digitalWrite(RELAY_AC, HIGH);
        delay(timerDuration);
        motionCounter = 0;
        digitalWrite(RELAY_LAMP, LOW);
        digitalWrite(RELAY_AC, LOW);
    }
}

// Handle messages from Root Node
void handleReceivedMessage(String &msg) {
    if (msg.startsWith("TIMER:")) {
        String timerValue = msg.substring(6);
        timerDuration = timerValue.toInt();
        timerUpdated = true;
        Serial.println("Timer updated: " + String(timerDuration / 1000) + " seconds");
    } else if (msg == "OVERRIDE_ON") {
        autoMode = false;
        digitalWrite(RELAY_LAMP, HIGH);
        digitalWrite(RELAY_AC, HIGH);
    } else if (msg == "OVERRIDE_OFF") {
        autoMode = true;
        digitalWrite(RELAY_LAMP, LOW);
        digitalWrite(RELAY_AC, LOW);
    }
}
