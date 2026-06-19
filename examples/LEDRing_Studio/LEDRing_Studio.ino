// LEDRing_Studio — LED Ring controlled from dashboard with position telemetry.
// Experiment: RING_EXPT  |  Data: D11, Clock: D13, Latch: D10
//
// Telemetry keys:
//   POS — current lit LED position 0-15 (int16)
//
// Dashboard commands:
//   RNG:pos — set single LED position 0-15 (e.g. "RNG:7")

#include <InsightBasicStudio.h>

static int16_t ringPos = 0;

static void writeRing(uint16_t pattern) {
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, highByte(pattern));
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, lowByte(pattern));
    digitalWrite(IB::SR_LATCH, HIGH);
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"POS", I(ringPos)};
    *count = 1;
}

void onCommand(const char* key, const char* value) {
    if (strcmp(key, "RNG") == 0) {
        int pos = constrain(atoi(value), 0, 15);
        ringPos = (int16_t)pos;
        writeRing(1u << pos);
        Serial.print(F("Ring position: "));
        Serial.println(pos);
    }
}

void setup() {
    studio.begin();
    studio.initExperiment({ RING_EXPT });
    writeRing(0x0000);
    studio.registerPayload(buildPayload, 1000);
    studio.registerCommandHandler(onCommand);
    Serial.println(F("LED Ring Studio — dashboard position control"));
}

void loop() {
    studio.update();
}
