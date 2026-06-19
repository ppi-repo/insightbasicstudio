// Buttons_LEDRing_Studio — Button-stepped LED ring with position telemetry.
// Experiments: { BTN_EXPT, RING_EXPT }
//
// Telemetry keys:
//   POS — current lit LED position 0-15 (int16)
//   BAH — active-HIGH button state (bool)
//   BAL — active-LOW button state (bool)

#include <InsightBasicStudio.h>

static int16_t ringPos = 0;
static bool    btnAH   = false;
static bool    btnAL   = false;
static bool    prevAH  = LOW;
static bool    prevAL  = HIGH;

static void writeRing(uint16_t pattern) {
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, highByte(pattern));
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, lowByte(pattern));
    digitalWrite(IB::SR_LATCH, HIGH);
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"POS", I(ringPos)};
    fields[1] = {"BAH", B(btnAH)};
    fields[2] = {"BAL", B(btnAL)};
    *count = 3;
}

void setup() {
    studio.begin();
    studio.initExperiment({ BTN_EXPT, RING_EXPT });
    writeRing(1u << ringPos);
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("Buttons + LED Ring Studio — press buttons to step"));
}

void loop() {
    bool curAH = digitalRead(IB::BTN_AH);
    bool curAL = digitalRead(IB::BTN_AL);

    btnAH = (curAH == HIGH);
    btnAL = (curAL == LOW);

    if (curAH == HIGH && prevAH == LOW) {
        ringPos = (ringPos + 1) % 16;
        writeRing(1u << ringPos);
        Serial.print(F("Position: "));
        Serial.println(ringPos);
        delay(50);
    }

    if (curAL == LOW && prevAL == HIGH) {
        ringPos = (ringPos + 15) % 16;
        writeRing(1u << ringPos);
        Serial.print(F("Position: "));
        Serial.println(ringPos);
        delay(50);
    }

    prevAH = curAH;
    prevAL = curAL;

    studio.update();
    delay(10);
}
