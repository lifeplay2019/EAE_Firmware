#include "eae_firmware/CanBus.h"

namespace eae::firmware {

void CanBus::receive(const CanFrame& frame) {
    receivedMailbox_[frame.id] = frame;
}

void CanBus::transmit(const CanFrame& frame) {
    transmittedFrames_.push_back(frame);
}

std::optional<CanFrame> CanBus::latestReceived(uint32_t id) const {
    const auto found = receivedMailbox_.find(id);
    if (found == receivedMailbox_.end()) {
        return std::nullopt;
    }
    return found->second;
}

const std::vector<CanFrame>& CanBus::transmittedFrames() const {
    return transmittedFrames_;
}

void CanBus::clearTransmitted() {
    transmittedFrames_.clear();
}

}  // namespace eae::firmware
