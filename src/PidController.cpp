#include "eae_firmware/PidController.h"

#include <algorithm>
#include <stdexcept>

namespace eae::firmware {

PidController::PidController(PidGains gains, double outputMin, double outputMax)
    : gains_(gains), outputMin_(outputMin), outputMax_(outputMax) {
    if (outputMin_ >= outputMax_) {
        throw std::invalid_argument("PID outputMin must be smaller than outputMax");
    }
}

double PidController::update(double error, double dtSeconds) {
    if (dtSeconds <= 0.0) {
        throw std::invalid_argument("PID dtSeconds must be greater than zero");
    }

    const double derivative = hasPreviousError_ ? (error - previousError_) / dtSeconds : 0.0;
    const double candidateIntegral = integral_ + error * dtSeconds;
    const double rawOutput = gains_.kp * error + gains_.ki * candidateIntegral + gains_.kd * derivative;
    const double clampedOutput = std::clamp(rawOutput, outputMin_, outputMax_);

    const bool outputSaturatedHigh = rawOutput > outputMax_;
    const bool outputSaturatedLow = rawOutput < outputMin_;
    if ((!outputSaturatedHigh || error < 0.0) && (!outputSaturatedLow || error > 0.0)) {
        integral_ = candidateIntegral;
    }

    previousError_ = error;
    hasPreviousError_ = true;

    return clampedOutput;
}

void PidController::reset() {
    integral_ = 0.0;
    previousError_ = 0.0;
    hasPreviousError_ = false;
}

}  // namespace eae::firmware
