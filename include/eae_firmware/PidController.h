#pragma once

namespace eae::firmware {

struct PidGains {
    double kp{4.0};
    double ki{0.08};
    double kd{1.2};
};

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