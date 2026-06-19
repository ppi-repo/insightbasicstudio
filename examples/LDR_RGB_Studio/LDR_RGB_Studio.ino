// LDR_RGB_Studio — Light-to-colour mapping with telemetry and dashboard override.
// Experiments: { LDR_EXPT, RGB_EXPT }
//
// Telemetry keys:
//   LGT — raw ADC light level (int16)
//   RED — current red channel (int16)
//   GRN — current green channel (int16)
//   BLU — current blue channel (int16)
//
// Dashboard command:
//   RGB:R,G,B — override auto-mapping (e.g. "RGB:255,0,128")
//   RGB:AUTO  — resume auto-mapping from LDR

#include <InsightBasicStudio.h>

static int16_t lastLight = 0;
static uint8_t curR = 0, curG = 0, curB = 0;
static bool    manualMode = false;

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
    fields[0] = {"LGT", I(lastLight)};
    fields[1] = {"RED", I((int16_t)curR)};
    fields[2] = {"GRN", I((int16_t)curG)};
    fields[3] = {"BLU", I((int16_t)curB)};
    *count = 4;
}

void onCommand(const char* key, const char* value) {
    if (strcmp(key, "RGB") == 0) {
        if (strncmp(value, "AUTO", 4) == 0) {
            manualMode = false;
            Serial.println(F("RGB auto-mapping resumed"));
        } else {
            parseRGB(value, curR, curG, curB);
            manualMode = true;
            analogWrite(IB::RGB_R, curR);
            analogWrite(IB::RGB_G, curG);
            analogWrite(IB::RGB_B, curB);
        }
    }
}

void setup() {
    studio.begin();
    studio.initExperiment({ LDR_EXPT, RGB_EXPT });
    studio.registerPayload(buildPayload, 500);
    studio.registerCommandHandler(onCommand);
    Serial.println(F("LDR + RGB Studio — light-to-colour with telemetry"));
}

void loop() {
    lastLight = (int16_t)analogRead(IB::LDR_SIG);

    if (!manualMode) {
        curR = map(lastLight, 0, 1023, 0, 255);
        curB = map(lastLight, 0, 1023, 255, 0);
        curG = 0;
        analogWrite(IB::RGB_R, curR);
        analogWrite(IB::RGB_G, curG);
        analogWrite(IB::RGB_B, curB);
    }

    studio.update();
    delay(50);
}
