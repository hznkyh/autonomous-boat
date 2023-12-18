/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-client-server-wi-fi/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

int MotorL = 25;
int MotorR = 26;
bool motorStarted = false;

const char* ssid = "camerateam32z";
const char* password = "per78dym69";

String pos;

const char* serverNameCam = "http://192.168.4.1/camera";

unsigned long previousMillis = 0;
const long interval = 1000; 

int trigPin = 13;
int echoPin = 12;

#define SOUND_SPEED 0.034

long traveltime;
float distanceCm;

int test = 0;

unsigned long previousMicros = 0;

HTTPClient http;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  //Motor setup
  ledcSetup(0, 500, 8);
  ledcAttachPin(MotorL, 0);

  ledcSetup(1, 500, 8);
  ledcAttachPin(MotorR, 1);

  //Sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  tft.setCursor(0, 0);
  tft.print("Wifi connected");
  http.setReuse(true);
}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    getSensor();
    if(WiFi.status()== WL_CONNECTED ){ 
      pos = httpGETRequest(serverNameCam);
      Serial.println("Position: " + pos);
      // save the last HTTP GET Request
      previousMillis = currentMillis;
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  }

  //Clear screen
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(0, 20);
  tft.print("Distance (cm): ");
  tft.println(distanceCm);
  
  tft.setCursor(0, 40);
  tft.print("Colour position: ");
  tft.println(pos);

  if (int(distanceCm <= 50)){
    //Stop motor
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    motorStarted = false;
  }
  else {
    if (pos.toInt() != -1) {
      //start the motor with high pwm
      if(!motorStarted) {
        startMotor();
      }
      if (pos.toInt() >= 220) {
        //Turn right
        ledcWrite(0, 80);
        ledcWrite(1, 70);
      }
      else if (pos.toInt() <= 100) {
        //Turn left
        ledcWrite(0, 70);
        ledcWrite(1, 80);
      }
      else {
        //Forward
        ledcWrite(0, 70);
        ledcWrite(1, 70);
      }
    }
    else {
      motorStarted = false;
      ledcWrite(0, 0);
      ledcWrite(1, 0);
    }
  }
  tft.setCursor(0, 80);
  tft.print("Motor: ");
  tft.println(motorStarted);
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;

  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void startMotor() {
  motorStarted = true;
  ledcWrite(0, 255);
  ledcWrite(1, 255);
  delay(1000);
}

void getSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  //get the sound wave travel time
  traveltime = pulseIn(echoPin, HIGH);

  distanceCm = traveltime/2 * SOUND_SPEED;
}