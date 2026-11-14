#include "coop3663/feed.h"

#include "coop3663/rotation.h"
#include <cmath>

namespace cdmp::coop3663 {
namespace {

bool finite_position(const Pose &pose) {
  return std::isfinite(pose.x) && std::isfinite(pose.y) && std::isfinite(pose.z);
}

}  // namespace

bool FakeActorBackend::create(const PeerId &id, const Pose &pose) {
  if (alive(id))
    return false;
  events.push_back(ActorEvent{ActorOp::Create, id, pose, true});
  return true;
}

bool FakeActorBackend::move(const PeerId &id, const Pose &pose) {
  if (!alive(id))
    return false;
  events.push_back(ActorEvent{ActorOp::Move, id, pose, true});
  return true;
}

bool FakeActorBackend::remove(const PeerId &id) {
  if (!alive(id))
    return false;
  events.push_back(ActorEvent{ActorOp::Remove, id, {}, false});
  return true;
}

bool FakeActorBackend::alive(const PeerId &id) const {
  int depth = 0;
  for (const ActorEvent &event : events) {
    if (event.id != id)
      continue;
    if (event.op == ActorOp::Create)
      ++depth;
    else if (event.op == ActorOp::Remove && depth > 0)
      --depth;
  }
  return depth > 0;
}

std::optional<Pose> accepted_pose(const PlayerSample &sample) {
  if (sample.readiness != Readiness::Synthetic || sample.id.value == 0)
    return std::nullopt;
  if (!sample.has_position || !sample.has_heading || !finite_position(sample.pose))
    return std::nullopt;
  const auto rotation = normalized_rotation({sample.pose.qx, sample.pose.qy, sample.pose.qz, sample.pose.qw});
  if (!rotation) return std::nullopt;
  Pose pose = sample.pose;
  pose.qx = (*rotation)[0];
  pose.qy = (*rotation)[1];
  pose.qz = (*rotation)[2];
  pose.qw = (*rotation)[3];
  return pose;
}

Feed::Feed(ActorBackend &backend) : backend_(backend) {}

Feed::Slot *Feed::find_active(uint64_t peer) {
  for (Slot &slot : slots_) {
    if (slot.active && slot.peer == peer)
      return &slot;
  }
  return nullptr;
}

Feed::Slot *Feed::free_slot() {
  for (Slot &slot : slots_) {
    if (!slot.active)
      return &slot;
  }
  return nullptr;
}

bool Feed::retire_active(Slot &slot) {
  if (slot.placed && !backend_.remove(PeerId{slot.peer, slot.generation}))
    return false;
  slot.active = false;
  slot.placed = false;
  slot.has_clock = false;
  return true;
}

bool Feed::begin_session(uint64_t peer, uint32_t generation) {
  if (peer == 0) return false;
  auto history = highest_generation_.find(peer);
  if (history != highest_generation_.end() && generation <= history->second) return false;
  Slot *slot = find_active(peer);
  if (!slot) slot = free_slot();
  if (!slot) return false;
  if (history == highest_generation_.end()) {
    if (highest_generation_.size() >= kMaxKnownPeers) return false;
    // Allocate history before changing actor state; never evict another peer.
    history = highest_generation_.emplace(peer, generation).first;
  }
  if (slot->active && !retire_active(*slot)) return false;
  history->second = generation;
  *slot = Slot{};
  slot->peer = peer;
  slot->generation = generation;
  slot->active = true;
  return true;
}

bool Feed::publish(const PlayerSample &sample) {
  const std::optional<Pose> pose = accepted_pose(sample);
  if (!pose.has_value())
    return false;
  Slot *slot = find_active(sample.id.value);
  if (slot == nullptr || slot->generation != sample.id.generation)
    return false;
  if (slot->has_clock && sample.observed_ms <= slot->observed_ms)
    return false;
  const PeerId id{slot->peer, slot->generation};
  if (!slot->placed) {
    if (!backend_.create(id, *pose))
      return false;
    slot->placed = true;
  } else if (!backend_.move(id, *pose)) {
    return false;
  }
  slot->observed_ms = sample.observed_ms;
  slot->has_clock = true;
  return true;
}

bool Feed::disconnect(uint64_t peer) {
  Slot *slot = find_active(peer);
  if (slot == nullptr)
    return false;
  return retire_active(*slot);
}

uint32_t Feed::disconnect_all() {
  uint32_t failed = 0;
  for (Slot &slot : slots_) {
    if (slot.active && !retire_active(slot)) ++failed;
  }
  return failed;
}

uint32_t Feed::active_count() const {
  uint32_t count = 0;
  for (const Slot &slot : slots_) {
    if (slot.active)
      ++count;
  }
  return count;
}

uint32_t Feed::known_peer_count() const { return static_cast<uint32_t>(highest_generation_.size()); }

}  // namespace cdmp::coop3663
