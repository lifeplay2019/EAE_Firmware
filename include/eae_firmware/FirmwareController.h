#pragma once

#include "eae_firmware/CanBus.h"
#include "eae_firmware/CoolingStateMachine.h"
#include "eae_firmware/PidController.h"

namespace eae::firmware {

struct FirmwareConfig {
    double targetTempC{65.0};
};

struct ControlOutputs {
    CoolingState state{CoolingState::Off};
    double pumpPercent{0.0};
    double fanPercent{0.0};
    bool derateRequested{false};
};

class FirmwareController {
public:
    explicit FirmwareController(FirmwareConfig config = {});

    ControlOutputs step(CanBus& bus, double dtSeconds);
    void reset();

private:
    FirmwareConfig config_;
    PidController pid_;
    CoolingStateMachine stateMachine_;
};

}  // namespace eae::firmware
