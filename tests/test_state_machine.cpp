#include "eae_firmware/CoolingStateMachine.h"

#include <gtest/gtest.h>

using namespace eae::firmware;

TEST(CoolingStateMachineTest, IgnitionOffGoesToOff) {
    CoolingStateMachine machine;

    const auto state = machine.update({false, true, true, false, 45.0, 65.0});

    EXPECT_EQ(state, CoolingState::Off);
}

TEST(CoolingStateMachineTest, HealthyIgnitionOnGoesToRunning) {
    CoolingStateMachine machine;

    const auto state = machine.update({true, true, true, false, 55.0, 65.0});

    EXPECT_EQ(state, CoolingState::Running);
}

TEST(CoolingStateMachineTest, HighTemperatureGoesToWarning) {
    CoolingStateMachine machine;

    const auto state = machine.update({true, true, true, false, 74.0, 65.0});

    EXPECT_EQ(state, CoolingState::Warning);
}

TEST(CoolingStateMachineTest, LowCoolantGoesToFault) {
    CoolingStateMachine machine;

    const auto state = machine.update({true, false, true, false, 55.0, 65.0});

    EXPECT_EQ(state, CoolingState::Fault);
}
