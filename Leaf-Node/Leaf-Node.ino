#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <painlessMesh.h>
#include <WiFi.h>
#include <time.h>
#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN 15
#define MOTION_PIN 16
#define RELAY_LAMP 17
#define RELAY_AC 18

#define MESH_SSID "SeizeTheClass"
#define MESH_PASSWORD "runincircles"
#define MESH_PORT 5007

#define MOTION_THRESHOLD 5
#define TIMER_DURATION (120 * 60 * 1000) 

DHT dht(DHTPIN, DHTTYPE);
painlessMesh mesh;

bool autoMode = true;
unsigned long motionCounter = 0;
unsigned long relayOffTime = 0;
bool relaysActive = false;
String classStartTime = "08:00";
String classEndTime = "10:00";

SemaphoreHandle_t relayMutex;
SemaphoreHandle_t motionMutex;

TaskHandle_t controlTaskHandle = NULL;
QueueHandle_t dhtQueue;

struct DHTData {
    double temperature;
    double humidity;
};

void taskControlAuto(void *pvParameters);
void taskControlOverride(void *pvParameters);
void taskMotion(void *pvParameters);
void taskDHT(void *pvParameters);
void taskMesh(void *pvParameters);
void handleReceivedMessage(String &msg);
void setupTimeSync();
bool isClassActive();

void onNewConnection(uint32_t nodeId);
void onChangedConnections();

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");

    dht.begin();
    pinMode(DHTPIN, INPUT);
    dhtQueue = xQueueCreate(5, sizeof(DHTData));

    pinMode(MOTION_PIN, INPUT);
    pinMode(RELAY_LAMP, OUTPUT);
    pinMode(RELAY_AC, OUTPUT);

    digitalWrite(RELAY_LAMP, LOW);
    digitalWrite(RELAY_AC, LOW);

    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive([](uint32_t from, String &msg) {
        handleReceivedMessage(msg);
    });
    mesh.onNewConnection(onNewConnection);
    mesh.onChangedConnections(onChangedConnections);
    Serial.println("Mesh initialized successfully!");

    relayMutex = xSemaphoreCreateMutex();
    motionMutex = xSemaphoreCreateMutex();
    if (relayMutex == NULL || motionMutex == NULL) {
        Serial.println("Failed to create semaphores. Halting.");
        while (1);
    }

    setupTimeSync();

    if (xTaskCreate(taskControlAuto, "Task_Control_Auto", 4096, NULL, 1, &controlTaskHandle) != pdPASS) {
        Serial.println("Failed to create Task_Control_Auto");
    }
    if (xTaskCreate(taskMotion, "Task_Motion", 2048, NULL, 1, NULL) != pdPASS) {
        Serial.println("Failed to create Task_Motion");
    }
    if (xTaskCreate(taskDHT, "Task_DHT", 4096, NULL, 1, NULL) != pdPASS) {
        Serial.println("Failed to create Task_DHT");
    }
    if (xTaskCreate(taskMesh, "Task_Mesh", 8192, NULL, 1, NULL) != pdPASS) {
        Serial.println("Failed to create Task_Mesh");
    }

    Serial.println("Setup complete.");
}

void loop() {
    mesh.update();
    delay(5);
}

void setupTimeSync() {
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Time sync initialized.");
}

bool isClassActive() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char currentTime[6];
    strftime(currentTime, sizeof(currentTime), "%H:%M", &timeinfo);
    return (classStartTime <= String(currentTime) && classEndTime >= String(currentTime));
}

