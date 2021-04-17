// Pins
#define PIN_TACH_IN  15
#define PIN_FAN_OUT  23

// Max time allowed between tach pulses
#define TIMEOUT_MS  1000

void setup() {
  pinMode(PIN_TACH_IN, INPUT_PULLUP);
  pinMode(PIN_FAN_OUT, OUTPUT);
  digitalWrite(PIN_FAN_OUT, LOW);
}

void loop() {
  static uint8_t state_tach_in = digitalRead(PIN_TACH_IN);
  static uint32_t timer_tach_in = UINT32_MAX / 2;

  // Read input fan tach
  if (!digitalRead(PIN_TACH_IN) != !state_tach_in) {
    state_tach_in = !state_tach_in;
    timer_tach_in = millis();
  }

  digitalWrite(PIN_FAN_OUT, (millis() - timer_tach_in < TIMEOUT_MS));
}
