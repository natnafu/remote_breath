#include <Arduino.h>
#include "math.h"

#define PIN_TACH_IN     15
#define PIN_PWM_OUT     23

#define PWM_FREQUENCY   25000
#define PWM_CHANNEL     0
#define PWM_RESOLUTION  8

#define TIMEOUT_MS      1000

#define MAX_INPUT_DELTA 7.0

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TACH_IN, INPUT_PULLUP);
  pinMode(PIN_PWM_OUT, OUTPUT);

  // Setup PWM output
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM_OUT, PWM_CHANNEL);
}

void loop() {
  static uint8_t state_tach_in  = digitalRead(PIN_TACH_IN);
  static uint32_t timer_tach_in = 0;
  static uint32_t delta_in = 0;
  static uint8_t duty = 0;

  // Read input fan tach
  if (!digitalRead(PIN_TACH_IN) != !state_tach_in) {
    state_tach_in = !state_tach_in;
    if (timer_tach_in == 0) {
      timer_tach_in = millis();
    } else {
      delta_in = millis() - timer_tach_in;
      Serial.print(delta_in); Serial.println("ms");
      timer_tach_in += delta_in;
    }
  }

  if (millis() - timer_tach_in >= TIMEOUT_MS) timer_tach_in = 0;

  // Drive fan if input fan spinning
  if (timer_tach_in) duty = (uint8_t)(255.0 * MAX_INPUT_DELTA / (float) delta_in); 
  else duty = 0;

  duty = duty >= 255 ? 255 : duty;
  //Serial.print("duty: "); Serial.println(duty);
  ledcWrite(PWM_CHANNEL, duty);
}