// Potentiometer_Studio — Pot position with telemetry.
// Experiment: POT_EXPT  |  Pin: A6 (analog-only)
//
// Telemetry keys:
//   POT — raw ADC pot position 0-1023 (int16)

#include <InsightBasicStudio.h>

static int16_t lastPot = 0;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"POT", I(lastPot)};
    *count = 1;
}

void setup() {
    studio.begin();
    studio.initExperiment({ POT_EXPT });
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("Potentiometer Studio — Position telemetry"));
}

void loop() {
    lastPot = (int16_t)analogRead(IB::POT_SIG);
    studio.update();
    delay(50);
}
