#include "eae_firmware/FirmwareController.h"

#include <algorithm>

namespace eae::firmware {

namespace {

// Missing frame or missing signal falls back to a caller-chosen default.
// Callers pass fail-safe values (coolant not OK, sensor invalid, ignition
// off), so a silent bus degrades to OFF/FAULT instead of looking healthy.
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

    // Note the temperature default: if the frame is absent we read "at
    // target" so the PID stays calm, but sensor_valid defaults to invalid,
    // which forces the state machine into FAULT anyway. Boolean signals are
    // encoded as 0.0/1.0 doubles, hence the > 0.5 comparisons.
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
        // Outputs stay at zero and the PID history is dropped, so a restart
        // after a fault begins from a clean controller instead of replaying
        // a stale integral. In FAULT we also ask the inverter to derate.
        pid_.reset();
        outputs.derateRequested = state == CoolingState::Fault;
    } else {
        // Only above-target error drives the PID; cooling cannot push the
        // temperature below target, so negative error is clipped to zero.
        const double coolingError = std::max(0.0, temperatureC - config_.targetTempC);
        const double demand = pid_.update(coolingError, dtSeconds);

        // Keep a minimum flow at all times so the sensor sees moving coolant,
        // and raise that floor once we get within 5 C of the target.
        const double basePump = temperatureC >= config_.targetTempC - 5.0 ? 25.0 : 10.0;

        outputs.pumpPercent = clampPercent(std::max(basePump, demand));
        // The fan is the second stage: it stays off until PID demand passes
        // 35% and then ramps at 1.35%/% so it reaches full speed while the
        // pump is still below its limit. Small corrections are pump-only.
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
