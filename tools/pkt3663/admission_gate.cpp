#include "pkt3663/admission_gate.h"

#include <windows.h>

namespace pkt3663 {

bool AdmissionGate::open() noexcept {
    uint32_t current = kClosed;
    return state_.compare_exchange_strong(current, 0, std::memory_order_release, std::memory_order_acquire);
}

bool AdmissionGate::admit() noexcept {
    uint32_t current = state_.load(std::memory_order_relaxed);
    for (;;) {
        if ((current & kClosed) != 0) return false;
        if ((current & ~kClosed) == ~kClosed) return false;
        if (state_.compare_exchange_strong(current, current + 1, std::memory_order_acq_rel,
                                           std::memory_order_acquire)) {
            return true;
        }
    }
}

void AdmissionGate::leave() noexcept { state_.fetch_sub(1, std::memory_order_release); }

void AdmissionGate::close() noexcept {
    uint32_t current = state_.load(std::memory_order_relaxed);
    for (;;) {
        if ((current & kClosed) != 0) return;
        if (state_.compare_exchange_strong(current, current | kClosed, std::memory_order_acq_rel,
                                           std::memory_order_acquire)) {
            return;
        }
    }
}

void AdmissionGate::close_and_wait() noexcept {
    close();
    while ((state_.load(std::memory_order_acquire) & ~kClosed) != 0) YieldProcessor();
}

bool AdmissionGate::closed() const noexcept {
    return (state_.load(std::memory_order_acquire) & kClosed) != 0;
}

uint32_t AdmissionGate::admitted() const noexcept {
    return state_.load(std::memory_order_acquire) & ~kClosed;
}

}  // namespace pkt3663
