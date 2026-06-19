// LDR_Studio — Light level with telemetry.
// Experiment: LDR_EXPT  |  Pin: A7 (analog-only)
//
// Telemetry keys:
//   LGT — raw ADC light level 0-1023 (int16)

#include <InsightBasicStudio.h>

static int16_t lastLight = 0;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"LGT", I(lastLight)};
    *count = 1;
}

void setup() {
    studio.begin();
    studio.initExperiment({ LDR_EXPT });
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("LDR Studio — Light Level telemetry"));
}

void loop() {
    lastLight = (int16_t)analogRead(IB::LDR_SIG);
    studio.update();
    delay(50);
}
