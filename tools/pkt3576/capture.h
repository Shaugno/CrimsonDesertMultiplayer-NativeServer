#pragma once
// Isolated SafetyHook capture backend for the experimental 3576 observer.
// Not linked into the ASI. It does not open the compatibility guard, read a
// game image, or adopt packet offsets. A caller that targets a game module
// has to apply those checks itself before calling start().

#include <cstdint>

#include "pkt3576/observer.h"

namespace pkt3576 {

struct CaptureLimits {
    uint32_t queue_capacity = 256;
    // Records kept in memory and written by the consumer thread.
    uint32_t output_records = 256;
};

// One capture at a time. create() allocates. release_owner() deletes the
// object only after the hooks are gone and the writer thread has been joined.
// A failed removal leaves the object, the hooks, the queue, and the writer
// alive and returns false.
class Capture {
public:
    enum class StartResult {
        Started,
        Refused,
        RolledBack,
        Retained,
    };

    enum class StopResult {
        Idle,
        Removed,
        Retained,
    };

    static Capture* create(const CaptureLimits& limits, const wchar_t* output_path);
    static bool release_owner(Capture*& capture);

    Capture(const Capture&) = delete;
    Capture& operator=(const Capture&) = delete;

    // Both entries must belong to `module`. Receive is installed first.
    // This does not check a PE timestamp and does not set a force-load flag.
    StartResult start(void* module, void* send_entry, void* recv_entry);
    StopResult stop();

    // The writer spins while blocked. Callbacks do not look at this flag.
    void set_writer_blocked(bool blocked);

    uint32_t stored() const;
    uint32_t omitted() const;
    uint64_t dropped() const;
    uint64_t unread() const;
    bool queue_allocated() const;
    bool accepting() const;
    // Valid after stop() returns Removed. The writer is the only producer.
    bool copy_record(uint32_t index, Record& out) const;

    // Incomplete in this header. The definition stays in the capture
    // translation unit so SafetyHook types are not part of this interface.
    struct State;

private:
    explicit Capture(State* state);
    ~Capture();
    void writer_main();

    State* state_ = nullptr;
};

}  // namespace pkt3576
