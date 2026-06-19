// Ultrasonic_BarLED_RGB_Buzzer_Studio — Full proximity feedback system.
// Experiments listed: { US_EXPT, BAR_EXPT, RGB_EXPT, BUZ_EXPT }
//
// IMPORTANT — Hardware pin conflict:
//   BAR_EXPT uses the 74HC595 shift-register bus (D10=latch, D11=data, D13=clock).
//   RGB_EXPT uses D10 (green) and D11 (blue) as independent PWM outputs.
//   These are the SAME physical pins, so initExperiment() will detect a conflict
//   and return false — no pins will be configured.
//
// This sketch demonstrates the conflict-detection output and halts cleanly.
// To run on current hardware, use US_EXPT + BAR_EXPT + BUZ_EXPT instead
// (see Ultrasonic_BarLED_Buzzer_Studio example).
//
// Telemetry keys (if conflict is resolved on future hardware):
//   DST — distance in cm (int16)

#include <InsightBasicStudio.h>

static int16_t lastDist = 0;

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
    *count = 1;
}

void setup() {
    studio.begin();

    bool ok = studio.initExperiment({ US_EXPT, BAR_EXPT, RGB_EXPT, BUZ_EXPT });
    if (!ok) {
        Serial.println(F("Conflict: BAR_EXPT and RGB_EXPT share D10/D11."));
        Serial.println(F("See sketch header for details. Halting."));
        while (true) { studio.update(); }
    }

    studio.registerPayload(buildPayload, 500);
}

void loop() {
    long cm      = measureCm();
    lastDist     = (int16_t)cm;
    long clamped = constrain(cm, 5L, 50L);

    uint8_t bars = (uint8_t)map(clamped, 5, 50, 8, 0);
    setBar(bars);

    uint8_t r, g;
    if (cm >= 50) {
        r = 0;   g = 255;
    } else if (cm >= 20) {
        r = (uint8_t)map(cm, 20, 50,  255, 0);
        g = (uint8_t)map(cm, 20, 50,  128, 255);
    } else {
        r = 255; g = 0;
    }
    analogWrite(IB::RGB_R, r);
    analogWrite(IB::RGB_G, g);
    analogWrite(IB::RGB_B, 0);

    if (cm <= 5) {
        tone(IB::BUZZER, 1500);
        delay(100);
    } else {
        int interval = (int)map(clamped, 5, 50, 80, 800);
        tone(IB::BUZZER, 880, 60);
        delay(interval);
        noTone(IB::BUZZER);
    }

    studio.update();
}
