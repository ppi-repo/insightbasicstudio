// DHT22_Studio — Temperature & humidity with telemetry.
// Experiment: DHT_EXPT  |  Pin: A0 (signal)
// Requires: adafruit/DHT sensor library
//
// Telemetry keys:
//   TMP — temperature in Celsius (float)
//   HUM — relative humidity in percent (float)

#include <InsightBasicStudio.h>
#include <DHT.h>

#define DHT_TYPE DHT22

DHT dht(IB::DHT_SIG, DHT_TYPE);

static float lastTemp = 0;
static float lastHum  = 0;
static uint32_t lastRead = 0;

void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"TMP", D(lastTemp)};
    fields[1] = {"HUM", D(lastHum)};
    *count = 2;
}

void setup() {
    studio.begin();
    studio.initExperiment({ DHT_EXPT });
    dht.begin();
    studio.registerPayload(buildPayload, 2000);
    Serial.println(F("DHT22 Studio — Temperature & Humidity telemetry"));
}

void loop() {
    if (millis() - lastRead >= 2000) {
        lastRead = millis();
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t)) lastTemp = t;
        if (!isnan(h)) lastHum  = h;

        Serial.print(F("Temp: "));
        Serial.print(lastTemp, 1);
        Serial.print(F(" C   Humidity: "));
        Serial.print(lastHum, 1);
        Serial.println(F(" %"));
    }

    studio.update();
}
