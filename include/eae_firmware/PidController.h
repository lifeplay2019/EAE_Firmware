#pragma once

namespace eae::firmware {

// Defaults are tuned against the demo thermal plant in main.cpp. The plant
// is slow (time constant in the tens of seconds), so ki is kept small; most
// of the work is done by the proportional term.
struct PidGains {
    double kp{4.0};
    double ki{0.08};
    double kd{1.2};
};

// PID with output clamping and conditional anti-windup: the integral only
// accumulates while the output is unsaturated, or while the error would
// drive the output back inside the limits.
class PidController {
public:
    PidController(PidGains gains, double outputMin, double outputMax);

    double update(double error, double dtSeconds);
    void reset();

private:
    PidGains gains_;
    double outputMin_;
    double outputMax_;
    double integral_{0.0};
    double previousError_{0.0};
    bool hasPreviousError_{false};
};

}  // namespace eae::firmware