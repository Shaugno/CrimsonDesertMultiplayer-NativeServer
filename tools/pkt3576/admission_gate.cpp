#include "pkt3576/admission_gate.h"

#include <windows.h>

namespace pkt3576 {

bool AdmissionGate::open() noexcept {
    uint32_t cur = kClosed;
    return state_.compare_exchange_strong(cur, 0, std::memory_order_release, std::memory_order_acquire);
}

bool AdmissionGate::admit() noexcept {
    uint32_t cur = state_.load(std::memory_order_relaxed);
    for (;;) {
        if ((cur & kClosed) != 0) return false;
        // Leave the top bit clear. A full count refuses instead of looking closed.
        if ((cur & ~kClosed) == ~kClosed) return false;
        if (state_.compare_exchange_strong(cur, cur + 1, std::memory_order_acq_rel,
                                            std::memory_order_acquire)) {
            return true;
        }
    }
}

void AdmissionGate::leave() noexcept { state_.fetch_sub(1, std::memory_order_release); }

void AdmissionGate::close_and_wait() noexcept {
    uint32_t cur = state_.load(std::memory_order_relaxed);
    for (;;) {
        if ((cur & kClosed) != 0) break;
        if (state_.compare_exchange_strong(cur, cur | kClosed, std::memory_order_acq_rel,
                                            std::memory_order_acquire)) {
            break;
        }
    }
    while ((state_.load(std::memory_order_acquire) & ~kClosed) != 0) YieldProcessor();
}

bool AdmissionGate::closed() const noexcept {
    return (state_.load(std::memory_order_acquire) & kClosed) != 0;
}

uint32_t AdmissionGate::admitted() const noexcept {
    return state_.load(std::memory_order_acquire) & ~kClosed;
}

}  // namespace pkt3576
