#pragma once
#include "common/troc_link.h"
#include <deque>

namespace cdmp::session2 {
// Network-thread bridge for an A+B host. No legacy join/save/observer callbacks.
class R3TunnelBridge {
public:
    explicit R3TunnelBridge(ClientSession& session);
    void tick(uint32_t now);
    std::string describe() const;
private:
    void receive(std::vector<uint8_t>&& packet, uint32_t now);
    troc::TrocLink link_;
    std::deque<std::vector<uint8_t>> pending_;
    uint32_t epoch_ = 0, first_pending_ = 0;
    uint64_t queued_ = 0, refused_ = 0;
    bool failed_ = false;
};
}
