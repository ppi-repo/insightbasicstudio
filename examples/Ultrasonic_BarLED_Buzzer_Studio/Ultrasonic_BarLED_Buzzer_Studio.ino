// Ultrasonic_BarLED_Buzzer_Studio — Proximity alarm with telemetry.
// Experiments: { US_EXPT, BAR_EXPT, BUZ_EXPT }
//
// Telemetry keys:
//   DST — distance in cm (int16)
//   LVL — bar level 0-8 (int16)
//   BZS — buzzer active (bool)

#include <InsightBasicStudio.h>

#define DIST_MAX   50
#define DIST_ALARM  5

static int16_t lastDist = 0;
static int16_t lastBars = 0;
static bool    buzActive = false;

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
    fields[2] = {"BZS", B(buzActive)};
    *count = 3;
}

void setup() {
    studio.begin();
    studio.initExperiment({ US_EXPT, BAR_EXPT, BUZ_EXPT });
    setBar(0);
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("Ultrasonic + Bar LED + Buzzer Studio — proximity alarm"));
}

void loop() {
    long cm = measureCm();
    lastDist = (int16_t)cm;
    long clamped = constrain(cm, (long)DIST_ALARM, (long)DIST_MAX);

    lastBars = (int16_t)map(clamped, DIST_ALARM, DIST_MAX, 8, 0);
    setBar((uint8_t)lastBars);

    if (cm <= DIST_ALARM) {
        tone(IB::BUZZER, 1200);
        buzActive = true;
        delay(100);
    } else {
        int interval = (int)map(clamped, DIST_ALARM, DIST_MAX, 80, 800);
        tone(IB::BUZZER, 880, 60);
        buzActive = true;
        delay(interval);
        noTone(IB::BUZZER);
        buzActive = false;
    }

    studio.update();
}
