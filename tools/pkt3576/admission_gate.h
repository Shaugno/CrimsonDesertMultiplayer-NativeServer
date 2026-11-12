#pragma once
// One-word admission gate. The closed bit and the active count share one
// atomic, so a callback publishes itself and observes retirement in one CAS.
// Entry retries CAS contention without waiting on a lock. close_and_wait is
// for the retiring thread.

#include <atomic>
#include <cstdint>

namespace pkt3576 {

class AdmissionGate {
public:
    AdmissionGate() = default;

    AdmissionGate(const AdmissionGate&) = delete;
    AdmissionGate& operator=(const AdmissionGate&) = delete;

    // Publish the resource, then open. Starts closed.
    bool open() noexcept;
    // True when this caller may use the resource. False means it must not.
    bool admit() noexcept;
    void leave() noexcept;
    // Stop new admissions and wait until every admitted caller has left.
    void close_and_wait() noexcept;

    bool closed() const noexcept;
    uint32_t admitted() const noexcept;

private:
    static constexpr uint32_t kClosed = 0x80000000u;
    // Closed and empty until open().
    std::atomic<uint32_t> state_{kClosed};
};

}  // namespace pkt3576
