# Autonomous Boat
## Description

An embedded system project of building an autonomous boat that tracks a red blob and stops when an obstacle is detected.

![boat](./imgs/boat.png)

## Features
- Drive straight towards a red blob (if none is visible, stop the motors).
- Avoid forward collision by reading the sensor and stopping in time.

## Tools
### Hardware
- T-Display 16MB CH9102F 
- ESP32-CAM
- OV2640
- HC-SR04 ultrasonic distance sensor
- Breadboard
- Jump wires
- L289N motor driver
- 4-AA battery holder
- Kmart 1:14 Series Radio Control Super Speed Boat

#### Connection diagram
![connections](./imgs/connections.png)

### Software
The code is written using [Arduino](https://www.arduino.cc/en/software). 

Setup and upload steps are available in [setup.txt](./setup.txt).

## Libraries Used
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [TTGO-T-Display](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)
- [WiFi](https://github.com/arduino-libraries/WiFi)
- [HttpClient](https://github.com/amcewen/HttpClient)

## Acknowledgment
- Embedded Robotics by Thomas Bräunl.
- [RGB to HSL](https://www.rapidtables.com/convert/color/rgb-to-hsl.html) conversion by RapidTables.
- [Tutorial](https://randomnerdtutorials.com/esp32-client-server-wi-fi/) by Random Nerd Tutorials.