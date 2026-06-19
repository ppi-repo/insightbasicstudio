// Ultrasonic_Studio — Distance measurement with telemetry.
// Experiment: US_EXPT  |  Trigger: D7, Echo: D8
//
// Telemetry keys:
//   DST — distance in centimetres (int16), 0 = out of range

#include <InsightBasicStudio.h>

static int16_t lastDist = 0;

static long measureCm() {
    digitalWrite(IB::US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(IB::US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(IB::US_TRIG, LOW);
    long duration = pulseIn(IB::US_ECHO, HIGH, 30000UL);
    if (duration == 0) return 0;
    return duration / 58L;
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"DST", I(lastDist)};
    *count = 1;
}

void setup() {
    studio.begin();
    studio.initExperiment({ US_EXPT });
    studio.registerPayload(buildPayload, 500);
    Serial.println(F("Ultrasonic Studio — Distance telemetry"));
}

void loop() {
    lastDist = (int16_t)measureCm();

    Serial.print(F("Distance: "));
    if (lastDist == 0)
        Serial.println(F("out of range"));
    else {
        Serial.print(lastDist);
        Serial.println(F(" cm"));
    }

    studio.update();
    delay(100);
}
