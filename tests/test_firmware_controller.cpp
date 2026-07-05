#include "eae_firmware/FirmwareController.h"

#include <gtest/gtest.h>

using namespace eae::firmware;

namespace {

void publishInputs(CanBus& bus,
                   double temperatureC,
                   bool coolantOk,
                   bool ignitionOn,
                   bool pumpFault = false) {
    bus.receive({CanIds::TemperatureStatus, "temp", {{"coolant_temp_c", temperatureC}, {"sensor_valid", 1.0}}});
    bus.receive({CanIds::CoolantStatus, "coolant", {{"coolant_level_ok", coolantOk ? 1.0 : 0.0}}});
    bus.receive({CanIds::DriverCommand, "driver", {{"ignition_on", ignitionOn ? 1.0 : 0.0}}});
    bus.receive({CanIds::PumpStatus, "pump", {{"pump_fault", pumpFault ? 1.0 : 0.0}}});
}

}  // namespace

TEST(FirmwareControllerTest, SendsCoolingCommandWhenRunningHot) {
    CanBus bus;
    FirmwareController controller({65.0});

    publishInputs(bus, 76.0, true, true);
    const auto outputs = controller.step(bus, 1.0);

    EXPECT_EQ(outputs.state, CoolingState::Warning);
    EXPECT_GT(outputs.pumpPercent, 25.0);
    EXPECT_TRUE(outputs.derateRequested);
    ASSERT_GE(bus.transmittedFrames().size(), 2U);
    EXPECT_EQ(bus.transmittedFrames().front().id, CanIds::CoolingCommand);
}

TEST(FirmwareControllerTest, FaultDisablesPumpAndRequestsDerate) {
    CanBus bus;
    FirmwareController controller({65.0});

    publishInputs(bus, 60.0, false, true);
    const auto outputs = controller.step(bus, 1.0);

    EXPECT_EQ(outputs.state, CoolingState::Fault);
    EXPECT_DOUBLE_EQ(outputs.pumpPercent, 0.0);
    EXPECT_DOUBLE_EQ(outputs.fanPercent, 0.0);
    EXPECT_TRUE(outputs.derateRequested);
}
