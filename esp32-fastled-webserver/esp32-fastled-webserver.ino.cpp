# 1 "C:\\Users\\ALVINT~1\\AppData\\Local\\Temp\\tmpnrb43r1f"
#include <Arduino.h>
# 1 "F:/Github/esp32-fastled-webserver/esp32-fastled-webserver/esp32-fastled-webserver.ino"
# 25 "F:/Github/esp32-fastled-webserver/esp32-fastled-webserver/esp32-fastled-webserver.ino"
#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <DNSServer.h>
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3003000)
#warning "Requires FastLED 3.3 or later; check github for latest code."
#endif

AsyncWebServer webServer(80);
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

const int led = 5;

uint8_t autoplay = 0;
uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t currentPatternIndex = 0;

uint8_t gHue = 0;

uint8_t power = 1;
uint8_t brightness = 8;

uint8_t speed = 30;




uint8_t cooling = 50;




uint8_t sparking = 120;

CRGB solidColor = CRGB::Blue;

uint8_t cyclePalettes = 0;
uint8_t paletteDuration = 10;
uint8_t currentPaletteIndex = 0;
unsigned long paletteTimeout = 0;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN 13

#define LED_TYPE NEOPIXEL
#define COLOR_ORDER GRB
#define NUM_STRIPS 1
#define NUM_LEDS_PER_STRIP 60
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
CRGB leds[NUM_LEDS];

#define MILLI_AMPS 1600
#define FRAMES_PER_SECOND 120

#include "patterns.h"

#include "field.h"
#include "fields.h"

#include "secrets.h"
#include "web.h"
#include "dns.h"
# 104 "F:/Github/esp32-fastled-webserver/esp32-fastled-webserver/esp32-fastled-webserver.ino"
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}


int timeout = 60;
void setup();
void loop();
void nextPattern();
void nextPalette();
#line 137 "F:/Github/esp32-fastled-webserver/esp32-fastled-webserver/esp32-fastled-webserver.ino"
void setup() {


  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000);

  Wire.setTimeout(3000);
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
  #endif


  pinMode(led, OUTPUT);
  digitalWrite(led, 1);


  Serial.begin(115200);

  SPIFFS.begin();
  listDir(SPIFFS, "/", 1);
  init_mpu();


  WiFi.mode(WIFI_MODE_APSTA);


  Serial.printf("Connecting to %s\n", wifi_ssid);
  if (String(WiFi.SSID()) != String(wifi_ssid)) {
    WiFi.begin(wifi_ssid, wifi_password);
    while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
    timeout -= 1;
    if (timeout < 0) {
      Serial.println("Took too long");
      break;
    }
  }
    if(WiFi.status() == WL_CONNECTED){
       Serial.printf("IP address at %s\n", WiFi.localIP().toString());
    }
  }


  Serial.printf("Creating AP at %s\n", ap_ssid);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.printf("AP IP at %s\n", WiFi.softAPIP().toString());
  dnsServer.start(53, "*", WiFi.softAPIP());




  setupWeb();


  FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);


  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);


  FastLED.setBrightness(brightness);

  autoPlayTimeout = millis() + (autoplayDuration * 1000);
}

void loop()
{

  dnsServer.processNextRequest();

  handleWeb();
  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  else {

    patterns[currentPatternIndex].pattern();

    EVERY_N_MILLISECONDS(40) {

      nblendPaletteTowardPalette(currentPalette, targetPalette, 8);
      gHue++;
    }

    if (autoplay == 1 && (millis() > autoPlayTimeout)) {
      nextPattern();
      autoPlayTimeout = millis() + (autoplayDuration * 1000);
    }

    if (cyclePalettes == 1 && (millis() > paletteTimeout)) {
      nextPalette();
      paletteTimeout = millis() + (paletteDuration * 1000);
    }
  }



  FastLED.show();


  FastLED.delay(1000 / FRAMES_PER_SECOND);

}

void nextPattern()
{

  currentPatternIndex = (currentPatternIndex + 1) % patternCount;
}

void nextPalette()
{
  currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
  targetPalette = palettes[currentPaletteIndex];
}