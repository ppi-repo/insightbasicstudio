// Ultrasonic_BarLED_Studio — Distance bar graph with telemetry.
// Experiments: { US_EXPT, BAR_EXPT }
//
// Telemetry keys:
//   DST — distance in cm (int16)
//   LVL — bar level 0-8 (int16)

#include <InsightBasicStudio.h>

static int16_t lastDist = 0;
static int16_t lastBars = 0;

static long measureCm() {
    digitalWrite(IB::US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(IB::US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(IB::US_TRIG, LOW);
    long dur = pulseIn(IB::US_ECHO, HIGH, 30000UL);
    return (dur == 0) ? 999L : dur / 58L;
}

static void setBar(uint8_t bars) {
    uint8_t pattern = (bars >= 8) ? 0xFF : (uint8_t)((1u << bars) - 1u);
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, pattern);
    digitalWrite(IB::SR_LATCH, HIGH);
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"DST", I(lastDist)};
    fields[1] = {"LVL", I(lastBars)};
    *count = 2;
}

void setup() {
    studio.begin();
    studio.initExperiment({ US_EXPT, BAR_EXPT });
    setBar(0);
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("Ultrasonic + Bar LED Studio — distance bar graph"));
}

void loop() {
    long cm = measureCm();
    lastDist = (int16_t)cm;
    long clamped = constrain(cm, 5L, 50L);
    lastBars = (int16_t)map(clamped, 5, 50, 8, 0);
    setBar((uint8_t)lastBars);

    studio.update();
    delay(100);
}
