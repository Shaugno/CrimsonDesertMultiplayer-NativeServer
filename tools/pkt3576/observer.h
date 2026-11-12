#pragma once
// Experimental passive observer for the 3576 packet candidates.
// Game-free: the hook sink and the memory reader are supplied by the caller.
// This library does not include the mod offsets or the compatibility guard,
// and it does not call either packet function.

#include <atomic>
#include <cstdint>

#include "pkt3576/admission_gate.h"
#include "pkt3576/candidates.h"

namespace pkt3576 {

class Observer;

// Returns false when the range cannot be read. Must not throw: the callback
// path calls it and then returns. A process reader can be guarded_read.
using ReadFn = bool (*)(uint64_t address, void* destination, size_t size, void* user);

// Test or future install backend. remove() may invoke the observer callback
// before it returns. release_callback_code() is called only after every site
// is gone and the queue has been freed. A failed removal must leave the
// callback code and any trampoline in place.
class HookSink {
public:
    virtual ~HookSink() = default;
    virtual bool install(uint64_t address, Observer* observer) = 0;
    virtual bool remove(uint64_t address) = 0;
    virtual void release_callback_code() = 0;
};

// SEH copy. Returns false for a null destination, an empty or wrapping range,
// or an access violation. Does not write game memory.
bool guarded_read(uint64_t address, void* destination, size_t size);

enum class Direction : uint8_t { Send = 1, Receive = 2 };

// Raw entry values. Stack slots are present only when that read was part of
// the direction. prefix_len is the number of raw bytes copied from r9.
struct Record {
    Direction direction = Direction::Send;
    uint64_t rcx = 0;
    uint64_t rdx = 0;
    uint64_t r8 = 0;
    uint64_t r9 = 0;
    bool has_stack_u16_28 = false;
    uint16_t stack_u16_28 = 0;
    bool has_stack_u8_30 = false;
    uint8_t stack_u8_30 = 0;
    bool has_stack_u16_38 = false;
    uint16_t stack_u16_38 = 0;
    uint8_t prefix[kPrefixCap] = {};
    uint8_t prefix_len = 0;
};

// Registers at the candidate entry. rsp is the stack pointer before the prologue.
struct EntryArgs {
    uint64_t rcx = 0;
    uint64_t rdx = 0;
    uint64_t r8 = 0;
    uint64_t r9 = 0;
    uint64_t rsp = 0;
};

// Heap only. create() allocates. release_owner() deletes the object when no
// hook remains. When removal fails, release_owner() leaves the object, the
// queue, the reader, and the callback code in place and returns false.
// The ring needs at least two slots. One slot cannot tell full from empty.
class Observer {
public:
    using ConsumerHold = void (*)(void* user);

    static Observer* create(uint32_t capacity);
    // Returns true if `observer` was deleted. Returns false if a site is
    // still installed; `observer` is unchanged and still accepts callbacks.
    static bool release_owner(Observer*& observer);

    Observer(const Observer&) = delete;
    Observer& operator=(const Observer&) = delete;

    struct Deps {
        ReadFn read = nullptr;
        void* read_user = nullptr;
        HookSink* hooks = nullptr;
        uint64_t image_base = 0;
    };

    enum class InstallResult {
        Installed,
        Refused,
        RolledBack,
        Retained,
    };

    enum class ShutdownResult {
        Idle,
        Removed,
        Retained,
    };

    InstallResult install(const Deps& deps);
    ShutdownResult shutdown();

    // Called on the consumer thread inside try_pop, before the queue is read,
    // while shutdown is not allowed to free the queue. Null unless a test
    // installs one. Callbacks do not call it.
    void set_consumer_hold(ConsumerHold hold, void* user);

    // Callback bodies. They do not wait, allocate, perform file I/O, modify
    // game state, or call either packet function.
    void on_send(const EntryArgs& args);
    void on_recv(const EntryArgs& args);

    // Single consumer. Producers and this pop share one admission gate.
    // Shutdown frees the queue only after every admitted caller has left.
    bool try_pop(Record& out);
    uint64_t dropped() const;
    uint64_t unread() const;
    bool queue_allocated() const;
    const void* queue_address() const;
    bool accepting() const;
    bool send_installed() const;
    bool recv_installed() const;
    bool callback_code_released() const;

private:
    struct Ring;

    explicit Observer(uint32_t capacity);
    ~Observer();

    void observe(Direction direction, const EntryArgs& args, bool send_slots);
    bool read_at(uint64_t address, void* destination, size_t size) const;
    void retire_queue();
    void free_queue();
    bool site_installed() const;

    uint32_t capacity_ = 0;
    Ring* ring_ = nullptr;
    ReadFn read_ = nullptr;
    void* read_user_ = nullptr;
    HookSink* hooks_ = nullptr;
    uint64_t send_address_ = 0;
    uint64_t recv_address_ = 0;
    bool send_installed_ = false;
    bool recv_installed_ = false;
    bool callback_released_ = false;
    AdmissionGate gate_{};
    std::atomic<uint64_t> dropped_{0};
    std::atomic<uint64_t> unread_{0};
    std::atomic<ConsumerHold> consumer_hold_{nullptr};
    std::atomic<void*> consumer_hold_user_{nullptr};
};

inline constexpr uint32_t kMinQueueCapacity = 2;

}  // namespace pkt3576
