// BarLED_Studio — Bar LED controlled from dashboard with level telemetry.
// Experiment: BAR_EXPT  |  Data: D11, Clock: D13, Latch: D10
//
// Telemetry keys:
//   LVL — current bar level 0-8 (int16)
//
// Dashboard commands:
//   BAR:level — set bar level 0-8 (e.g. "BAR:5")

#include <InsightBasicStudio.h>

static int16_t barLevel = 0;

static void setBar(uint8_t bars) {
    uint8_t pattern = (bars >= 8) ? 0xFF : (uint8_t)((1u << bars) - 1u);
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, pattern);
    digitalWrite(IB::SR_LATCH, HIGH);
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"LVL", I(barLevel)};
    *count = 1;
}

void onCommand(const char* key, const char* value) {
    if (strcmp(key, "BAR") == 0) {
        int level = constrain(atoi(value), 0, 8);
        barLevel = (int16_t)level;
        setBar((uint8_t)level);
        Serial.print(F("Bar level: "));
        Serial.println(level);
    }
}

void setup() {
    studio.begin();
    studio.initExperiment({ BAR_EXPT });
    setBar(0);
    studio.registerPayload(buildPayload, 1000);
    studio.registerCommandHandler(onCommand);
    Serial.println(F("Bar LED Studio — dashboard level control"));
}

void loop() {
    studio.update();
}
