#pragma once
// Isolated SafetyHook backend for the experimental 3663 raw observer.
//
// This target is not linked into the ASI and does not accept a game build by
// itself. A production runner must pass the strict mapped-image checks before
// calling start(). The fixture build may omit the install check for dummy
// functions in its own process.

#include <cstdint>

#include "pkt3663/observer.h"

namespace pkt3663 {

struct CaptureLimits {
  uint32_t queue_capacity = 256;
  uint32_t output_records = 256;
  bool exclusive_create = false;
};

enum class InstallSite : uint8_t {
  Receive = 1,
  Send = 2,
  Complete = 3,
};

struct InstallEvidence {
  // Send describes the installed Receive slot. Complete describes the
  // installed Send slot. Every pointer is valid only during the callback.
  const uint8_t *target_patch = nullptr;
  uint8_t target_patch_size = 0;
  const uint8_t *original_bytes = nullptr;
  uint8_t original_size = 0;
};

// Receive and Send run synchronously immediately before their hook factories.
// Complete runs immediately after Send installation. A production callback
// uses the evidence to compare SafetyHook's saved instruction-aligned original
// bytes with the verified candidate entry. It must not throw, wait on packet
// callbacks, mutate either target, or call this Capture. False fails closed;
// installed hooks are retired only while exact ownership can still be proved.
using InstallCheck = bool (*)(InstallSite site, const InstallEvidence *evidence,
                              void *user) noexcept;

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

  static Capture *create(const CaptureLimits &limits,
                         const wchar_t *output_path);
  static bool release_owner(Capture *&capture);

  Capture(const Capture &) = delete;
  Capture &operator=(const Capture &) = delete;

  // Both entries must belong to module. Receive is installed first. The
  // check is optional only for the game-free fixture path.
  StartResult start(void *module, void *send_entry, void *recv_entry,
                    InstallCheck check = nullptr, void *check_user = nullptr);
  StopResult stop();

  void set_writer_blocked(bool blocked);
  void set_phase(uint8_t phase);
  // Optional same-thread marks. Production uses them for session timing.
  // 1 receive hook installed, 2 send hook installed, 3 writer thread started.
  void set_stage_mark(void (*mark)(uint8_t stage) noexcept) noexcept;

  // Game-free scheduling seams. Production leaves these null.
  static void set_records_open_hook(void (*hook)(void *), void *user);
  static void set_phase_hold(void (*hold)(void *), void *user);
  // Runs after receive installation and immediately before its live-patch
  // integrity proof. Game-free tests only; production leaves it null.
  static void set_pre_send_hook(void (*hook)(void *), void *user);

  uint32_t stored() const;
  uint32_t omitted() const;
  uint64_t dropped() const;
  uint64_t unread() const;
  uint64_t truncated() const;
  bool queue_allocated() const;
  bool accepting() const;
  bool copy_record(uint32_t index, Record &out) const;

  struct State;

private:
  explicit Capture(State *state);
  ~Capture();
  void writer_main();

  State *state_ = nullptr;
};

} // namespace pkt3663
