#define BLYNK_PRINT Serial 
#define BLYNK_TEMPLATE_ID "TMPL6B0UuhKtb"
#define BLYNK_TEMPLATE_NAME "Finpro IoT"
#define BLYNK_AUTH_TOKEN "T7Lzogew03q6tNpsgTLZ2J_ddthYA7MG"

#include <Arduino.h>
#include <painlessMesh.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Konfigurasi mesh network
#define MESH_SSID "SeizeTheClass"
#define MESH_PASSWORD "runincircles"
#define MESH_PORT 5007

painlessMesh mesh;

// Konfigurasi Blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Arifa";
char pass[] = "aliaskosonq";

// Variabel global
String sensorData = "";
int manualTimerDuration = 30 * 60 * 1000; // Durasi timer manual (ms)
String classStartTime = "08:00";          // Waktu mulai kelas
String classEndTime = "09:40";            // Waktu akhir kelas
bool lampStatus = false;                  // Status lampu
bool acStatus = false;                    // Status AC
int motionCount = 0;                      // Hitungan motion terakhir
double temperature = 0.0;                 // Suhu dari sensor DHT
double humidity = 0.0;                    // Kelembapan dari sensor DHT

void setup() {
    Serial.begin(115200);

    // Inisialisasi Blynk
    Blynk.begin(auth, ssid, pass);

    // Inisialisasi mesh network
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    mesh.onNewConnection(onNewConnection);
    mesh.onReceive([](uint32_t from, String &msg) {
        handleReceivedMessage(msg);
    });

    // Kirim data awal timer
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
    // Sinkronisasi timer dan jadwal kelas dengan node baru
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
        // Proses data suhu dan kelembapan
        sensorData = msg;
        Serial.println("Received data: " + sensorData);
        String temp = msg.substring(5, msg.indexOf(","));
        String hum = msg.substring(msg.indexOf("HUM:") + 4);
        temperature = temp.toDouble();
        humidity = hum.toDouble();
        Blynk.virtualWrite(V1, temperature);
        Blynk.virtualWrite(V2, humidity);
    } else if (msg.startsWith("RELAY_STATUS:")) {
        // Proses status relay lampu dan AC
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
    } else if (msg.startsWith("MOTION:")) {
        // Proses data motion 
        String motionStr = msg.substring(7); // Ambil nilai motion
        motionCount = motionStr.toInt();
        Blynk.virtualWrite(V5, motionCount); // Tampilkan motion di Blynk 
        Serial.println("Motion count received: " + String(motionCount));
    }
}

BLYNK_WRITE(V0) {  
    // Override mode ON/OFF
    int overrideMode = param.asInt(); 
    if (overrideMode == 1) {
        mesh.sendBroadcast("OVERRIDE_ON");
    } else {
        mesh.sendBroadcast("OVERRIDE_OFF");
    }
}

BLYNK_WRITE(V3) {
    // Kirim perintah untuk membaca data sensor
    mesh.sendBroadcast("READ_SENSORS");
}

BLYNK_WRITE(V4) {
    // Atur ulang timer manual
    int timerValue = param.asInt(); 
    manualTimerDuration = timerValue * 60 * 1000; 
    String timerMessage = "TIMER:" + String(manualTimerDuration);
    mesh.sendBroadcast(timerMessage); 
    Serial.println("Timer updated to: " + String(timerValue) + " minutes");
}

BLYNK_WRITE(V7) {
    // Perbarui waktu mulai kelas
    classStartTime = param.asStr();
    String startTimeMessage = "START_TIME:" + classStartTime;
    mesh.sendBroadcast(startTimeMessage);
    Serial.println("Class start time updated: " + classStartTime);
}

BLYNK_WRITE(V8) {
    // Perbarui waktu akhir kelas
    classEndTime = param.asStr(); 
    String endTimeMessage = "END_TIME:" + classEndTime;
    mesh.sendBroadcast(endTimeMessage); 
    Serial.println("Class end time updated: " + classEndTime);
}

void updateClassActiveStatus() {
    // Perbarui status kelas aktif
    bool isActive = lampStatus || acStatus;

    Blynk.virtualWrite(V9, isActive ? 1 : 0);

    Serial.print("Lamp Status: "); Serial.print(lampStatus);
    Serial.print(" | AC Status: "); Serial.print(acStatus);
    Serial.print(" | Class Active Status: "); Serial.println(isActive);
}
