#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <FastLED.h>

#define NUM_LEDS 1
#define DATA_PIN 0

CRGB leds[NUM_LEDS];

#ifndef STASSID
#define STASSID "network"
#define STAPSK "password"
#endif

unsigned char outIP[] = {192,168,14,226}; // The IP Address of your computer
int localPort = 1235; // The Port the Pico will send from (Doesn't matter much when sending but should be different from the outPort)
int outPort = 1234;

WiFiUDP Udp;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int analogInPin = A0;
const int analogOutPin = 9;

void setup(void) {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {} // Don't proceed, loop forever
  }

  display.clearDisplay(); // Clear display buffer
  display.cp437(true); // Render Text Properly
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Set the Text Color (White is the only option)
  display.setCursor(0, 0); // Start drawing/writing in the top left
  display.println("Connecting To Wifi");

  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    display.print(".");
    display.display();
    delay(500);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Connected to"));
  display.println(F(STASSID));
  display.println(F("IP Address"));
  display.println(WiFi.localIP());
  display.print(F("UDP Port : "));
  display.println(localPort);
  display.display();

  Udp.begin(localPort);
}

void loop(void) {
  int sensor1 = analogRead(A0); // Read from sensor on A0
  int outputValue = map(sensor1, 742, 405, 0, 255);
  analogWrite(analogOutPin, outputValue);

  OSCMessage msgOUT("/sensor"); // Set the Address to "/sensor"
  msgOUT.add(outputValue); // Add the value from the sensor to our message

  Udp.beginPacket(outIP, outPort);
  msgOUT.send(Udp);
  Udp.endPacket();
  msgOUT.empty();

}

void printStatus() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pico Send OSC");
  display.print("WiFi: ");
  display.println(F(STASSID));
  display.print("IP: ");
  display.println(WiFi.localIP());
}