void taskControlAuto(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(relayMutex, portMAX_DELAY) == pdTRUE) {
            if (motionCounter >= MOTION_THRESHOLD) {
                if (!relaysActive) {
                    digitalWrite(RELAY_LAMP, HIGH);
                    digitalWrite(RELAY_AC, HIGH);
                    relaysActive = true;
                    relayOffTime = millis() + TIMER_DURATION;
                    Serial.println("Relays ON (Auto Mode)");
                }
            }

            if (relaysActive && millis() > relayOffTime) {
                digitalWrite(RELAY_LAMP, LOW);
                digitalWrite(RELAY_AC, LOW);
                relaysActive = false;
                motionCounter = 0;
                Serial.println("Relays OFF (Timer expired in Auto Mode)");
            }

            xSemaphoreGive(relayMutex);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void taskMotion(void *pvParameters) {
    while (1) {
        if (digitalRead(MOTION_PIN) == HIGH) {
            if (xSemaphoreTake(motionMutex, portMAX_DELAY) == pdTRUE) {
                motionCounter++;
                Serial.println("Motion detected. Counter: " + String(motionCounter));
                xSemaphoreGive(motionMutex);
            }
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void taskDHT(void *pvParameters) {
    DHTData dhtData;
    while (1) {
        dhtData.temperature = dht.readTemperature();
        dhtData.humidity = dht.readHumidity();

        if (isnan(dhtData.temperature) || isnan(dhtData.humidity)) {
            Serial.println("Failed to read from DHT sensor!");
        } else {
            Serial.println("Temperature: " + String(dhtData.temperature, 2) + "°C, Humidity: " + String(dhtData.humidity, 2) + "%");

            if (xQueueSend(dhtQueue, &dhtData, portMAX_DELAY) != pdPASS) {
                Serial.println("Failed to enqueue DHT data.");
            }
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS); 
    }
}

void taskMesh(void *pvParameters) {
    DHTData dhtData;
    while (1) {
        if (mesh.getNodeList().size() > 0) { 
            String status = "RELAY_STATUS:" + String(digitalRead(RELAY_LAMP) ? "LAMP_ON," : "LAMP_OFF,");
            status += String(digitalRead(RELAY_AC) ? "AC_ON" : "AC_OFF");
            if (!mesh.sendBroadcast(status)) {
                Serial.println("Failed to broadcast relay status.");
            } else {
                Serial.println("Broadcasting relay status: " + status);
            }

            if (xQueueReceive(dhtQueue, &dhtData, portMAX_DELAY)) {
                String message = "Temperature: " + String(dhtData.temperature, 2) + "°C, Humidity: " + String(dhtData.humidity, 2) + "%";
                if (!mesh.sendBroadcast(message)) {
                    Serial.println("Failed to broadcast DHT data.");
                } else {
                    Serial.println("Broadcasting DHT data: " + message);
                }
            }
        } else {
            Serial.println("No nodes connected. Skipping broadcast.");
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS); 
    }
}

void handleReceivedMessage(String &msg) {
    if (msg.startsWith("OVERRIDE_ON")) {
        autoMode = false;
        vTaskDelete(controlTaskHandle);
        if (xTaskCreate(taskControlOverride, "Task_Control_Override", 4096, NULL, 1, &controlTaskHandle) != pdPASS) {
            Serial.println("Failed to create Task_Control_Override");
        }
        Serial.println("Switched to Override Mode");
    } else if (msg.startsWith("OVERRIDE_OFF")) {
        autoMode = true;
        vTaskDelete(controlTaskHandle);
        if (xTaskCreate(taskControlAuto, "Task_Control_Auto", 4096, NULL, 1, &controlTaskHandle) != pdPASS) {
            Serial.println("Failed to create Task_Control_Auto");
        }
        Serial.println("Switched to Auto Mode");
    }
}

void onNewConnection(uint32_t nodeId) {
    Serial.println("New connection established with Node ID: " + String(nodeId));
}

void onChangedConnections() {
    Serial.println("Connections changed. Current nodes:");
    SimpleList<uint32_t>::iterator node = mesh.getNodeList().begin();
    while (node != mesh.getNodeList().end()) {
        Serial.println(" - Node ID: " + String(*node));
        ++node;
    }
}