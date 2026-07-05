#include "eae_firmware/FirmwareController.h"

#include <algorithm>

namespace eae::firmware {

namespace {

double signalOrDefault(const std::optional<CanFrame>& frame, const std::string& signal, double fallback) {
    if (!frame) {
        return fallback;
    }
    const auto found = frame->signals.find(signal);
    if (found == frame->signals.end()) {
        return fallback;
    }
    return found->second;
}

double clampPercent(double value) {
    return std::clamp(value, 0.0, 100.0);
}

}  // namespace

FirmwareController::FirmwareController(FirmwareConfig config)
    : config_(config), pid_(PidGains{}, 0.0, 100.0) {}

ControlOutputs FirmwareController::step(CanBus& bus, double dtSeconds) {
    const auto temperatureFrame = bus.latestReceived(CanIds::TemperatureStatus);
    const auto coolantFrame = bus.latestReceived(CanIds::CoolantStatus);
    const auto driverFrame = bus.latestReceived(CanIds::DriverCommand);
    const auto pumpStatusFrame = bus.latestReceived(CanIds::PumpStatus);

    const double temperatureC = signalOrDefault(temperatureFrame, "coolant_temp_c", config_.targetTempC);
    const bool temperatureValid = signalOrDefault(temperatureFrame, "sensor_valid", 0.0) > 0.5;
    const bool coolantLevelOk = signalOrDefault(coolantFrame, "coolant_level_ok", 0.0) > 0.5;
    const bool ignitionOn = signalOrDefault(driverFrame, "ignition_on", 0.0) > 0.5;
    const bool pumpFault = signalOrDefault(pumpStatusFrame, "pump_fault", 0.0) > 0.5;

    const CoolingState state = stateMachine_.update({
        ignitionOn,
        coolantLevelOk,
        temperatureValid,
        pumpFault,
        temperatureC,
        config_.targetTempC,
    });

    ControlOutputs outputs;
    outputs.state = state;

    if (state == CoolingState::Off || state == CoolingState::Fault) {
        pid_.reset();
        outputs.derateRequested = state == CoolingState::Fault;
    } else {
        const double coolingError = std::max(0.0, temperatureC - config_.targetTempC);
        const double demand = pid_.update(coolingError, dtSeconds);
        const double basePump = temperatureC >= config_.targetTempC - 5.0 ? 25.0 : 10.0;

        outputs.pumpPercent = clampPercent(std::max(basePump, demand));
        outputs.fanPercent = clampPercent(std::max(0.0, (demand - 35.0) * 1.35));
        outputs.derateRequested = state == CoolingState::Warning;
    }

    bus.transmit({
        CanIds::CoolingCommand,
        "CoolingCommand",
        {
            {"pump_percent", outputs.pumpPercent},
            {"fan_percent", outputs.fanPercent},
            {"derate_request", outputs.derateRequested ? 1.0 : 0.0},
        },
    });

    bus.transmit({
        CanIds::DiagnosticStatus,
        "DiagnosticStatus",
        {
            {"state", static_cast<double>(outputs.state)},
            {"temperature_c", temperatureC},
            {"target_temp_c", config_.targetTempC},
        },
    });

    return outputs;
}

void FirmwareController::reset() {
    pid_.reset();
    stateMachine_ = CoolingStateMachine{};
}

}  // namespace eae::firmware
