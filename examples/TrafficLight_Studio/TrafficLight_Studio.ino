// TrafficLight_Studio — Traffic light controlled from dashboard with telemetry.
// Experiment: TLIGHT_EXPT  |  Data: D11, Clock: D13, Latch: D10
//
// Telemetry keys:
//   TLS — current state: 'R', 'A', or 'G' (char)
//
// Dashboard commands:
//   TLT:R — set red
//   TLT:A — set amber
//   TLT:G — set green

#include <InsightBasicStudio.h>

static char tlState = 'G';

static void setTrafficLight(uint8_t pattern) {
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, pattern);
    digitalWrite(IB::SR_LATCH, HIGH);
}

static void applyState() {
    switch (tlState) {
        case 'R': setTrafficLight(0x01); break;
        case 'A': setTrafficLight(0x02); break;
        case 'G': setTrafficLight(0x04); break;
    }
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"TLS", C(tlState)};
    *count = 1;
}

void onCommand(const char* key, const char* value) {
    if (strcmp(key, "TLT") == 0 && value[0] != '\0') {
        char cmd = value[0];
        if (cmd == 'R' || cmd == 'A' || cmd == 'G') {
            tlState = cmd;
            applyState();
            Serial.print(F("Traffic light: "));
            Serial.println(tlState);
        }
    }
}

void setup() {
    studio.begin();
    studio.initExperiment({ TLIGHT_EXPT });
    applyState();
    studio.registerPayload(buildPayload, 1000);
    studio.registerCommandHandler(onCommand);
    Serial.println(F("Traffic Light Studio — dashboard control"));
}

void loop() {
    studio.update();
}
