// Buttons_Studio — Button states with telemetry.
// Experiment: BTN_EXPT  |  D2: Active-HIGH, D3: Active-LOW
//
// Telemetry keys:
//   BAH — active-HIGH button pressed (bool)
//   BAL — active-LOW button pressed (bool)

#include <InsightBasicStudio.h>

static bool btnAH  = false;
static bool btnAL  = false;
static bool prevAH = LOW;
static bool prevAL = HIGH;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"BAH", B(btnAH)};
    fields[1] = {"BAL", B(btnAL)};
    *count = 2;
}

void setup() {
    studio.begin();
    studio.initExperiment({ BTN_EXPT });
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("Buttons Studio — Button state telemetry"));
}

void loop() {
    bool curAH = digitalRead(IB::BTN_AH);
    bool curAL = digitalRead(IB::BTN_AL);

    btnAH = (curAH == HIGH);
    btnAL = (curAL == LOW);

    if (curAH == HIGH && prevAH == LOW)
        Serial.println(F("Active-HIGH pressed  (D2)"));
    if (curAH == LOW  && prevAH == HIGH)
        Serial.println(F("Active-HIGH released (D2)"));
    if (curAL == LOW  && prevAL == HIGH)
        Serial.println(F("Active-LOW pressed   (D3)"));
    if (curAL == HIGH && prevAL == LOW)
        Serial.println(F("Active-LOW released  (D3)"));

    prevAH = curAH;
    prevAL = curAL;

    studio.update();
    delay(20);
}
