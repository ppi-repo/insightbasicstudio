# InsightBasicStudio

A unified Arduino library for the **Insight Basic Experimental Board** that merges experiment management ([InsightBasicLabs](https://github.com/ppi-repo/insightbasiclabs)) with UART telemetry and dashboard command dispatch ([InsightBasicBoard](https://github.com/ppi-repo/insightbasic)) into a single API.

One `#include`, one global instance, one `begin()` call.

---

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Installation](#installation)
  - [Arduino IDE (ZIP)](#arduino-ide-zip)
  - [Arduino IDE (Manual)](#arduino-ide-manual)
  - [PlatformIO](#platformio)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
  - [studio.begin()](#studiobegin)
  - [studio.initExperiment()](#studioinitexperiment)
  - [studio.printExperimentPins()](#studioprintexperimentpins)
  - [studio.registerPayload()](#studioregisterpayload)
  - [studio.registerCommandHandler()](#studioregistercommandhandler)
  - [studio.send()](#studiosend)
  - [studio.update()](#studioupdate)
  - [ExperimentRegistry](#experimentregistry)
  - [Experiment Enum](#experiment-enum)
  - [IB:: Pin Constants](#ib-pin-constants)
  - [Payload Helpers](#payload-helpers)
- [Telemetry Key Codebook](#telemetry-key-codebook)
- [Dashboard Command Keys](#dashboard-command-keys)
- [Examples](#examples)
  - [Input Experiments (Sensor to Telemetry)](#input-experiments-sensor-to-telemetry)
  - [Output Experiments (Dashboard to Actuator)](#output-experiments-dashboard-to-actuator)
  - [Combined Sketches](#combined-sketches)
- [Hardware](#hardware)
  - [Pin Map](#pin-map)
  - [Known Pin Conflicts](#known-pin-conflicts)
  - [Reserved UART Pins](#reserved-uart-pins)
- [Dependencies](#dependencies)
- [Compatibility](#compatibility)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- **Single unified API** -- `studio.initExperiment()` for pin management, `studio.registerPayload()` for telemetry, `studio.registerCommandHandler()` for dashboard control.
- **One-call initialisation** -- `studio.begin()` sets up both pin management (all digital pins to INPUT, hardware Serial) and UART communication (SoftwareSerial on D4/D5).
- **Conflict detection** -- inherited from InsightBasicLabs; checks pin conflicts before configuring any hardware.
- **Auto telemetry** -- register a payload provider callback and the library transmits sensor data as chunked JSON at your chosen interval.
- **Dashboard commands** -- receive `KEY:VALUE` commands from the web dashboard to control output experiments (RGB colour, buzzer frequency, traffic light state, etc.).
- **Named pin constants** -- the `IB::` namespace means sketches never contain bare numbers.
- **Flash-friendly** -- all diagnostic strings stored in program memory with `F()`.

---

## Architecture

```
InsightBasicStudio
    |
    +-- InsightBasicLabs    (experiment pin management)
    |     - initExperiment(), printExperimentPins()
    |     - Experiment enum, IB:: pin constants
    |     - Pin conflict detection
    |
    +-- InsightBasicBoard   (UART telemetry & commands)
          - registerPayload(), registerCommandHandler()
          - update(), send()
          - Chunked JSON framing protocol
          - PING / RESET / GET system commands
```

InsightBasicStudio composes both libraries and delegates to each. The `studio` global instance wraps everything.

---

## Installation

### Arduino IDE (ZIP)

1. Install the dependencies first:
   - Download [InsightBasicLabs](https://github.com/ppi-repo/insightbasiclabs) as ZIP and install via **Sketch > Include Library > Add .ZIP Library...**
   - Download [InsightBasicBoard](https://github.com/ppi-repo/insightbasic) as ZIP and install the same way.
2. Download this repository as ZIP and install via the same menu.
3. Examples appear under **File > Examples > InsightBasicStudio**.

### Arduino IDE (Manual)

Copy all three library folders into your Arduino libraries directory:

```
Arduino/
  libraries/
    InsightBasicLabs/
      src/
        InsightBasicLabs.h
        InsightBasicLabs.cpp
        ibl_initializer_list.h
      library.properties
    InsightBasicBoard/
      src/
        InsightBasicBoard.h
        InsightBasicBoard.cpp
      library.properties
    InsightBasicStudio/
      src/
        InsightBasicStudio.h
        InsightBasicStudio.cpp
      examples/
        DHT22_Studio/
          DHT22_Studio.ino
        ...
      library.properties
```

### PlatformIO

**Option A -- Git dependencies:**

```ini
[env:your_board]
platform = atmelavr
board = uno
framework = arduino
lib_deps =
    https://github.com/ppi-repo/insightbasicstudio.git
    https://github.com/ppi-repo/insightbasiclabs.git
    https://github.com/ppi-repo/insightbasic.git
    adafruit/DHT sensor library
    adafruit/Adafruit Unified Sensor
```

**Option B -- Local project libraries:**

Place all three library folders inside your project's `lib/` directory.

---

## Quick Start

```cpp
#include <InsightBasicStudio.h>
#include <DHT.h>

DHT dht(IB::DHT_SIG, DHT22);

float lastTemp = 0;
float lastHum  = 0;

// Called by the library to build telemetry payload
void buildPayload(PayloadField* fields, uint8_t* count) {
    fields[0] = {"TMP", D(lastTemp)};  // float -> {"TMP":23.50}
    fields[1] = {"HUM", D(lastHum)};   // float -> {"HUM":61.20}
    *count = 2;
}

void setup() {
    studio.begin();                              // pins + Serial + SoftwareSerial
    studio.initExperiment({ DHT_EXPT });         // conflict check + pin config
    dht.begin();
    studio.registerPayload(buildPayload, 2000);  // auto-transmit every 2s
}

void loop() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) lastTemp = t;
    if (!isnan(h)) lastHum  = h;

    studio.update();  // handles telemetry + inbound commands
}
```

---

## API Reference

### `studio.begin()`

```cpp
void begin(uint32_t baudRate = 115200);
```

Initialises both subsystems:
1. **InsightBasicLabs** -- resets all digital pins D2-D19 to INPUT, starts hardware Serial at `baudRate`.
2. **InsightBasicBoard** -- starts SoftwareSerial on D4 (TX) / D5 (RX) at `baudRate`.

---

### `studio.initExperiment()`

```cpp
bool initExperiment(std::initializer_list<Experiment> experiments);
bool initExperiment(const ExperimentRegistry& registry);
```

Initialises experiments with automatic pin conflict detection. Returns `false` (without changing any pin) if a conflict is found. See [InsightBasicLabs README](https://github.com/ppi-repo/insightbasiclabs) for full details.

---

### `studio.printExperimentPins()`

```cpp
void printExperimentPins(Experiment experiment);
```

Prints pin mapping for one experiment to hardware Serial (flash-stored strings).

---

### `studio.registerPayload()`

```cpp
void registerPayload(PayloadProvider provider, uint32_t intervalMs = 0);
```

Register a callback that builds the telemetry payload. If `intervalMs > 0`, the library auto-transmits at that interval. The payload is also sent on demand when the comm board sends a `GET` command.

**PayloadProvider signature:**
```cpp
void myPayload(PayloadField* fields, uint8_t* count);
```

---

### `studio.registerCommandHandler()`

```cpp
void registerCommandHandler(CommandHandler handler);
```

Register a callback for `KEY:VALUE` commands from the web dashboard. System commands (`PING`, `RESET`, `GET`) are handled internally and never forwarded.

**CommandHandler signature:**
```cpp
void onCommand(const char* key, const char* value);
```

---

### `studio.enableSerialLogging()`

```cpp
void enableSerialLogging();
```

Enables mirroring of transmitted JSON packets to the hardware Serial (USB) port for debugging. Initializes `Serial` at the same baud rate used in `begin()`. Call once in `setup()` after `begin()`.

---

### `studio.send()`

```cpp
void send(const PayloadField* fields, uint8_t count, const char* cmd = "DATA");
```

Ad-hoc one-shot payload transmission. Prefer `registerPayload()` for periodic telemetry.

---

### `studio.update()`

```cpp
void update();
```

Call every `loop()` iteration. Non-blocking. Handles:
- Periodic auto-transmission (if `intervalMs > 0`)
- Inbound command polling and dispatch

---

### `ExperimentRegistry`

For runtime-conditional experiment setup:

```cpp
ExperimentRegistry registry;
registry.add(DHT_EXPT);
if (condition) registry.add(RGB_EXPT);
studio.initExperiment(registry);
```

---

### Experiment Enum

```cpp
enum Experiment : uint8_t {
    DHT_EXPT,    LDR_EXPT,    POT_EXPT,    BTN_EXPT,    US_EXPT,
    RGB_EXPT,    BUZ_EXPT,    RING_EXPT,   HRT_EXPT,    TLIGHT_EXPT,
    BAR_EXPT,    EXPERIMENT_COUNT
};
```

---

### `IB::` Pin Constants

| Constant | Pin | Experiment |
|----------|-----|------------|
| `IB::DHT_SIG` | A0 | DHT22 signal |
| `IB::LDR_SIG` | A7 | LDR signal |
| `IB::POT_SIG` | A6 | Potentiometer wiper |
| `IB::BTN_AH` | D2 | Active-HIGH button |
| `IB::BTN_AL` | D3 | Active-LOW button |
| `IB::US_TRIG` | D7 | HC-SR04 trigger |
| `IB::US_ECHO` | D8 | HC-SR04 echo |
| `IB::RGB_R` | D9 | RGB red (PWM) |
| `IB::RGB_G` | D10 | RGB green (PWM) |
| `IB::RGB_B` | D11 | RGB blue (PWM) |
| `IB::BUZZER` | D6 | Buzzer signal |
| `IB::SR_DATA` | D11 | Shift-register data |
| `IB::SR_CLK` | D13 | Shift-register clock |
| `IB::SR_LATCH` | D10 | Shift-register latch |
| `IB::HEART` | D13 | Heart LED signal |

---

### Payload Helpers

Use these to construct `PayloadVal` values in payload providers:

| Helper | Type | Example |
|--------|------|---------|
| `D(v)` | float | `D(23.5)` -> `23.50` |
| `I(v)` | int16_t | `I(142)` -> `142` |
| `B(v)` | bool | `B(true)` -> `true` |
| `C(v)` | char | `C('A')` -> `"A"` |
| `S(v)` | const char* | `S("OK")` -> `"OK"` |

---

## Telemetry Key Codebook

Standard 3-character keys used across Studio examples:

| Key | Type | Description |
|-----|------|-------------|
| `TMP` | float | Temperature in Celsius |
| `HUM` | float | Relative humidity % |
| `LGT` | int16 | Light level (ADC 0-1023) |
| `POT` | int16 | Potentiometer position (ADC 0-1023) |
| `BAH` | bool | Active-HIGH button pressed |
| `BAL` | bool | Active-LOW button pressed |
| `DST` | int16 | Distance in cm |
| `RED` | int16 | RGB red channel 0-255 |
| `GRN` | int16 | RGB green channel 0-255 |
| `BLU` | int16 | RGB blue channel 0-255 |
| `FRQ` | int16 | Buzzer frequency in Hz |
| `BZS` | bool | Buzzer active |
| `POS` | int16 | LED Ring position 0-15 |
| `PTN` | int16 | Animation pattern index |
| `BPM` | int16 | Heart rate in BPM |
| `HUE` | int16 | HSV hue 0-255 |
| `TLS` | char | Traffic light state: R/A/G |
| `LVL` | int16 | Bar LED level 0-8 |
| `FRZ` | bool | Animation frozen |

---

## Dashboard Command Keys

Standard commands accepted by Studio output examples:

| Key | Value format | Description |
|-----|-------------|-------------|
| `RGB` | `R,G,B` or `AUTO` | Set RGB colour (e.g. `RGB:255,0,128`) or resume auto-mapping |
| `BUZ` | `0` or `freq` | Silence buzzer or play tone at freq Hz |
| `RNG` | `0-15` | Set LED Ring position |
| `HRT` | `30-200` | Set heart rate in BPM |
| `TLT` | `R`, `A`, or `G` | Set traffic light state |
| `BAR` | `0-8` | Set bar LED level |

---

## Examples

All examples live in `examples/` and open from **File > Examples > InsightBasicStudio** in the Arduino IDE.

### Input Experiments (Sensor to Telemetry)

| Sketch | Experiment | Telemetry keys |
|--------|------------|---------------|
| [DHT22_Studio](examples/DHT22_Studio/DHT22_Studio.ino) | `{DHT_EXPT}` | TMP, HUM |
| [LDR_Studio](examples/LDR_Studio/LDR_Studio.ino) | `{LDR_EXPT}` | LGT |
| [Potentiometer_Studio](examples/Potentiometer_Studio/Potentiometer_Studio.ino) | `{POT_EXPT}` | POT |
| [Buttons_Studio](examples/Buttons_Studio/Buttons_Studio.ino) | `{BTN_EXPT}` | BAH, BAL |
| [Ultrasonic_Studio](examples/Ultrasonic_Studio/Ultrasonic_Studio.ino) | `{US_EXPT}` | DST |

### Output Experiments (Dashboard to Actuator)

| Sketch | Experiment | Command key | Telemetry keys |
|--------|------------|-------------|---------------|
| [RGB_Studio](examples/RGB_Studio/RGB_Studio.ino) | `{RGB_EXPT}` | RGB | RED, GRN, BLU |
| [Buzzer_Studio](examples/Buzzer_Studio/Buzzer_Studio.ino) | `{BUZ_EXPT}` | BUZ | BZS, FRQ |
| [LEDRing_Studio](examples/LEDRing_Studio/LEDRing_Studio.ino) | `{RING_EXPT}` | RNG | POS |
| [Heart_Studio](examples/Heart_Studio/Heart_Studio.ino) | `{HRT_EXPT}` | HRT | BPM |
| [TrafficLight_Studio](examples/TrafficLight_Studio/TrafficLight_Studio.ino) | `{TLIGHT_EXPT}` | TLT | TLS |
| [BarLED_Studio](examples/BarLED_Studio/BarLED_Studio.ino) | `{BAR_EXPT}` | BAR | LVL |

### Combined Sketches

#### 2 Experiments

| Sketch | Experiments | Telemetry keys |
|--------|------------|---------------|
| [LDR_RGB_Studio](examples/LDR_RGB_Studio/LDR_RGB_Studio.ino) | `{LDR_EXPT, RGB_EXPT}` | LGT, RED, GRN, BLU |
| [Potentiometer_Buzzer_Studio](examples/Potentiometer_Buzzer_Studio/Potentiometer_Buzzer_Studio.ino) | `{POT_EXPT, BUZ_EXPT}` | POT, FRQ |
| [Ultrasonic_BarLED_Studio](examples/Ultrasonic_BarLED_Studio/Ultrasonic_BarLED_Studio.ino) | `{US_EXPT, BAR_EXPT}` | DST, LVL |
| [DHT22_TrafficLight_Studio](examples/DHT22_TrafficLight_Studio/DHT22_TrafficLight_Studio.ino) | `{DHT_EXPT, TLIGHT_EXPT}` | TMP, HUM, TLS |
| [Buttons_LEDRing_Studio](examples/Buttons_LEDRing_Studio/Buttons_LEDRing_Studio.ino) | `{BTN_EXPT, RING_EXPT}` | POS, BAH, BAL |
| [Potentiometer_Heart_Studio](examples/Potentiometer_Heart_Studio/Potentiometer_Heart_Studio.ino) | `{POT_EXPT, HRT_EXPT}` | POT, BPM |

#### 3 Experiments

| Sketch | Experiments | Telemetry keys |
|--------|------------|---------------|
| [LDR_RGB_Buzzer_Studio](examples/LDR_RGB_Buzzer_Studio/LDR_RGB_Buzzer_Studio.ino) | `{LDR_EXPT, RGB_EXPT, BUZ_EXPT}` | LGT, RED, BLU, BZS |
| [Ultrasonic_BarLED_Buzzer_Studio](examples/Ultrasonic_BarLED_Buzzer_Studio/Ultrasonic_BarLED_Buzzer_Studio.ino) | `{US_EXPT, BAR_EXPT, BUZ_EXPT}` | DST, LVL, BZS |
| [Potentiometer_RGB_Heart_Studio](examples/Potentiometer_RGB_Heart_Studio/Potentiometer_RGB_Heart_Studio.ino) | `{POT_EXPT, RGB_EXPT, HRT_EXPT}` | POT, HUE, BPM |
| [DHT22_TrafficLight_Buzzer_Studio](examples/DHT22_TrafficLight_Buzzer_Studio/DHT22_TrafficLight_Buzzer_Studio.ino) | `{DHT_EXPT, TLIGHT_EXPT, BUZ_EXPT}` | TMP, HUM, TLS, BZS |

#### 4 Experiments

| Sketch | Experiments | Telemetry keys |
|--------|------------|---------------|
| [LDR_Buttons_Ring_Buzzer_Studio](examples/LDR_Buttons_Ring_Buzzer_Studio/LDR_Buttons_Ring_Buzzer_Studio.ino) | `{LDR_EXPT, BTN_EXPT, RING_EXPT, BUZ_EXPT}` | LGT, POS, PTN, FRZ |
| [Ultrasonic_BarLED_RGB_Buzzer_Studio](examples/Ultrasonic_BarLED_RGB_Buzzer_Studio/Ultrasonic_BarLED_RGB_Buzzer_Studio.ino) | `{US_EXPT, BAR_EXPT, RGB_EXPT, BUZ_EXPT}` | DST (pin conflict demo) |

---

## Hardware

### Pin Map

#### Input Experiments

| # | Experiment | Pin(s) |
|---|------------|--------|
| 1 | DHT22 | Signal: A0 |
| 2 | LDR | Signal: A7 *(analog-only)* |
| 3 | Potentiometer | Signal: A6 *(analog-only)* |
| 4 | Buttons | Active-HIGH: D2 / Active-LOW: D3 |
| 5 | Ultrasonic (HC-SR04) | Trigger: D7 / Echo: D8 |

#### Output Experiments

| # | Experiment | Pin(s) |
|---|------------|--------|
| 6 | RGB LED | RED: D9 / GREEN: D10 / BLUE: D11 |
| 7 | Buzzer | Signal: D6 |
| 8 | LED Ring | Data: D11 / Clock: D13 / Latch: D10 |
| 9 | Heart LED | Signal: D13 |
| 10 | Traffic Light | *(shift-register bus -- see LED Ring)* |
| 11 | Bar LED | *(shift-register bus -- see LED Ring)* |

### Known Pin Conflicts

| Experiment A | Experiment B | Shared Pins |
|---|---|---|
| RGB LED | LED Ring / Traffic Light / Bar LED | D10, D11 |
| Heart LED | LED Ring / Traffic Light / Bar LED | D13 |

### Reserved UART Pins

InsightBasicBoard reserves **D4** (TX) and **D5** (RX) for SoftwareSerial communication with the comm board. These pins must not be used for experiments or other purposes while communication is active. Use `InsightBasicStudio::isPinReserved(pin)` to check at runtime.

---

## Dependencies

| Library | Required by | Source |
|---------|------------|--------|
| [InsightBasicLabs](https://github.com/ppi-repo/insightbasiclabs) | All sketches | GitHub |
| [InsightBasicBoard](https://github.com/ppi-repo/insightbasic) | All sketches | GitHub |
| [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library) | DHT22 sketches | Arduino Library Manager |
| [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor) | Transitive dep of DHT | Arduino Library Manager |

---

## Compatibility

| Attribute | Value |
|-----------|-------|
| Microcontroller | ATmega328P |
| Boards tested | Arduino Uno, Arduino Nano, bare ATmega328P at 16 MHz |
| Framework | Arduino |
| Build system | Arduino IDE 1.8+, Arduino IDE 2.x, PlatformIO |
| C++ standard | C++11 |
| UART | SoftwareSerial on D4 (TX) / D5 (RX) |

---

## Contributing

1. Fork this repository.
2. Create a feature branch (`git checkout -b feature/my-change`).
3. Commit your changes.
4. Push to the branch and open a Pull Request.

Please ensure all examples compile cleanly for the ATmega328P before submitting.

---

## License

MIT License -- see [LICENSE](LICENSE) for full text.

---

*Insight Basic Experimental Board -- designed by Erictronics.*
