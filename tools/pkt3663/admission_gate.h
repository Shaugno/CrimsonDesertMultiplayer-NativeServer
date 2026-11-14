#pragma once
// One-word admission gate for the isolated 3663 raw observer. The closed bit
// and active count share one atomic so teardown cannot free the ring while a
// callback or the consumer still owns an admission.

#include <atomic>
#include <cstdint>

namespace pkt3663 {

class AdmissionGate {
public:
    AdmissionGate() = default;
    AdmissionGate(const AdmissionGate&) = delete;
    AdmissionGate& operator=(const AdmissionGate&) = delete;

    bool open() noexcept;
    bool admit() noexcept;
    void leave() noexcept;
    void close() noexcept;
    void close_and_wait() noexcept;

    bool closed() const noexcept;
    uint32_t admitted() const noexcept;

private:
    static constexpr uint32_t kClosed = 0x80000000u;
    std::atomic<uint32_t> state_{kClosed};
};

}  // namespace pkt3663
