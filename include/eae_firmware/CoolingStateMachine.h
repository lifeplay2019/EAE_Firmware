#pragma once

#include <string>

namespace eae::firmware {

enum class CoolingState {
    Off = 0,
    Running = 1,
    Warning = 2,
    Fault = 3,
};

struct StateInputs {
    bool ignitionOn{false};
    bool coolantLevelOk{true};
    bool temperatureValid{true};
    bool pumpFault{false};
    double temperatureC{25.0};
    double targetTempC{65.0};
};

class CoolingStateMachine {
public:
    [[nodiscard]] CoolingState state() const;
    [[nodiscard]] std::string stateName() const;

    CoolingState update(const StateInputs& inputs);

private:
    CoolingState state_{CoolingState::Off};
};

std::string toString(CoolingState state);

}  // namespace eae::firmware
