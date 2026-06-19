// DHT22_TrafficLight_Studio — Temperature-driven traffic light with telemetry.
// Experiments: { DHT_EXPT, TLIGHT_EXPT }
// Requires: adafruit/DHT sensor library
//
// Telemetry keys:
//   TMP — temperature in Celsius (float)
//   HUM — relative humidity (float)
//   TLS — traffic light state: 'R', 'A', or 'G' (char)

#include <InsightBasicStudio.h>
#include <DHT.h>

#define DHT_TYPE  DHT22
#define TEMP_WARM 20.0f
#define TEMP_HOT  28.0f

DHT dht(IB::DHT_SIG, DHT_TYPE);

static float lastTemp = 0;
static float lastHum  = 0;
static char  tlState  = 'G';
static uint32_t lastRead = 0;

static void setTrafficLight(uint8_t pattern) {
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, pattern);
    digitalWrite(IB::SR_LATCH, HIGH);
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"TMP", D(lastTemp)};
    fields[1] = {"HUM", D(lastHum)};
    fields[2] = {"TLS", C(tlState)};
    *count = 3;
}

void setup() {
    studio.begin();
    studio.initExperiment({ DHT_EXPT, TLIGHT_EXPT });
    dht.begin();
    setTrafficLight(0x04);
    studio.registerPayload(buildPayload, 2000);
    Serial.println(F("DHT22 + Traffic Light Studio — temperature states"));
}

void loop() {
    if (millis() - lastRead >= 2000) {
        lastRead = millis();
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t)) lastTemp = t;
        if (!isnan(h)) lastHum  = h;

        char newState;
        if      (lastTemp >= TEMP_HOT)  newState = 'R';
        else if (lastTemp >= TEMP_WARM) newState = 'A';
        else                            newState = 'G';

        if (newState != tlState) {
            tlState = newState;
            switch (tlState) {
                case 'R': setTrafficLight(0x01); break;
                case 'A': setTrafficLight(0x02); break;
                case 'G': setTrafficLight(0x04); break;
            }
        }

        Serial.print(F("Temp: "));
        Serial.print(lastTemp, 1);
        Serial.print(F(" C -> "));
        Serial.println(tlState);
    }

    studio.update();
}
