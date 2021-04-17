#include <BlynkSimpleEsp32.h>
#include <Ticker.h>
#include <WiFiManager.h>

// add to .gitignore, holds BLYNK token.
#include "creds.h"

// Pins
#define PIN_TACH_IN  15
#define PIN_FAN_OUT  23
#define PIN_LED      2

// Max time allowed between tach pulses
#define TIMEOUT_MS  1000

// Blynk things
#define BLYNK_PRINT Serial
#ifndef TOKEN
  #error "define TOKEN in creds.h"
#endif

Ticker ticker;

void toggle_led() {
  digitalWrite(PIN_LED, !digitalRead(PIN_LED));
}

void setup() {
  // Setup pin modes
  pinMode(PIN_TACH_IN, INPUT_PULLUP);
  pinMode(PIN_FAN_OUT, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  // Set fan and LED off
  digitalWrite(PIN_FAN_OUT, LOW);
  digitalWrite(PIN_LED, LOW);

  // WiFi setup
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);    // 3min timeout
  ticker.attach(0.5, toggle_led); // slow blink
  if(!wifiManager.autoConnect("remote_breath")) {
    ESP.restart(); //reset and try again
    while(1);
  }

  // Blynk setup
  ticker.attach(0.2, toggle_led); // fast blink
  String wifi_ssid = WiFi.SSID();
  String wifi_pass = WiFi.psk();
  char blynk_ssid[wifi_ssid.length()+1];
  char blynk_pass[wifi_pass.length()+1];
  wifi_ssid.toCharArray(blynk_ssid, wifi_ssid.length()+1);
  wifi_pass.toCharArray(blynk_pass, wifi_pass.length()+1);
  Blynk.begin(TOKEN, blynk_ssid, blynk_pass);

  ticker.detach();
  digitalWrite(PIN_LED, HIGH);
}

void loop() {
  Blynk.run();
  static uint8_t state_tach_in = digitalRead(PIN_TACH_IN);
  static uint32_t timer_tach_in = UINT32_MAX / 2;

  // Read input fan tach
  if (!digitalRead(PIN_TACH_IN) != !state_tach_in) {
    state_tach_in = !state_tach_in;
    timer_tach_in = millis();
  }

  digitalWrite(PIN_FAN_OUT, (millis() - timer_tach_in < TIMEOUT_MS));
}
