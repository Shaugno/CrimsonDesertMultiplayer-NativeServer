#pragma once
// Raw, passive observer for the experimental 3663 packet entries.
//
// This component has no opcode allow-list and assigns no packet/header/payload
// semantics. It copies a bounded byte range exactly as presented at the entry.
// A later runner supplies the read callback and hook delivery. The observer
// never calls either packet function and never writes game memory.

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "pkt3663/admission_gate.h"

namespace pkt3663 {

inline constexpr uint32_t kCopyCap = 4096;
inline constexpr uint32_t kMinQueueCapacity = 2;
inline constexpr uint32_t kAllowlistCount = 0;
inline constexpr uint64_t kStackSlot28 = 0x28;
inline constexpr uint64_t kStackSlot30 = 0x30;
inline constexpr uint64_t kStackSlot38 = 0x38;

// The observer catches an unexpected exception and counts the observation as
// unreadable, but production readers must still return false rather than throw.
using ReadFn = bool (*)(uint64_t address, void* destination, size_t size, void* user);

// SEH-protected local-process copy used by the future hook backend. Empty,
// null, and wrapping ranges are rejected.
bool guarded_read(uint64_t address, void* destination, size_t size);

enum class Direction : uint8_t { Send = 1, Receive = 2 };
enum class RawStatus : uint8_t { Complete = 1, Truncated = 2 };
enum class Phase : uint8_t { Unlabeled = 0, Stand = 1, Walk = 2, Turn = 3 };

struct Record {
    uint64_t rcx = 0;
    uint64_t rdx = 0;
    uint64_t r8 = 0;
    uint64_t r9 = 0;
    uint64_t qpc_ticks = 0;
    uint32_t claimed_length = 0;
    uint32_t copied_length = 0;
    uint16_t stack_u16_28 = 0;
    uint16_t stack_u16_38 = 0;
    // The first two copied bytes, when present. This is deliberately not
    // named opcode: a live 3663 capture has not established that meaning.
    uint16_t raw_word0 = 0;
    Direction direction = Direction::Send;
    RawStatus status = RawStatus::Complete;
    uint8_t stack_u8_30 = 0;
    uint8_t phase = 0;
    bool has_stack_u16_28 = false;
    bool has_stack_u8_30 = false;
    bool has_stack_u16_38 = false;
    bool has_raw_word0 = false;
    // The receive entry can invoke an optional transform before inspecting
    // bytes. Entry-time receive bytes therefore stay explicitly uncertain.
    bool receive_pre_transform_possible = false;
    uint8_t bytes[kCopyCap] = {};
};

static_assert(sizeof(Record) >= kCopyCap);
static_assert(sizeof(Record) <= kCopyCap + 256);
static_assert(256ull * sizeof(Record) * 2ull < 3ull * 1024ull * 1024ull);

struct EntryArgs {
    uint64_t rcx = 0;
    uint64_t rdx = 0;
    uint64_t r8 = 0;
    uint64_t r9 = 0;
    uint64_t rsp = 0;
};

class Observer {
public:
    using ConsumerHold = void (*)(void* user);

    static Observer* create(uint32_t capacity);
    static bool release_owner(Observer*& observer);

    Observer(const Observer&) = delete;
    Observer& operator=(const Observer&) = delete;

    bool arm(ReadFn read, void* user);
    bool disarm();

    void set_consumer_hold(ConsumerHold hold, void* user);
    void set_phase(uint8_t phase);

    // Callback path: no allocation, waiting, file I/O, packet call, or write
    // through the supplied reader.
    void on_send(const EntryArgs& args) noexcept;
    void on_recv(const EntryArgs& args) noexcept;

    // Single consumer. Producers may be concurrent.
    bool try_pop(Record& out) noexcept;
    uint64_t dropped() const;
    uint64_t unread() const;
    uint64_t truncated() const;
    bool queue_allocated() const;
    const void* queue_address() const;
    bool accepting() const;

private:
    struct Ring;

    explicit Observer(uint32_t capacity);
    ~Observer();

    void observe(Direction direction, const EntryArgs& args, bool send_slots);
    bool read_at(uint64_t address, void* destination, size_t size) const;
    void retire_queue();
    void free_queue();

    uint32_t capacity_ = 0;
    Ring* ring_ = nullptr;
    ReadFn read_ = nullptr;
    void* read_user_ = nullptr;
    AdmissionGate gate_{};
    std::atomic<uint64_t> dropped_{0};
    std::atomic<uint64_t> unread_{0};
    std::atomic<uint64_t> truncated_{0};
    std::atomic<uint8_t> phase_{0};
    std::atomic<ConsumerHold> consumer_hold_{nullptr};
    std::atomic<void*> consumer_hold_user_{nullptr};
};

}  // namespace pkt3663
