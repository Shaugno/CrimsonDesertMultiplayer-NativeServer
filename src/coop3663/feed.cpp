#include "coop3663/feed.h"

#include <algorithm>
#include <cmath>

namespace cdmp::coop3663 {
namespace {

constexpr float kMinHeadingNorm = 1.0e-4f;
constexpr float kMaxHeadingComponent = 1.0e8f;
constexpr float kMinNormalizableNorm = 0.5f;
constexpr float kMaxNormalizableNorm = 2.0f;

bool finite_position(const Pose &pose) {
  return std::isfinite(pose.x) && std::isfinite(pose.y) && std::isfinite(pose.z);
}

float abs_max_heading(const Pose &pose) {
  float m = std::fabs(pose.qx);
  m = std::max(m, std::fabs(pose.qy));
  m = std::max(m, std::fabs(pose.qz));
  return std::max(m, std::fabs(pose.qw));
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
  if (!std::isfinite(sample.pose.qx) || !std::isfinite(sample.pose.qy) ||
      !std::isfinite(sample.pose.qz) || !std::isfinite(sample.pose.qw)) {
    return std::nullopt;
  }
  const float scale = abs_max_heading(sample.pose);
  if (!(scale > 0.0f) || scale > kMaxHeadingComponent)
    return std::nullopt;
  const float x = sample.pose.qx / scale;
  const float y = sample.pose.qy / scale;
  const float z = sample.pose.qz / scale;
  const float w = sample.pose.qw / scale;
  const float unit = std::sqrt(x * x + y * y + z * z + w * w);
  if (!(unit > 0.0f) || !std::isfinite(unit))
    return std::nullopt;
  const float norm = unit * scale;
  if (!(norm >= kMinHeadingNorm) || norm < kMinNormalizableNorm || norm > kMaxNormalizableNorm)
    return std::nullopt;
  const float inv = 1.0f / norm;
  Pose pose = sample.pose;
  pose.qx *= inv;
  pose.qy *= inv;
  pose.qz *= inv;
  pose.qw *= inv;
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

bool Feed::retired(uint64_t peer, uint32_t generation) const {
  for (const Retired &entry : retired_) {
    if (entry.used && entry.peer == peer && entry.generation == generation)
      return true;
  }
  return false;
}

void Feed::fence(uint64_t peer, uint32_t generation) {
  if (retired(peer, generation))
    return;
  Retired &entry = retired_[retired_next_];
  const bool replacing = entry.used;
  entry = Retired{peer, generation, true};
  retired_next_ = (retired_next_ + 1) % kMaxRetired;
  if (!replacing && retired_used_ < kMaxRetired)
    ++retired_used_;
}

bool Feed::retire_active(Slot &slot) {
  if (slot.placed && !backend_.remove(PeerId{slot.peer, slot.generation}))
    return false;
  fence(slot.peer, slot.generation);
  slot.active = false;
  slot.placed = false;
  slot.has_clock = false;
  return true;
}

bool Feed::begin_session(uint64_t peer, uint32_t generation) {
  if (peer == 0 || retired(peer, generation))
    return false;
  if (Slot *slot = find_active(peer)) {
    if (slot->generation == generation)
      return true;
    if (!retire_active(*slot))
      return false;
  }
  Slot *slot = free_slot();
  if (slot == nullptr)
    return false;
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
  if (retired(sample.id.value, sample.id.generation))
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

void Feed::disconnect_all() {
  for (Slot &slot : slots_) {
    if (slot.active)
      retire_active(slot);
  }
}

uint32_t Feed::active_count() const {
  uint32_t count = 0;
  for (const Slot &slot : slots_) {
    if (slot.active)
      ++count;
  }
  return count;
}

uint32_t Feed::retired_count() const { return retired_used_; }

}  // namespace cdmp::coop3663
