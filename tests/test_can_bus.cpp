#include "eae_firmware/CanBus.h"

#include <gtest/gtest.h>

using namespace eae::firmware;

TEST(CanBusTest, StoresLatestReceivedFrameById) {
    CanBus bus;

    bus.receive({CanIds::TemperatureStatus, "old", {{"coolant_temp_c", 40.0}}});
    bus.receive({CanIds::TemperatureStatus, "new", {{"coolant_temp_c", 55.0}}});

    const auto frame = bus.latestReceived(CanIds::TemperatureStatus);
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->name, "new");
    EXPECT_DOUBLE_EQ(frame->signals.at("coolant_temp_c"), 55.0);
}

TEST(CanBusTest, KeepsTransmittedFrameLogUntilCleared) {
    CanBus bus;

    bus.transmit({CanIds::CoolingCommand, "command", {{"pump_percent", 75.0}}});

    ASSERT_EQ(bus.transmittedFrames().size(), 1U);
    EXPECT_DOUBLE_EQ(bus.transmittedFrames().front().signals.at("pump_percent"), 75.0);

    bus.clearTransmitted();
    EXPECT_TRUE(bus.transmittedFrames().empty());
}
