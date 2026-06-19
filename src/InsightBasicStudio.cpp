#include "InsightBasicStudio.h"

void InsightBasicStudio::begin(uint32_t baudRate) {
    _labs.begin(baudRate);
    _comms.begin(baudRate);
}

bool InsightBasicStudio::initExperiment(std::initializer_list<Experiment> experiments) {
    return _labs.initExperiment(experiments);
}

bool InsightBasicStudio::initExperiment(const ExperimentRegistry& registry) {
    return _labs.initExperiment(registry);
}

void InsightBasicStudio::printExperimentPins(Experiment experiment) {
    _labs.printExperimentPins(experiment);
}

void InsightBasicStudio::registerPayload(PayloadProvider provider, uint32_t intervalMs) {
    _comms.registerPayload(provider, intervalMs);
}

void InsightBasicStudio::registerCommandHandler(CommandHandler handler) {
    _comms.registerCommandHandler(handler);
}

void InsightBasicStudio::send(const PayloadField* fields, uint8_t count, const char* cmd) {
    _comms.send(fields, count, cmd);
}

void InsightBasicStudio::update() {
    _comms.update();
}

bool InsightBasicStudio::isPinReserved(uint8_t pin) {
    return InsightBasicBoard::isPinReserved(pin);
}

InsightBasicStudio studio;
