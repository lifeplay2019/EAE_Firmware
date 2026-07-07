#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace eae::firmware {

struct CanFrame {
    uint32_t id{};
    std::string name;
    std::map<std::string, double> signals;
};

namespace CanIds {
constexpr uint32_t TemperatureStatus = 0x180;
constexpr uint32_t CoolantStatus = 0x181;
constexpr uint32_t DriverCommand = 0x190;
constexpr uint32_t PumpStatus = 0x191;
constexpr uint32_t CoolingCommand = 0x200;
constexpr uint32_t DiagnosticStatus = 0x201;
}

// Simulated bus. The receive side is a mailbox that keeps only the newest
// frame per CAN id, which is how most CAN peripherals present data to the
// application. The transmit side keeps every frame in order so tests can
// assert on exactly what the controller sent in a scan.
class CanBus {
public:
    void receive(const CanFrame& frame);
    void transmit(const CanFrame& frame);

    [[nodiscard]] std::optional<CanFrame> latestReceived(uint32_t id) const;
    [[nodiscard]] const std::vector<CanFrame>& transmittedFrames() const;

    void clearTransmitted();

private:
    std::map<uint32_t, CanFrame> receivedMailbox_;
    std::vector<CanFrame> transmittedFrames_;
};

}  // namespace eae::firmware
