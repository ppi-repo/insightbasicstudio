// DHT22_TrafficLight_Buzzer_Studio — Temperature light + audible alerts with telemetry.
// Experiments: { DHT_EXPT, TLIGHT_EXPT, BUZ_EXPT }
// Requires: adafruit/DHT sensor library
//
// Telemetry keys:
//   TMP — temperature in Celsius (float)
//   HUM — relative humidity (float)
//   TLS — traffic light state: 'R', 'A', or 'G' (char)
//   BZS — buzzer active (bool)

#include <InsightBasicStudio.h>
#include <DHT.h>

#define DHT_TYPE  DHT22
#define TEMP_WARM 20.0f
#define TEMP_HOT  28.0f

DHT dht(IB::DHT_SIG, DHT_TYPE);

static float    lastTemp  = 0;
static float    lastHum   = 0;
static char     tlState   = 'G';
static char     prevState = '\0';
static bool     buzActive = false;
static uint32_t lastRead  = 0;

static void setTrafficLight(uint8_t pattern) {
    digitalWrite(IB::SR_LATCH, LOW);
    shiftOut(IB::SR_DATA, IB::SR_CLK, MSBFIRST, pattern);
    digitalWrite(IB::SR_LATCH, HIGH);
}

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"TMP", D(lastTemp)};
    fields[1] = {"HUM", D(lastHum)};
    fields[2] = {"TLS", C(tlState)};
    fields[3] = {"BZS", B(buzActive)};
    *count = 4;
}

void setup() {
    studio.begin();
    studio.initExperiment({ DHT_EXPT, TLIGHT_EXPT, BUZ_EXPT });
    dht.begin();
    setTrafficLight(0x04);
    studio.registerPayload(buildPayload, 2000);
    Serial.println(F("DHT22 + Traffic Light + Buzzer Studio"));
}

void loop() {
    if (millis() - lastRead >= 2000) {
        lastRead = millis();
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t)) lastTemp = t;
        if (!isnan(h)) lastHum  = h;

        if      (lastTemp >= TEMP_HOT)  tlState = 'R';
        else if (lastTemp >= TEMP_WARM) tlState = 'A';
        else                            tlState = 'G';

        if (tlState != prevState) {
            switch (tlState) {
                case 'R': setTrafficLight(0x01); break;
                case 'A': setTrafficLight(0x02); break;
                case 'G': setTrafficLight(0x04); break;
            }
            tone(IB::BUZZER, 1000, 100);
            prevState = tlState;

            Serial.print(F("Temp: "));
            Serial.print(lastTemp, 1);
            Serial.print(F(" C -> "));
            Serial.println(tlState);
        }

        if (tlState == 'R') {
            tone(IB::BUZZER, 1500);
            buzActive = true;
        } else {
            noTone(IB::BUZZER);
            buzActive = false;
        }
    }

    studio.update();
}
