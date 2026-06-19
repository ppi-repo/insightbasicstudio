// Buzzer_Studio — Buzzer controlled from dashboard with status telemetry.
// Experiment: BUZ_EXPT  |  Pin: D6
//
// Telemetry keys:
//   BZS — buzzer active (bool)
//   FRQ — current frequency in Hz (int16), 0 = silent
//
// Dashboard commands:
//   BUZ:0       — silence buzzer
//   BUZ:freq    — play tone at freq Hz (e.g. "BUZ:1000")

#include <InsightBasicStudio.h>

static bool    buzActive = false;
static int16_t buzFreq   = 0;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"BZS", B(buzActive)};
    fields[1] = {"FRQ", I(buzFreq)};
    *count = 2;
}

void onCommand(const char* key, const char* value) {
    if (strcmp(key, "BUZ") == 0) {
        int freq = atoi(value);
        if (freq <= 0) {
            noTone(IB::BUZZER);
            buzActive = false;
            buzFreq   = 0;
            Serial.println(F("Buzzer off"));
        } else {
            tone(IB::BUZZER, (uint16_t)freq);
            buzActive = true;
            buzFreq   = (int16_t)freq;
            Serial.print(F("Buzzer: "));
            Serial.print(freq);
            Serial.println(F(" Hz"));
        }
    }
}

void setup() {
    studio.begin();
    studio.initExperiment({ BUZ_EXPT });
    studio.registerPayload(buildPayload, 1000);
    studio.registerCommandHandler(onCommand);
    Serial.println(F("Buzzer Studio — dashboard buzzer control"));
}

void loop() {
    studio.update();
}
