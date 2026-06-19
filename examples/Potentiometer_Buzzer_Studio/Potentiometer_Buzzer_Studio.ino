// Potentiometer_Buzzer_Studio — Pot controls buzzer pitch with telemetry.
// Experiments: { POT_EXPT, BUZ_EXPT }
//
// Telemetry keys:
//   POT — raw ADC pot position (int16)
//   FRQ — current buzzer frequency in Hz (int16), 0 = silent

#include <InsightBasicStudio.h>

#define CENTRE      512
#define DEAD_ZONE    50
#define FREQ_MIN    200
#define FREQ_MAX   2000

static int16_t lastPot  = 0;
static int16_t lastFreq = 0;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"POT", I(lastPot)};
    fields[1] = {"FRQ", I(lastFreq)};
    *count = 2;
}

void setup() {
    studio.begin();
    studio.initExperiment({ POT_EXPT, BUZ_EXPT });
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("Pot + Buzzer Studio — sweep to control pitch"));
}

void loop() {
    lastPot = (int16_t)analogRead(IB::POT_SIG);

    if (abs(lastPot - CENTRE) < DEAD_ZONE) {
        noTone(IB::BUZZER);
        lastFreq = 0;
    } else {
        int freq = map(lastPot, 0, 1023, FREQ_MIN, FREQ_MAX);
        tone(IB::BUZZER, (uint16_t)freq);
        lastFreq = (int16_t)freq;
    }

    studio.update();
    delay(20);
}
