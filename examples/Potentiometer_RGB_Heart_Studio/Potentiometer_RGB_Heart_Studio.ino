// Potentiometer_RGB_Heart_Studio — Pot maps to hue and BPM with telemetry.
// Experiments: { POT_EXPT, RGB_EXPT, HRT_EXPT }
//
// Telemetry keys:
//   POT — raw ADC pot position (int16)
//   HUE — current hue 0-255 (int16)
//   BPM — current heart rate (int16)

#include <InsightBasicStudio.h>

#define BPM_MIN    40
#define BPM_MAX   180
#define LUB_DUR    80
#define INTER_DUR  80
#define DUB_DUR    80
#define PULSE_TOTAL (LUB_DUR + INTER_DUR + DUB_DUR)

static int16_t lastPot = 0;
static int16_t lastHue = 0;
static int16_t lastBPM = 0;

static void hsvToRgb(uint8_t h, uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t region    = h / 43;
    uint8_t remainder = (h - region * 43) * 6;
    uint8_t q = 255 - remainder;
    uint8_t t = remainder;

    switch (region) {
        case 0:  r = 255; g = t;   b = 0;   break;
        case 1:  r = q;   g = 255; b = 0;   break;
        case 2:  r = 0;   g = 255; b = t;   break;
        case 3:  r = 0;   g = q;   b = 255; break;
        case 4:  r = t;   g = 0;   b = 255; break;
        default: r = 255; g = 0;   b = q;   break;
    }
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"POT", I(lastPot)};
    fields[1] = {"HUE", I(lastHue)};
    fields[2] = {"BPM", I(lastBPM)};
    *count = 3;
}

void setup() {
    studio.begin();
    studio.initExperiment({ POT_EXPT, RGB_EXPT, HRT_EXPT });
    studio.registerPayload(buildPayload, 1000);
    Serial.println(F("Pot + RGB + Heart Studio — hue & BPM from one pot"));
}

void loop() {
    lastPot = (int16_t)analogRead(IB::POT_SIG);

    // Set RGB colour from hue
    lastHue = (int16_t)map(lastPot, 0, 1023, 0, 255);
    uint8_t r, g, b;
    hsvToRgb((uint8_t)lastHue, r, g, b);
    analogWrite(IB::RGB_R, r);
    analogWrite(IB::RGB_G, g);
    analogWrite(IB::RGB_B, b);

    // Lub-dub at pot-controlled BPM
    lastBPM = (int16_t)map(lastPot, 0, 1023, BPM_MIN, BPM_MAX);
    long period = 60000L / lastBPM;

    digitalWrite(IB::HEART, HIGH);
    delay(LUB_DUR);
    digitalWrite(IB::HEART, LOW);
    delay(INTER_DUR);
    digitalWrite(IB::HEART, HIGH);
    delay(DUB_DUR);
    digitalWrite(IB::HEART, LOW);

    long rest = period - PULSE_TOTAL;
    uint32_t restStart = millis();
    while ((long)(millis() - restStart) < rest) {
        studio.update();
        delay(10);
    }
}
