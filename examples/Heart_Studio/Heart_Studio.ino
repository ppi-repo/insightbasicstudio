// Heart_Studio — Heart LED with dashboard BPM control and telemetry.
// Experiment: HRT_EXPT  |  Pin: D13
//
// Telemetry keys:
//   BPM — current heart rate in beats per minute (int16)
//
// Dashboard commands:
//   HRT:bpm — set heart rate (e.g. "HRT:90"), range 30-200

#include <InsightBasicStudio.h>

static int16_t currentBPM = 72;

#define LUB_DUR    80
#define INTER_DUR  80
#define DUB_DUR    80
#define PULSE_TOTAL (LUB_DUR + INTER_DUR + DUB_DUR)

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"BPM", I(currentBPM)};
    *count = 1;
}

void onCommand(const char* key, const char* value) {
    if (strcmp(key, "HRT") == 0) {
        int bpm = constrain(atoi(value), 30, 200);
        currentBPM = (int16_t)bpm;
        Serial.print(F("BPM set: "));
        Serial.println(bpm);
    }
}

void setup() {
    studio.begin();
    studio.initExperiment({ HRT_EXPT });
    studio.registerPayload(buildPayload, 2000);
    studio.registerCommandHandler(onCommand);
    Serial.print(F("Heart Studio — "));
    Serial.print(currentBPM);
    Serial.println(F(" BPM (send HRT:bpm to change)"));
}

void loop() {
    // Lub
    digitalWrite(IB::HEART, HIGH);
    delay(LUB_DUR);
    digitalWrite(IB::HEART, LOW);
    delay(INTER_DUR);

    // Dub
    digitalWrite(IB::HEART, HIGH);
    delay(DUB_DUR);
    digitalWrite(IB::HEART, LOW);

    // Rest — fill the remaining time, calling update() periodically
    long period = 60000L / currentBPM;
    long rest   = period - PULSE_TOTAL;
    uint32_t restStart = millis();
    while ((long)(millis() - restStart) < rest) {
        studio.update();
        delay(10);
    }
}
