#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <painlessMesh.h>
#include <time.h>
#include <DHT.h>

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

DHT dht(DHTPIN, DHTTYPE);
painlessMesh mesh;

bool relaysActive = false;
int motionCounter = 0;

SemaphoreHandle_t motionMutex; //modifikasi: menambahkan mutex untuk sinkronisasi gerakan
QueueHandle_t dhtQueue;        //modifikasi: menambahkan queue untuk data DHT

// Task prototypes
void taskMotion(void *pvParameters);
void taskDHT(void *pvParameters);
void handleReceivedMessage(String &msg);

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing Leaf Node...");

    // Inisialisasi pin
    pinMode(MOTION_PIN, INPUT);
    pinMode(RELAY_LAMP, OUTPUT);
    pinMode(RELAY_AC, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(RELAY_LAMP, LOW);
    digitalWrite(RELAY_AC, LOW);
    digitalWrite(LED_PIN, LOW);

    // Inisialisasi DHT sensor
    dht.begin();

    // Inisialisasi mesh network
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive([](uint32_t from, String &msg) {
        Serial.println("Message received: " + msg);
        handleReceivedMessage(msg);
    });

    // Inisialisasi semaphore dan queue
    motionMutex = xSemaphoreCreateMutex(); //modifikasi: inisialisasi semaphore
    dhtQueue = xQueueCreate(5, sizeof(float[2])); //modifikasi: inisialisasi queue untuk suhu&kelembapan

    if (motionMutex == NULL || dhtQueue == NULL) {
        Serial.println("Failed to create semaphores or queues. Halting."); //modifikasi: check semaphore dan queue
        while (1);
    }

    // Inisialisasi task
    xTaskCreate(taskMotion, "Motion Task", 2048, NULL, 1, NULL);
    xTaskCreate(taskDHT, "DHT Task", 2048, NULL, 1, NULL);

    Serial.println("Setup complete.");
}

void loop() {
    mesh.update();
}

void taskMotion(void *pvParameters) {
    bool motionDetected = false;
    while (1) {
        motionDetected = digitalRead(MOTION_PIN);
        if (motionDetected) {
            if (xSemaphoreTake(motionMutex, portMAX_DELAY) == pdTRUE) {
                motionCounter++;
                xSemaphoreGive(motionMutex);
            }
            Serial.println("Motion detected! Counter: " + String(motionCounter));
            mesh.sendBroadcast("MOTION:" + String(motionCounter)); //modifikasi: broadcast jumlah motion
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void taskDHT(void *pvParameters) {
    while (1) {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
        } else {
            float dhtData[2] = {temperature, humidity}; //modifikasi: array untuk menyimpan data
            xQueueSend(dhtQueue, &dhtData, portMAX_DELAY); //modifikasi: kirim data ke queue

            String message = "Temperature: " + String(temperature, 2) + ", Humidity: " + String(humidity, 2);
            mesh.sendBroadcast(message); //modifikasi: broadcast data DHT
            Serial.println("Broadcasted DHT data: " + message);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void handleReceivedMessage(String &msg) {
    if (msg.startsWith("SHUTDOWN")) {
        digitalWrite(RELAY_LAMP, LOW);
        digitalWrite(RELAY_AC, LOW);
        Serial.println("Shutdown command received.");
    } else if (msg.startsWith("OVERRIDE_ON")) { //modifikasi: handle override on
        relaysActive = true;
        digitalWrite(RELAY_LAMP, HIGH);
        digitalWrite(RELAY_AC, HIGH);
        Serial.println("Override ON received.");
    } else if (msg.startsWith("OVERRIDE_OFF")) { //modifikasi: handle override off
        relaysActive = false;
        digitalWrite(RELAY_LAMP, LOW);
        digitalWrite(RELAY_AC, LOW);
        Serial.println("Override OFF received.");
    }
}
