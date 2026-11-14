#pragma once
// Game-free stand-in for a future 3663 state feed.
//
// A sample is not a live player. Unknown readiness never reaches the backend.
// Position and heading are present only when the caller sets those flags and
// the numbers are finite. A heading must also be a usable rotation: its
// quaternion norm must be from 0.5 through 2, inclusive, and is normalized
// to length 1 using the same policy as the offline movement decoder.
// An identity quaternion does not set the heading flag by itself.
//
// Sessions are explicit. publish accepts a sample only for the generation
// last admitted by begin_session for that peer. Timestamps are ordered inside
// that generation. A new begin_session retires the previous generation, fences
// it, and resets the clock so a restarted session may use a smaller timestamp.
// begin_session accepts only a strictly higher generation than any previously
// admitted for that peer. History is never evicted, including on disconnect.
// Generation wrap requires a new Feed lifetime, not an older generation.
//
// At most kMaxPeers active sessions are stored. Empty slots are reused.
// At most kMaxKnownPeers histories are stored; new peers are refused when full.
// Known peers can still reconnect with a higher generation.
// Single-threaded; caller serializes all operations, including backend calls.

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace cdmp::coop3663 {

inline constexpr uint32_t kMaxPeers = 8;
inline constexpr uint32_t kMaxKnownPeers = 4096;

enum class Readiness : uint8_t {
  Unknown = 0,
  Synthetic = 1,
  Invalid = 2,
};

struct PeerId {
  uint64_t value = 0;
  uint32_t generation = 0;
  bool operator==(const PeerId &) const = default;
};

struct Pose {
  float x = 0;
  float y = 0;
  float z = 0;
  float qx = 0;
  float qy = 0;
  float qz = 0;
  float qw = 0;
};

struct PlayerSample {
  Readiness readiness = Readiness::Unknown;
  PeerId id{};
  bool has_position = false;
  bool has_heading = false;
  Pose pose{};
  uint64_t observed_ms = 0;
};

enum class ActorOp : uint8_t { Create = 1, Move = 2, Remove = 3 };

struct ActorEvent {
  ActorOp op = ActorOp::Create;
  PeerId id{};
  Pose pose{};
  bool has_pose = false;
};

class ActorBackend {
public:
  virtual ~ActorBackend() = default;
  // Places the actor at pose, or leaves no actor when it returns false.
  virtual bool create(const PeerId &id, const Pose &pose) = 0;
  virtual bool move(const PeerId &id, const Pose &pose) = 0;
  virtual bool remove(const PeerId &id) = 0;
};

class FakeActorBackend final : public ActorBackend {
public:
  bool create(const PeerId &id, const Pose &pose) override;
  bool move(const PeerId &id, const Pose &pose) override;
  bool remove(const PeerId &id) override;

  bool alive(const PeerId &id) const;
  std::vector<ActorEvent> events;
};

// Returns the pose that would be delivered, with heading normalized, or
// nullopt when the sample must not reach an actor.
std::optional<Pose> accepted_pose(const PlayerSample &sample);

class Feed {
public:
  explicit Feed(ActorBackend &backend);

  bool begin_session(uint64_t peer, uint32_t generation);
  bool publish(const PlayerSample &sample);
  bool disconnect(uint64_t peer);
  // Returns the number of sessions whose actors could not be removed.
  uint32_t disconnect_all();

  uint32_t active_count() const;
  uint32_t known_peer_count() const;

private:
  struct Slot {
    uint64_t peer = 0;
    uint32_t generation = 0;
    uint64_t observed_ms = 0;
    bool active = false;
    bool placed = false;
    bool has_clock = false;
  };
  Slot *find_active(uint64_t peer);
  Slot *free_slot();
  bool retire_active(Slot &slot);

  ActorBackend &backend_;
  Slot slots_[kMaxPeers]{};
  std::unordered_map<uint64_t, uint32_t> highest_generation_;
};

}  // namespace cdmp::coop3663
