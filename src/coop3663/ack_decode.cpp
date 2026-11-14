#include "coop3663/ack_decode.h"
#include "coop3663/rotation.h"

#include <cstring>
#include <bit>
#include <cmath>

namespace cdmp::coop3663 {
namespace {

constexpr uint16_t kHeaderBytes = 5;
constexpr uint16_t kLiteralBound = 0x7FC1;

uint32_t little_word(std::span<const uint8_t> bytes, size_t at) {
  return uint32_t{bytes[at]} | (uint32_t{bytes[at + 1]} << 8) |
         (uint32_t{bytes[at + 2]} << 16) | (uint32_t{bytes[at + 3]} << 24);
}
bool opcode_is(const AckBytes &frame, uint16_t opcode) {
  return (uint32_t{frame.bytes[0]} | (uint32_t{frame.bytes[1]} << 8)) == opcode;
}

}  // namespace

bool ack_header_ok(const AckBytes &frame) {
  if (frame.bytes.size() < kHeaderBytes || frame.complete_length < kHeaderBytes ||
      frame.bytes.size() < frame.complete_length) {
    return false;
  }
  uint16_t declared = 0;
  std::memcpy(&declared, frame.bytes.data() + 3, sizeof declared);
  if (static_cast<uint16_t>(frame.complete_length - kHeaderBytes) != declared)
    return false;
  // cmp ax, cx / jb rejects only when 0x7FC1 < declared.
  if (frame.bound_flag == 0 && kLiteralBound < declared)
    return false;
  return true;
}

std::optional<MoveSpatialFields> decode_move_ack_spatial_fields(const AckBytes &frame) {
  // 239 bytes follows all nested reads, not merely the top-level calls.
  if (!ack_header_ok(frame) || frame.complete_length != 244)
    return std::nullopt;
  const auto bytes = frame.bytes;
  if (!opcode_is(frame, 3008))
    return std::nullopt;
  const auto word = [&](size_t at) {
    return uint32_t{bytes[at]} | (uint32_t{bytes[at + 1]} << 8) |
           (uint32_t{bytes[at + 2]} << 16) | (uint32_t{bytes[at + 3]} << 24);
  };
  MoveSpatialFields out;
  out.raw_word0 = word(5);
  for (size_t i = 0; i < out.spatial.size(); ++i) {
    out.spatial[i] = std::bit_cast<float>(word(5 + 196 + 4 * i));
    if (!std::isfinite(out.spatial[i]))
      return std::nullopt;
  }
  return out;
}

std::optional<MoveTransformFields> decode_move_ack_transform_fields(const AckBytes &frame) {
  auto movement = decode_move_ack_spatial_fields(frame);
  if (!movement)
    return std::nullopt;
  MoveTransformFields out{*movement, {}};
  for (size_t i = 0; i < 4; ++i) {
    const float value = std::bit_cast<float>(little_word(frame.bytes, 5 + 180 + 4 * i));
    if (!std::isfinite(value))
      return std::nullopt;
    out.rotation[i] = value;
  }
  const auto rotation = normalized_rotation(out.rotation);
  if (!rotation) return std::nullopt;
  out.rotation = *rotation;
  return out;
}

std::optional<std::vector<SpawnRecordView>> split_actor_load_file_records(const AckBytes &frame) {
  if (!ack_header_ok(frame) || !opcode_is(frame, 2005))
    return std::nullopt;
  std::vector<SpawnRecordView> out;
  const auto payload = frame.bytes.first(frame.complete_length).subspan(5);
  size_t cursor = 0;
  while (cursor < payload.size()) {
    if (out.size() == kMaxActorRecords || payload.size() - cursor < 2)
      return std::nullopt;
    const size_t count = size_t{payload[cursor]} | (size_t{payload[cursor + 1]} << 8);
    cursor += 2;
    if (count < 5 || count > payload.size() - cursor)
      return std::nullopt;
    const auto body = payload.subspan(cursor, count);
    out.push_back({body[0], little_word(body, 1), body});
    cursor += count;
  }
  return out;
}

std::optional<BodyRelationTail> decode_body_relation_tail(std::span<const uint8_t> tail) {
  if (tail.size() < 6)
    return std::nullopt;
  const size_t count = size_t{tail[4]} | (size_t{tail[5]} << 8);
  // Offline allocation policy; neither a native limit nor a spawn validator.
  if (count > kMaxActorRecords || tail.size() - 6 != count * 4)
    return std::nullopt;
  BodyRelationTail out{little_word(tail, 0), {}};
  out.raw_keys.reserve(count);
  for (size_t i = 0; i < count; ++i)
    out.raw_keys.push_back(little_word(tail, 6 + i * 4));
  return out;
}

std::optional<CharacterControlFields> decode_character_control_fields(std::span<const uint8_t> input) {
  size_t cursor = 0;
  auto take = [&](size_t count, std::span<const uint8_t> &out) {
    if (count > input.size() - cursor) return false;
    out = input.subspan(cursor, count);
    cursor += count;
    return true;
  };
  auto byte = [&](uint8_t &out) {
    std::span<const uint8_t> bytes;
    if (!take(1, bytes)) return false;
    out = bytes[0];
    return true;
  };
  auto word = [&](uint32_t &out) {
    std::span<const uint8_t> bytes;
    if (!take(4, bytes)) return false;
    out = little_word(bytes, 0);
    return true;
  };
  auto entry = [&](std::span<const uint8_t> &out) {
    const size_t begin = cursor;
    uint32_t tag = 0;
    std::span<const uint8_t> rest;
    if (!word(tag) || (tag != 0 && !take(27, rest))) return false;
    out = input.subspan(begin, cursor - begin);
    return true;
  };
  CharacterControlFields out;
  if (!word(out.raw_word0)) return std::nullopt;
  for (size_t i = 0; i < 2; ++i) {
    if (!byte(out.slot_flags[i])) return std::nullopt;
    if (out.slot_flags[i] == 0 && !take(14, out.slot_data[i])) return std::nullopt;
  }
  if (!byte(out.optional_flag)) return std::nullopt;
  if (out.optional_flag != 0) {
    uint8_t value = 0;
    if (!byte(value)) return std::nullopt;
    out.optional_value = value;
  }
  if (!take(16, out.raw_resource_ids) || !byte(out.raw_byte)) return std::nullopt;
  uint32_t count = 0;
  // Offline resource policy, not a native string limit or encoding claim.
  if (!word(count) || count > 4096 || !take(count, out.raw_string_bytes) ||
      !take(14, out.raw_suffix)) return std::nullopt;
  for (auto &item : out.fixed_entries)
    if (!entry(item)) return std::nullopt;
  if (!word(count) || count > kMaxActorRecords) return std::nullopt;
  out.extra_entries.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    std::span<const uint8_t> item;
    if (!entry(item)) return std::nullopt;
    out.extra_entries.push_back(item);
  }
  out.consumed = cursor;
  return out;
}

std::optional<RemoveActorFields> decode_remove_actor_fields(const AckBytes &frame) {
  if (!ack_header_ok(frame) || !opcode_is(frame, 3037))
    return std::nullopt;
  const auto payload = frame.bytes.first(frame.complete_length).subspan(5);
  if (payload.size() < 6 || (payload.size() - 6) % 4 != 0 ||
      (payload.size() - 6) / 4 > kMaxActorRecords)
    return std::nullopt;
  RemoveActorFields out;
  out.raw_kind = payload[0];
  out.raw_word1 = little_word(payload, 1);
  out.raw_reason = payload[5];
  out.raw_keys.reserve((payload.size() - 6) / 4);
  for (size_t cursor = 6; cursor < payload.size(); cursor += 4)
    out.raw_keys.push_back(little_word(payload, cursor));
  return out;
}

}  // namespace cdmp::coop3663
