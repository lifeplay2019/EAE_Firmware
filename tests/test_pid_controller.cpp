#include "eae_firmware/PidController.h"

#include <gtest/gtest.h>

using namespace eae::firmware;

TEST(PidControllerTest, OutputIncreasesForPositiveCoolingError) {
    PidController pid({4.0, 0.1, 0.0}, 0.0, 100.0);

    const double first = pid.update(2.0, 1.0);
    const double second = pid.update(4.0, 1.0);

    EXPECT_GT(first, 0.0);
    EXPECT_GT(second, first);
}

TEST(PidControllerTest, OutputIsClampedToConfiguredRange) {
    PidController pid({50.0, 10.0, 0.0}, 0.0, 100.0);

    EXPECT_DOUBLE_EQ(pid.update(100.0, 1.0), 100.0);
}

TEST(PidControllerTest, ResetClearsIntegralHistory) {
    PidController pid({0.0, 1.0, 0.0}, 0.0, 100.0);

    const double accumulated = pid.update(5.0, 1.0);
    pid.reset();
    const double afterReset = pid.update(5.0, 1.0);

    EXPECT_DOUBLE_EQ(accumulated, afterReset);
}
