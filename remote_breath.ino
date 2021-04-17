#include <BlynkSimpleEsp32.h>
#include <Ticker.h>
#include <WiFiManager.h>

#include "creds.h" // add to .gitignore, holds BLYNK token.

// Pins
#define PIN_TACH_IN  15   // tach from input fan
#define PIN_FAN_OUT  23   // drives speed pin of output fan
#define PIN_LED      2    // on-board LED
#define PIN_ID       32   // sets ID of hardware
#define PIN_GND      33   // used to ground PIN_ID

#define BLYNK_RATE   200  // blynk push/pull rate, units ms
#define TIMEOUT_MS   1000 // Max time allowed between tach pulses, units ms

#ifndef TOKEN
  #error "define Blynk TOKEN in creds.h"
#endif

Ticker ticker;
uint8_t remote_state;

BLYNK_WRITE(0)
{
  remote_state = param.asInt();
}

BLYNK_WRITE(1)
{
  remote_state = param.asInt();
}

void toggle_led() {
  digitalWrite(PIN_LED, !digitalRead(PIN_LED));
}

void setup() {
  // Pin setup
  pinMode(PIN_TACH_IN, INPUT_PULLUP);
  pinMode(PIN_ID, INPUT_PULLUP);
  pinMode(PIN_GND, OUTPUT);
  pinMode(PIN_FAN_OUT, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_FAN_OUT, LOW);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_GND, LOW);

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

  // Keep LED high during main loop
  ticker.detach();
  digitalWrite(PIN_LED, HIGH);
}

void loop() {
  static uint8_t state_tach_in = digitalRead(PIN_TACH_IN);
  static uint8_t local_state = 0;
  static uint32_t timer_update = millis();
  static uint32_t timer_tach_in = UINT32_MAX / 2;

  // Read input fan tach
  if (!digitalRead(PIN_TACH_IN) != !state_tach_in) {
    state_tach_in = !state_tach_in;
    timer_tach_in = millis();
  }

  // Push/pull from Blynk at intervals
  if (millis() - timer_update > BLYNK_RATE) {
    if (!local_state != !(millis() - timer_tach_in < TIMEOUT_MS)) {
      // Send fan state to remote if changed
      local_state = !local_state;
      Blynk.virtualWrite(digitalRead(PIN_ID), local_state);
    }

    // Drive local fan based on remote state
    Blynk.syncVirtual(!digitalRead(PIN_ID));
    digitalWrite(PIN_FAN_OUT, remote_state);
  }

  Blynk.run();
}
