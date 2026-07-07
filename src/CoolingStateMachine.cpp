#include "eae_firmware/CoolingStateMachine.h"

#include <cmath>

namespace eae::firmware {

CoolingState CoolingStateMachine::state() const {
    return state_;
}

std::string CoolingStateMachine::stateName() const {
    return toString(state_);
}

CoolingState CoolingStateMachine::update(const StateInputs& inputs) {
    // Margins are relative to the target so the same thresholds work for any
    // setpoint passed on the command line.
    constexpr double warningMarginC = 8.0;
    constexpr double warningClearMarginC = 4.0;
    constexpr double criticalMarginC = 22.0;

    // A NaN or inf reading is treated the same as the sensor flagging itselfinvalid
    // We never feed garbage into the fault comparison below.
    const bool invalidTemperature = !inputs.temperatureValid || !std::isfinite(inputs.temperatureC);
    const bool criticalTemperature = inputs.temperatureC >= inputs.targetTempC + criticalMarginC;

    // Priority order matters: ignition gates everything, hard faults beat
    // the warning band, and RUNNING is only reached when nothing else fired.
    if (!inputs.ignitionOn) {
        state_ = CoolingState::Off;
        return state_;
    }

    if (!inputs.coolantLevelOk || invalidTemperature || inputs.pumpFault || criticalTemperature) {
        state_ = CoolingState::Fault;
        return state_;
    }

    if (inputs.temperatureC >= inputs.targetTempC + warningMarginC) {
        state_ = CoolingState::Warning;
        return state_;
    }

    // Hysteresis on the way out of WARNING: entry is at target +8 but the
    // state only clears below target+4. Without the gap the state (and the
    // derate request that follows it) would chatter around one trip point.
    if (state_ == CoolingState::Warning &&
        inputs.temperatureC > inputs.targetTempC + warningClearMarginC) {
        return state_;
    }

    state_ = CoolingState::Running;
    return state_;
}

std::string toString(CoolingState state) {
    switch (state) {
        case CoolingState::Off:
            return "OFF";
        case CoolingState::Running:
            return "RUNNING";
        case CoolingState::Warning:
            return "WARNING";
        case CoolingState::Fault:
            return "FAULT";
    }
    return "UNKNOWN";
}

}  // namespace eae::firmware
