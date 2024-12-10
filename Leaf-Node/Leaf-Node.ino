#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <painlessMesh.h>
#include <WiFi.h>
#include <time.h>
#include <DHT.h>

// ======= Define Macro & Konstanta =======
#define DHTTYPE DHT11
#define DHTPIN 15
#define MOTION_PIN 16
#define RELAY_LAMP 17
#define RELAY_AC 18
#define LED_PIN 4

#define MESH_SSID "SeizeTheClass"
#define MESH_PASSWORD "runincircles"
#define MESH_PORT 5007

#define MOTION_THRESHOLD 5
#define TIMER_DURATION (1 * 60 * 1000) // 1 minute for testing

// ======= Deklarasi Variabel Global =======
DHT dht(DHTPIN, DHTTYPE);
painlessMesh mesh;

bool autoMode = true;
unsigned long motionCounter = 0;
bool relaysActive = false;
String classStartTime = "08:00";
String classEndTime = "09:40";

// FreeRTOS resource
SemaphoreHandle_t relayMutex;
SemaphoreHandle_t motionMutex;
QueueHandle_t dhtQueue;
TaskHandle_t controlTaskHandle = NULL;
TimerHandle_t relayAutoTimer;

struct DHTData {
    double temperature;
    double humidity;
};

// ======= Prototipe Fungsi =======
void taskControlAuto(void *pvParameters);
void taskControlOverride(void *pvParameters);
void taskMotion(void *pvParameters);
void taskDHT(void *pvParameters);
void taskMesh(void *pvParameters);
void relayAutoCallback(TimerHandle_t xTimer);
void handleReceivedMessage(String &msg);
void setupTimeSync();
bool isClassActive();
void onNewConnection(uint32_t nodeId);
void onChangedConnections();

// ======= Fungsi Setup =======
void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");

    // Initialize hardware
    dht.begin();
    pinMode(MOTION_PIN, INPUT);
    pinMode(RELAY_LAMP, OUTPUT);
    pinMode(RELAY_AC, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    // Initialize relay states
    digitalWrite(RELAY_LAMP, LOW);
    digitalWrite(RELAY_AC, LOW);
    digitalWrite(LED_PIN, LOW);

    // Initialize mesh network
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive([](uint32_t from, String &msg) { handleReceivedMessage(msg); });
    mesh.onNewConnection(onNewConnection);
    mesh.onChangedConnections(onChangedConnections);
    Serial.println("Mesh initialized successfully!");

    // Initialize FreeRTOS resources
    relayMutex = xSemaphoreCreateMutex();
    motionMutex = xSemaphoreCreateMutex();
    if (relayMutex == NULL || motionMutex == NULL) {
        Serial.println("Failed to create semaphores. Halting.");
        while (1);
    }

    // Initialize timer
    relayAutoTimer = xTimerCreate("RelayAutoTimer", pdMS_TO_TICKS(TIMER_DURATION), pdFALSE, (void *)0, relayAutoCallback);

    // Time sync
    setupTimeSync();

    // Create tasks
    xTaskCreate(taskControlAuto, "Task_Control_Auto", 4096, NULL, 1, &controlTaskHandle);
    xTaskCreate(taskMotion, "Task_Motion", 2048, NULL, 1, NULL);
    xTaskCreate(taskDHT, "Task_DHT", 4096, NULL, 1, NULL);
    xTaskCreate(taskMesh, "Task_Mesh", 8192, NULL, 1, NULL);

    Serial.println("Setup complete.");
}

// ======= Fungsi Loop =======
void loop() {
    mesh.update();
    delay(5);
}

// ======= Implementasi Task =======
void taskControlAuto(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(relayMutex, portMAX_DELAY)) {
            if (motionCounter >= MOTION_THRESHOLD && !relaysActive) {
                digitalWrite(RELAY_LAMP, HIGH);
                digitalWrite(RELAY_AC, HIGH);
                relaysActive = true;
                xTimerReset(relayAutoTimer, 0);
                Serial.println("Relays ON (Auto Mode)");
            }
            xSemaphoreGive(relayMutex);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void taskMotion(void *pvParameters) {
    bool previousMotionState = LOW;
    while (1) {
        bool currentMotionState = digitalRead(MOTION_PIN);
        if (currentMotionState == HIGH && previousMotionState == LOW) {
            if (xSemaphoreTake(motionMutex, portMAX_DELAY)) {
                motionCounter++;
                Serial.println("Motion detected. Counter: " + String(motionCounter));
                xSemaphoreGive(motionMutex);
            }
            digitalWrite(LED_PIN, HIGH);
        } else if (currentMotionState == LOW) {
            digitalWrite(LED_PIN, LOW);
        }
        previousMotionState = currentMotionState;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void taskDHT(void *pvParameters) {
    DHTData dhtData;
    while (1) {
        dhtData.temperature = dht.readTemperature();
        dhtData.humidity = dht.readHumidity();
        if (!isnan(dhtData.temperature) && !isnan(dhtData.humidity)) {
            Serial.println("Temperature: " + String(dhtData.temperature) + "°C, Humidity: " + String(dhtData.humidity) + "%");
            xQueueSend(dhtQueue, &dhtData, portMAX_DELAY);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void taskMesh(void *pvParameters) {
    DHTData dhtData;
    while (1) {
        if (mesh.getNodeList().size() > 0) {
            String status = "RELAY_STATUS:" + String(digitalRead(RELAY_LAMP) ? "LAMP_ON," : "LAMP_OFF,") +
                            String(digitalRead(RELAY_AC) ? "AC_ON " : "AC_OFF ");
            mesh.sendBroadcast(status);
            if (xQueueReceive(dhtQueue, &dhtData, portMAX_DELAY)) {
                String message = "Temperature: " + String(dhtData.temperature) + "°C, Humidity: " + String(dhtData.humidity) + "%";
                mesh.sendBroadcast(message);
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

// ======= Utility Function =======
void handleReceivedMessage(String &msg) {
    // Handle received messages
}

void relayAutoCallback(TimerHandle_t xTimer) {
    // Callback logic for timer
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

void onNewConnection(uint32_t nodeId) {
    Serial.println("New connection established with Node ID: " + String(nodeId));
}

void onChangedConnections() {
    Serial.println("Connections changed.");
}
