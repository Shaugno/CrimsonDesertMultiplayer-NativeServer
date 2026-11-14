#pragma once
// Shared length gate for the 3663 handlers whose listing uses
// `mov eax, 0x7FC1` / `cmp ax, cx` / `jb`. That failure branch runs when the
// declared payload length is greater than 0x7FC1. Equality is allowed by this
// comparison alone. ActorLoadDataAck slot 2 computes its bound; file arithmetic
// on the recorded image also resolves to 0x7FC1. No live value is assumed.
//
// Only the separately documented full-parse transform and envelope fields
// are decoded. Spawn component bodies remain opaque. The reader helper at
// exe+0x26A8CE0, and the calls it makes into exe+0x14CB150, consume the cursor
// between the top-level reads. Summing only the top-level sizes is not a layout.

#include <cstdint>
#include <span>
#include <array>
#include <optional>
#include <vector>

namespace cdmp::coop3663 {

struct AckBytes {
  uint16_t complete_length = 0;
  std::span<const uint8_t> bytes;
  // Zero selects the inclusive 0x7FC1 bound. Nonzero skips that comparison.
  uint8_t bound_flag = 0;
};

bool ack_header_ok(const AckBytes &frame);

// File-proven full-parse branch only. This is an offline observation, not
// a live-ready player pose. The descriptor's opcode is file evidence;
// coordinate space, live orientation convention, and actor-key semantics
// remain unresolved. Callers must supply frames from the assessed 3663 file
// format; these helpers do not identify a process or authorize deployment.
struct MoveSpatialFields {
  uint32_t raw_word0 = 0;
  std::array<float, 3> spatial{};
};
std::optional<MoveSpatialFields> decode_move_ack_spatial_fields(const AckBytes &frame);

struct MoveTransformFields {
  MoveSpatialFields movement;
  std::array<float, 4> rotation{}; // Normalized; axis convention still unverified.
};
std::optional<MoveTransformFields> decode_move_ack_transform_fields(const AckBytes &frame);

// Offline resource limit, not a claim about the game's supported actor count.
inline constexpr size_t kMaxActorRecords = 256;
struct SpawnRecordView {
  uint8_t raw_kind = 0;
  uint32_t raw_key = 0;
  // Borrowed from AckBytes; source storage must outlive the returned views.
  // Includes kind/key; component schemas are deliberately left opaque.
  std::span<const uint8_t> body;
};
std::optional<std::vector<SpawnRecordView>> split_actor_load_file_records(const AckBytes &frame);

// Body reader 0xB846AC9: AFTER all component readers, a raw dword followed
// by a u16 count and that many dwords. Caller must establish the component
// endpoint independently. Never search backward for a plausible tail or use
// this helper to certify an opaque spawn body. Input is exactly the tail.
struct BodyRelationTail {
  uint32_t raw_relation = 0;
  std::vector<uint32_t> raw_keys;
};
std::optional<BodyRelationTail> decode_body_relation_tail(std::span<const uint8_t> tail);

// Isolated component start, established by an external, evidenced component
// inventory. This is structural consumption, not successful native creation.
// All spans borrow input storage. Raw bytes retain unresolved field semantics.
struct CharacterControlFields {
  uint32_t raw_word0 = 0;
  std::array<uint8_t, 2> slot_flags{}; // zero consumes 14 bytes; nonzero skips
  std::array<std::span<const uint8_t>, 2> slot_data{};
  uint8_t optional_flag = 0;
  std::optional<uint8_t> optional_value;
  std::span<const uint8_t> raw_resource_ids; // two 8-byte wire IDs, not resolved IDs
  uint8_t raw_byte = 0;
  std::span<const uint8_t> raw_string_bytes;
  std::span<const uint8_t> raw_suffix; // 8 + 1 + 1 + 4 bytes
  std::array<std::span<const uint8_t>, 4> fixed_entries;
  std::vector<std::span<const uint8_t>> extra_entries;
  size_t consumed = 0; // following components remain unread
};
std::optional<CharacterControlFields> decode_character_control_fields(std::span<const uint8_t> input);

struct RemoveActorFields {
  uint8_t raw_kind = 0;
  uint32_t raw_word1 = 0;
  uint8_t raw_reason = 0;
  std::vector<uint32_t> raw_keys;
};
std::optional<RemoveActorFields> decode_remove_actor_fields(const AckBytes &frame);

}  // namespace cdmp::coop3663
