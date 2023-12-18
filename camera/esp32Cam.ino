#include "esp_camera.h"
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

const char* ssid = "camerateam32z";
const char* password = "per78dym69";
AsyncWebServer server(80);

int pos;
int hueR = 15;
int hueT = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  Serial.print("Initializing the camera module...");
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  Serial.println("Ok!");
  
  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/camera", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readCam().c_str());
  });
  
  // Start server
  server.begin();
}

void loop() {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  if(!fb){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
  }

  size_t fb_len = 0;
  if(fb->format != PIXFORMAT_JPEG){
    Serial.println("Non-JPEG data not implemented");
    return;
  }

  // allocate memory to store the rgb data (in psram, 3 bytes per pixel)
  uint8_t *rgb = NULL;                                                                           // create a pointer for memory location to store the data
  uint32_t ARRAY_LENGTH = fb->width * fb->height * 3; // calculate memory required to store the RGB data (i.e. number of pixels in the jpg image x 3)
                             
  // allocate memory space for the rgb data
  rgb = (uint8_t*)malloc(ARRAY_LENGTH);                                                             // create the 'rgb' array pointer to the allocated memory space

  // convert the captured jpg image (fb) to rgb data (store in 'rgb' array)                                                                              // store time that image conversion process started
  bool jpeg_converted = fmt2rgb888(fb->buf, fb->len, fb->format, rgb);
  if (!jpeg_converted) {
    Serial.println("jpeg_converted failed");
    return;
  }

  int x,y,val, count, distance, idx;
  pos=-1;
  val=0; /*init*/ 
  for (x = 0; x < fb->width; x ++) {
    count = 0;
    for (y = 0 ; y < fb->height; y ++) { 
      idx = 3*(y*fb->width + x); // color image
      float r = rgb[idx+2];
      float g = rgb[idx+1];
      float b = rgb[idx];
      float h, s, i;
      RGB2HSI(r, g, b, h, s, i);
      if (h > 0) { 
        distance = abs((int)h - hueT); /* hue distance */
        if (distance > 180) distance = 360 - distance;
        if (distance < hueR && s > 0.4 && i > 0.1 && i < 0.9) {
          count++; 
        }
      }
    }
    if(count > val){
      val = count;
      pos = x;
    }
  }

  // if position is in the middle portion, set the position to the middle of the image
  if (pos < 220 && pos > 100) {
    pos = 160;
  }

  esp_camera_fb_return(fb);
  free(rgb);     // rgb data
  delay(500);
}

// Convert RGB to HSI
void RGB2HSI(float r, float g, float b, float &h, float &s, float &i) {
  r = r / 255.0;
  g = g / 255.0;
  b = b / 255.0;
  float maxVal = max(r, max(g, b));
  float minVal = min(r, min(g, b));
  float delta = maxVal - minVal;

  i = (maxVal + minVal) / 2.0;

  if (delta == 0.0) {
    h = -1;
    s = 0;
  } else {
    if (maxVal == r) {
      h = 60 * (fmod((g - b)/delta, 6.0));
    } else if (maxVal == g) {
      h = 60 * ((b - r)/delta + 2.0);
    } else if (maxVal == b) {
      h = 60 * ((r - g)/delta + 4.0);
    }
    s = delta/(1.0 - abs(2*i - 1.0));
  }

  if (h < 0) {
    h += 360; // Ensure the hue is in the range [0, 360)
  }
}

// Read the position of the ball
String readCam() {
  return String(pos);
}
