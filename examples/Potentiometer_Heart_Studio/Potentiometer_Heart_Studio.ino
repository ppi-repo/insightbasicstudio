// Potentiometer_Heart_Studio — Pot controls heart rate with BPM telemetry.
// Experiments: { POT_EXPT, HRT_EXPT }
//
// Telemetry keys:
//   POT — raw ADC pot position (int16)
//   BPM — current heart rate (int16)

#include <InsightBasicStudio.h>

#define BPM_MIN    40
#define BPM_MAX   180
#define LUB_DUR    80
#define INTER_DUR  80
#define DUB_DUR    80
#define PULSE_TOTAL (LUB_DUR + INTER_DUR + DUB_DUR)

static int16_t lastPot = 0;
static int16_t lastBPM = 0;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"POT", I(lastPot)};
    fields[1] = {"BPM", I(lastBPM)};
    *count = 2;
}

void setup() {
    studio.begin();
    studio.initExperiment({ POT_EXPT, HRT_EXPT });
    studio.registerPayload(buildPayload, 2000);
    Serial.println(F("Pot + Heart Studio — turn pot to change BPM"));
}

void loop() {
    lastPot = (int16_t)analogRead(IB::POT_SIG);
    lastBPM = (int16_t)map(lastPot, 0, 1023, BPM_MIN, BPM_MAX);
    long period = 60000L / lastBPM;

    // Lub
    digitalWrite(IB::HEART, HIGH);
    delay(LUB_DUR);
    digitalWrite(IB::HEART, LOW);
    delay(INTER_DUR);

    // Dub
    digitalWrite(IB::HEART, HIGH);
    delay(DUB_DUR);
    digitalWrite(IB::HEART, LOW);

    // Rest — call update() during the pause
    long rest = period - PULSE_TOTAL;
    uint32_t restStart = millis();
    while ((long)(millis() - restStart) < rest) {
        studio.update();
        delay(10);
    }
}
