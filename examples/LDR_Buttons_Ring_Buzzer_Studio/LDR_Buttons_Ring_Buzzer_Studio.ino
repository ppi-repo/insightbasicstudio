// LDR_Buttons_Ring_Buzzer_Studio — Interactive light show with telemetry.
// Experiments: { LDR_EXPT, BTN_EXPT, RING_EXPT, BUZ_EXPT }
//
// LDR    -> continuously controls animation speed
// D2 (active-HIGH) -> cycle to next animation pattern + confirmation beep
// D3 (active-LOW)  -> freeze / unfreeze animation
//
// Telemetry keys:
//   LGT — raw ADC light level (int16)
//   POS — current frame index (int16)
//   PTN — current pattern index (int16)
//   FRZ — frozen state (bool)

#include <InsightBasicStudio.h>

#define NUM_PATTERNS 4

static uint16_t patternSpin(uint8_t frame) {
    return 1u << (frame % 16);
}

static uint16_t patternFill(uint8_t frame) {
    uint8_t f = frame % 32;
    if (f == 0)  return 0x0000;
    if (f <= 16) return (uint16_t)((1UL << f) - 1UL);
    uint8_t drain = 32 - f;
    return (drain == 0) ? 0x0000 : (uint16_t)((1UL << drain) - 1UL);
}

static uint16_t patternAlternate(uint8_t frame) {
    return (frame % 2 == 0) ? 0xAAAAu : 0x5555u;
}

static uint16_t patternComet(uint8_t frame) {
    uint8_t  pos   = frame % 16;
    uint16_t comet = 1u << pos;
    if (pos > 0) comet |= 1u << (pos - 1);
    return comet;
}

static uint16_t getFrame(uint8_t pattern, uint8_t frame) {
    switch (pattern % NUM_PATTERNS) {
        case 0:  return patternSpin(frame);
        case 1:  return patternFill(frame);
        case 2:  return patternAlternate(frame);
        default: return patternComet(frame);
    }
}

static void writeRing(uint16_t pattern) {
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, highByte(pattern));
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, lowByte(pattern));
    digitalWrite(IB::SR_LATCH, HIGH);
}

static uint8_t  currentPattern = 0;
static uint8_t  frameIdx       = 0;
static bool     frozen         = false;
static bool     prevAH         = LOW;
static bool     prevAL         = HIGH;
static int16_t  lastLight      = 0;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"LGT", I(lastLight)};
    fields[1] = {"POS", I((int16_t)frameIdx)};
    fields[2] = {"PTN", I((int16_t)currentPattern)};
    fields[3] = {"FRZ", B(frozen)};
    *count = 4;
}

void setup() {
    studio.begin();
    studio.initExperiment({ LDR_EXPT, BTN_EXPT, RING_EXPT, BUZ_EXPT });
    writeRing(getFrame(0, 0));
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("LDR + Buttons + Ring + Buzzer Studio"));
    Serial.println(F("D2=next pattern  D3=freeze/unfreeze"));
}

void loop() {
    bool curAH = digitalRead(IB::BTN_AH);
    bool curAL = digitalRead(IB::BTN_AL);

    if (curAH == HIGH && prevAH == LOW) {
        currentPattern = (currentPattern + 1) % NUM_PATTERNS;
        frameIdx = 0;
        tone(IB::BUZZER, 880, 100);
        Serial.print(F("Pattern: "));
        Serial.println(currentPattern);
        delay(50);
    }

    if (curAL == LOW && prevAL == HIGH) {
        frozen = !frozen;
        tone(IB::BUZZER, frozen ? 440 : 660, 100);
        Serial.println(frozen ? F("Frozen") : F("Running"));
        delay(50);
    }

    prevAH = curAH;
    prevAL = curAL;

    if (!frozen) {
        writeRing(getFrame(currentPattern, frameIdx));
        frameIdx++;

        lastLight = (int16_t)analogRead(IB::LDR_SIG);
        int frameDelay = map(lastLight, 0, 1023, 500, 30);

        uint32_t start = millis();
        while ((int)(millis() - start) < frameDelay) {
            studio.update();
            delay(10);
        }
    } else {
        studio.update();
        delay(10);
    }
}
