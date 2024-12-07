# Final Project IoT - Group 7

## **Seize the Class: Smart Classroom Occupancy Control System**

---
## Overview
This project aims to optimize electricity usage in classrooms located on the same floor of a building by implementing a smart control system. Utilizing a mesh network powered by painlessMesh and Blynk IoT Platform as the user interface, the system allows facility managers to monitor and control the status of classroom lights and air conditioners (AC) based on occupancy and environmental conditions. Two operation modes, Auto and Override, are implemented to provide flexibility and efficiency.

In Auto mode, motion sensors automatically activate lights and AC based on the detected activity. In Override mode, the manual control from the Blynk interface ensures that devices stay in their last state regardless of detected motion. The system also includes temperature and humidity monitoring using DHT11 sensors and displays the data on the Blynk console.

---

## Background
Efficient energy management is critical in shared spaces like classrooms, where lights and ACs are often left running unnecessarily. Traditional systems rely heavily on manual intervention, leading to waste and inefficiency. This project addresses these issues by introducing a smart occupancy control system that automates and enhances classroom management. This system uses RCWL0516 Microwave Radar Sensors for motion detection, DHT11 sensors for environmental monitoring, and ESP32 microcontrollers to process and transmit data between nodes.

### Key Challenges Addressed:
1. **Energy Efficiency**: Reducing electricity wastage through intelligent automation.
2. **Ease of Management**: Providing facility managers with a user-friendly interface for remote monitoring and control.
3. **Scalability**: Using a mesh network to support multiple nodes within the same system.

---

## Project Description
### System Features
1. **Auto Mode**:
   - Uses RCWL0516 sensors to detect motion.
   - Activates lights after a single motion detection and both lights and AC after two or more detections.
   - Devices remain active for a timer-based duration if motion persists.

2. **Override Mode**:
   - Allows manual control via the Blynk IoT interface.
   - Devices remain in their manually set state, irrespective of motion detection.

3. **Environmental Monitoring**:
   - Monitors temperature and humidity using DHT11 sensors.
   - Displays data in real-time on the Blynk console.

4. **Mesh Network**:
   - Root Node connects to Wi-Fi and manages communication between Leaf Nodes.
   - Leaf Nodes handle sensor readings and device control.

5. **Scheduled Shutdown**:
   - Automatically shuts down all devices at a specified time (e.g., 8:00 PM), unless motion is detected.

---

### Workflow
- **Leaf Nodes**:
  - Equipped with RCWL0516 sensors, DHT11 sensors, and relays to control lights and ACs.
  - Periodically send data (motion status, room status, device status, temperature, and humidity) to the Root Node.

- **Root Node**:
  - Connected to Wi-Fi and the Blynk platform.
  - Displays real-time room conditions on the Blynk dashboard.
  - Sends manual or automated commands to Leaf Nodes via the mesh network.

- **User Interface**:
  - Facility managers interact with the system through the Blynk IoT Console.
  - Enables real-time monitoring and manual device control.

---

### Components
1. **Hardware**:
   - 2 ESP32 modules (1 Root Node, 1 Leaf Node).
   - 1 RCWL0516 Microwave Radar Sensors.
   - 1 DHT11 sensors.
   - 2 Relays.
   - Breadboards, jumper wires, and optional LED indicators.

2. **Software**:
   - **painlessMesh**: To establish a wireless communication network.
   - **Blynk IoT Platform**: For remote monitoring and control.

---

### Key Functionalities
- **Occupancy Detection**:
  - Single motion detection activates lights.
  - Five or more motions activate both lights and AC, assuming a class is starting.

- **Environmental Monitoring**:
  - Temperature and humidity data are displayed on Blynk widgets.

- **Manual Override**:
  - Facility managers can force devices ON/OFF through the Blynk interface.

- **Energy Conservation**:
  - Scheduled shutdown prevents energy waste in unoccupied classrooms.
