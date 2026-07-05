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
    constexpr double warningMarginC = 8.0;
    constexpr double warningClearMarginC = 4.0;
    constexpr double criticalMarginC = 22.0;

    const bool invalidTemperature = !inputs.temperatureValid || !std::isfinite(inputs.temperatureC);
    const bool criticalTemperature = inputs.temperatureC >= inputs.targetTempC + criticalMarginC;

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
