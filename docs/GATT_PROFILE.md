# **BLE Server Profile for scanForge's 3D Scanning Project**

This document defines the Generic Attribute Profile (GATT) for the BLE server running on the ESP32. It specifies the custom service, characteristics, and the communication protocol required for the 3D scanning application.

### PROMPT

Here is the basic LLM friendly prompt of the service spec written here

```text
Service:
  Name: photogrammaticScanner
  UUID: CODECAFE0000BAD0BAE00000DEADBEA7

Characteristic:
  Name: Command Control
  UUID: 0000BABE-0000-1000-8000-00805f9b34fb
  Properties: Write
  Permissions: Write
  Enum: START, NEXT, STOP, RESUME

Characteristic:
  Name: Scanning Status
  UUID: 0000FOOD-0000-1000-8000-00805f9b34fb
  Properties: Notify
  Permissions: Read, Notify
  Enum: SHOOT, END, STOPPED
```

### **Service Definition**

A single custom primary service will be used to group all related characteristics for the scanning process.

| Attribute | Value | Description |
| :---- | :---- | :---- |
| **Service Name** | photogrammaticScanner | A descriptive name for the service. |
| **UUID** | CODECAFE0000BAD0BAE00000DEADBEA7 | A custom 128-bit UUID for the service. |
| **Type** | Primary Service | This service is the main entry point for the scanning functions. |

### **Characteristics**

The service will contain two characteristics to manage the command and status flow.

#### **1\. Command Characteristic**

This characteristic is used by the Android app to send commands to the ESP32.

| Attribute | Value | Description |
| :---- | :---- | :---- |
| **Characteristic Name** | Command Control | Controls the state of the scanning process. |
| **UUID** | 0000BABE-0000-1000-8000-00805f9b34fb | A standard 128-bit UUID based on the 16-bit BABE value. |
| **Properties** | Write | Allows the client (Android app) to write commands to the characteristic. |
| **Data Format** | UTF-8 String | Commands are sent as short text strings (e.g., "START"). |

#### **2\. Status Characteristic**

This characteristic is used by the ESP32 to notify the Android app of the current status or required actions.

| Attribute | Value | Description |
| :---- | :---- | :---- |
| **Characteristic Name** | Scanning Status | Provides real-time status updates to the client. |
| **UUID** | 0000FOOD-0000-1000-8000-00805f9b34fb | A standard 128-bit UUID based on the 16-bit FOOD value. |
| **Properties** | Notify | The server (ESP32) can send notifications to the client. The client must subscribe to these notifications. |
| **Data Format** | UTF-8 String | Status signals are sent as short text strings (e.g., "SHOOT"). |

### **Communication Protocol**

The interaction between the Android app and the ESP32 follows a defined sequence of commands and notifications. The table below explicitly shows which characteristic is used for each signal.

| Sender | Signal | Characteristic | Description |
| :---- | :---- | :---- | :---- |
| **Android App** | START | **Command Characteristic** | Initiates the 360-degree scanning process. |
| **ESP32** | SHOOT | **Status Characteristic** | Notifies the app to take a picture. This signal is sent after every 5-degree rotation. |
| **Android App** | NEXT | **Command Characteristic** | Confirms the picture has been taken and the ESP32 can rotate to the next position. |
| **Android App** | STOP | **Command Characteristic** | Immediately halts the scanning process, regardless of its current state. |
| **Android App** | RESUME | **Command Characteristic** | Continues the scanning process from the last stopped position. This is only valid if a STOP command was previously sent. |
| **ESP32** | END | **Status Characteristic** | Signals that the 360-degree scan is complete. The process is over. |
| **ESP32** | STOPPED | **Status Characteristic** | Acknowledges that the STOP command has been received and the process has been halted. |

**Example Flow**

1. **Connection:** Android app connects to the ESP32 and subscribes to the Scanning Status characteristic.  
2. **Start Scan:** App writes START to the Command Control characteristic.  
3. **Picture Loop:**  
   * ESP32 rotates 5 degrees, stops, and notifies SHOOT.  
   * App receives SHOOT notification, takes a picture, and writes NEXT to Command Control.  
   * This continues until a full 360 degrees is complete.  
4. **End Scan:** Once the 360-degree rotation is complete, ESP32 notifies END. The connection can then be closed.

**Emergency Stop Flow**

1. **Ongoing Scan:** The picture loop is in progress.  
2. **Stop Command:** App writes STOP to Command Control.  
3. **Process Halt:** ESP32 immediately stops all rotation and notifies STOPPED. The process is now paused.  
4. **Resume:** App can write RESUME to Command Control to continue the loop from the last position.

This profile provides a robust and clear communication channel for your project.