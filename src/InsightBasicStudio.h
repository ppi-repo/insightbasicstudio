#pragma once

#include <InsightBasicLabs.h>
#include <InsightBasicBoard.h>

class InsightBasicStudio {
public:
    void begin(uint32_t baudRate = 115200);

    // ── Experiment management (InsightBasicLabs) ───────────────────────────
    bool initExperiment(std::initializer_list<Experiment> experiments);
    bool initExperiment(const ExperimentRegistry& registry);
    void printExperimentPins(Experiment experiment);

    // ── Telemetry & commands (InsightBasicBoard) ───────────────────────────
    void registerPayload(PayloadProvider provider, uint32_t intervalMs = 0);
    void registerCommandHandler(CommandHandler handler);
    void enableSerialLogging();
    void send(const PayloadField* fields, uint8_t count, const char* cmd = "DATA");
    void update();

    static bool isPinReserved(uint8_t pin);

private:
    InsightBasicLabs _labs;
    InsightBasicBoard _comms;
};

extern InsightBasicStudio studio;
