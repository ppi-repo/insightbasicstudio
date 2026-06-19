// LDR_RGB_Buzzer_Studio — Light-reactive feedback with telemetry.
// Experiments: { LDR_EXPT, RGB_EXPT, BUZ_EXPT }
//
// Telemetry keys:
//   LGT — raw ADC light level (int16)
//   RED — red channel (int16)
//   BLU — blue channel (int16)
//   BZS — buzzer active (bool)

#include <InsightBasicStudio.h>

#define BRIGHT_THRESHOLD  700
#define ALERT_FREQ        1000

static int16_t lastLight = 0;
static uint8_t curR = 0, curB = 0;
static bool    buzActive = false;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"LGT", I(lastLight)};
    fields[1] = {"RED", I((int16_t)curR)};
    fields[2] = {"BLU", I((int16_t)curB)};
    fields[3] = {"BZS", B(buzActive)};
    *count = 4;
}

void setup() {
    studio.begin();
    studio.initExperiment({ LDR_EXPT, RGB_EXPT, BUZ_EXPT });
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("LDR + RGB + Buzzer Studio — light-reactive feedback"));
}

void loop() {
    lastLight = (int16_t)analogRead(IB::LDR_SIG);

    curR = map(lastLight, 0, 1023, 0, 255);
    curB = map(lastLight, 0, 1023, 255, 0);
    analogWrite(IB::RGB_R, curR);
    analogWrite(IB::RGB_G, 0);
    analogWrite(IB::RGB_B, curB);

    if (lastLight >= BRIGHT_THRESHOLD) {
        tone(IB::BUZZER, ALERT_FREQ);
        buzActive = true;
    } else {
        noTone(IB::BUZZER);
        buzActive = false;
    }

    studio.update();
    delay(50);
}
