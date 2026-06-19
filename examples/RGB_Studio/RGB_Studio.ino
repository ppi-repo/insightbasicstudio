// RGB_Studio — RGB LED controlled from dashboard with status telemetry.
// Experiment: RGB_EXPT  |  RED: D9, GREEN: D10, BLUE: D11
//
// Telemetry keys:
//   RED — current red channel 0-255 (int16)
//   GRN — current green channel 0-255 (int16)
//   BLU — current blue channel 0-255 (int16)
//
// Dashboard command:
//   RGB:R,G,B  — set colour (e.g. "RGB:255,0,128")

#include <InsightBasicStudio.h>

static uint8_t curR = 0, curG = 0, curB = 0;

static void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    curR = r; curG = g; curB = b;
    analogWrite(IB::RGB_R, r);
    analogWrite(IB::RGB_G, g);
    analogWrite(IB::RGB_B, b);
}

static void parseRGB(const char* csv, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = (uint8_t)atoi(csv);
    const char* p = strchr(csv, ',');
    if (!p) return;
    g = (uint8_t)atoi(p + 1);
    p = strchr(p + 1, ',');
    if (!p) return;
    b = (uint8_t)atoi(p + 1);
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"RED", I((int16_t)curR)};
    fields[1] = {"GRN", I((int16_t)curG)};
    fields[2] = {"BLU", I((int16_t)curB)};
    *count = 3;
}

void onCommand(const char* key, const char* value) {
    if (strcmp(key, "RGB") == 0) {
        uint8_t r = 0, g = 0, b = 0;
        parseRGB(value, r, g, b);
        setRGB(r, g, b);
        Serial.print(F("RGB set: "));
        Serial.print(r); Serial.print(',');
        Serial.print(g); Serial.print(',');
        Serial.println(b);
    }
}

void setup() {
    studio.begin();
    studio.initExperiment({ RGB_EXPT });
    studio.registerPayload(buildPayload, 1000);
    studio.registerCommandHandler(onCommand);
    Serial.println(F("RGB Studio — dashboard colour control"));
}

void loop() {
    studio.update();
}
