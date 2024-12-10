# **Final Project IoT - Group 7** <br>**(Seize the Class: Smart Classroom Occupancy Control System)**

## **Overview**  
This project aims to optimize electricity usage in classrooms located on the same floor of a building by implementing a smart control system. Utilizing a mesh network powered by painlessMesh and Blynk IoT Platform as the user interface, the system allows facility managers to monitor and control classroom lights and air conditioners (AC) based on occupancy and environmental conditions.  

**Key Features:**  
- **Auto Mode**: Automates control based on motion detection.  
- **Override Mode**: Manual control using the Blynk IoT platform.  
- **Environmental Monitoring**: Displays temperature and humidity in real-time.  
- **Scheduled Shutdown**: Ensures devices are off after hours.  

---

## **Background**  
Efficient energy management is essential in shared spaces like classrooms, where unnecessary energy consumption often occurs. Traditional systems require manual intervention, which leads to inefficiency and waste. This project leverages IoT technology to automate energy management, ensuring flexibility, scalability, and ease of use.  

**Challenges Addressed:**  
1. Reducing electricity wastage with intelligent automation.  
2. Simplifying management through an intuitive user interface.  
3. Supporting multiple classrooms using a scalable mesh network.  

---

## **System Architecture**  

### **Leaf Node**  

![IoTFinpro-Page-1 drawio (3)](https://github.com/user-attachments/assets/13e98d54-8a84-423b-aaab-58ce90838015)

The Leaf Node is responsible for collecting data from the sensors and controlling devices like lights and ACs. Each classroom has its own Leaf Node, equipped with **RCWL0516 Microwave Radar Sensors** for motion detection and **DHT11 sensors** for environmental data. The Leaf Node communicates with the Root Node via the mesh network.  

**Functionality of the Leaf Node**:  
- **Motion Detection**: If motion is detected, the node can trigger devices like lights and AC.  
- **Environmental Monitoring**: Measures the temperature and humidity in the classroom and sends data to the Root Node.  
- **Control**: Relays connected to the Leaf Node allow for controlling lights and AC based on sensor readings or manual overrides.

### **Root Node**  

![IoTFinpro-Page-3 drawio](https://github.com/user-attachments/assets/4d69933b-1767-4f2a-b9ce-59d84d73de93)

The Root Node connects to Wi-Fi and serves as the central controller, coordinating communication between all the Leaf Nodes and the Blynk interface. It collects sensor data from the Leaf Nodes, processes it, and provides real-time updates on the Blynk dashboard.  

**Functionality of the Root Node**:  
- **Wi-Fi Connectivity**: Ensures seamless communication with the Blynk platform.  
- **Data Aggregation**: Collects and processes data from all connected Leaf Nodes.  
- **User Interface Control**: The Root Node communicates with the Blynk dashboard, allowing facility managers to manually control devices and monitor room conditions.

---

## **Key Functionalities**  

1. **Occupancy Detection**:  
   - **Motion Sensors**: The RCWL0516 sensors detect motion in the room.  
   - **Auto Mode**: Lights are turned on with the first detected motion, and both lights and AC are activated with two or more motions, indicating a class has started.  
   - **Manual Override**: Using the Blynk app, facility managers can manually switch lights and AC on or off, regardless of motion detection.

2. **Environmental Monitoring**:  
   - **Temperature & Humidity**: The DHT11 sensor continuously monitors room conditions.  
   - **Real-time Display**: The data is shown on the Blynk interface, allowing managers to keep track of environmental comfort.

3. **Energy Conservation**:  
   - **Scheduled Shutdown**: The system automatically turns off lights and AC at a preset time (e.g., 8:00 PM) unless motion is detected. This ensures energy is not wasted in unoccupied classrooms.

---

## **Hardware Schematic**  

![IoTFinpro-Page-2 drawio (4)](https://github.com/user-attachments/assets/bdc320aa-baf3-4cae-a4cf-bfac85a587c7)

1. **ESP32 Modules**:  
   - **Root Node**: The central unit connects to Wi-Fi and manages the mesh network.  
   - **Leaf Nodes**: Deployed in each classroom to monitor occupancy and control devices.  

2. **Sensors and Actuators**:  
   - **RCWL0516 Radar Sensors**: Detect motion in the classrooms.  
   - **DHT11 Sensors**: Monitor temperature and humidity.  
   - **Relays**: Control lights and air conditioners based on motion and manual inputs.  

---

## **Software**  
- **painlessMesh**: Establishes the mesh network between Leaf Nodes and the Root Node.  
- **Blynk IoT Platform**: Manages the user interface and provides remote control functionality.  

### **Serial Monitor Output**  
```  
[INFO] Motion detected: 3 instance(s)  
[INFO] Lights ON, AC ON  
[INFO] Temperature: 26°C, Humidity: 65%  
[INFO] Scheduled Shutdown: Devices OFF  
```

---

## **User Interface (Blynk Console)**  

![messageImage_1733837040731](https://github.com/user-attachments/assets/afa6f1c3-3565-4868-906b-8aae50e3e6be)

The Blynk IoT platform is used to visualize room conditions and provide manual control over connected devices. The console displays:  
- **Real-time motion status**: Whether the room is occupied.  
- **Temperature and humidity**: Current environmental data.  
- **Device status**: Whether the lights and AC are on or off.  
Facility managers can interact with the system using this interface, manually overriding the system or adjusting settings.

---

## **Real Implementation**  

![20241209_120901](https://github.com/user-attachments/assets/1c955c1e-5fcd-4314-850e-4d6079581986)

- ESP32, RCWL0516 motion sensor, DHT11 sensor, and relays are connected as per the schematic.  
- Neat and compact wiring ensures reliability during operation. 

---

## **Results**  

- **Efficiency**: Significant energy savings by ensuring devices only run when needed.  
- **Reliability**: The mesh network ensures robust and consistent communication between devices.  
- **Ease of Use**: The Blynk platform makes it easy for facility managers to monitor and control classrooms remotely.  

---

## **Future Enhancements**  

1. **Energy Analytics**: Adding features for energy usage tracking and analysis for improved decision-making.  
2. **Notifications**: Integrating real-time notifications to alert managers about important events, such as motion detection or temperature thresholds.  
3. **Expansion**: Expanding the system to additional classrooms and integrating with other building management systems for enhanced control.
