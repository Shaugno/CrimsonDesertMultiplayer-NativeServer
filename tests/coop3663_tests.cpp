#include "coop3663/ack_decode.h"
#include "coop3663/feed.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <bit>

namespace {

int g_failures = 0;

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);         \
      ++g_failures;                                                            \
    }                                                                          \
  } while (false)

bool near(float value, float expected) { return std::fabs(value - expected) < 1.0e-5f; }

cdmp::coop3663::PlayerSample synthetic(uint64_t peer, uint32_t generation, uint64_t ms) {
  cdmp::coop3663::PlayerSample sample;
  sample.readiness = cdmp::coop3663::Readiness::Synthetic;
  sample.id = {peer, generation};
  sample.has_position = true;
  sample.has_heading = true;
  sample.pose = {1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  sample.observed_ms = ms;
  return sample;
}

class PlaceFailsOnce final : public cdmp::coop3663::ActorBackend {
public:
  bool create(const cdmp::coop3663::PeerId &id, const cdmp::coop3663::Pose &pose) override {
    if (fail_next) {
      fail_next = false;
      return false;
    }
    return inner.create(id, pose);
  }
  bool move(const cdmp::coop3663::PeerId &id, const cdmp::coop3663::Pose &pose) override {
    return inner.move(id, pose);
  }
  bool remove(const cdmp::coop3663::PeerId &id) override { return inner.remove(id); }

  bool fail_next = true;
  cdmp::coop3663::FakeActorBackend inner;
};

void test_unknown_fields_and_rotations() {
  cdmp::coop3663::FakeActorBackend backend;
  cdmp::coop3663::Feed feed(backend);
  CHECK(feed.begin_session(7, 1));
  cdmp::coop3663::PlayerSample unknown = synthetic(7, 1, 10);
  unknown.readiness = cdmp::coop3663::Readiness::Unknown;
  CHECK(!feed.publish(unknown));

  cdmp::coop3663::PlayerSample headingless = synthetic(7, 1, 10);
  headingless.has_heading = false;
  CHECK(!feed.publish(headingless));

  cdmp::coop3663::PlayerSample zero_quat = synthetic(7, 1, 10);
  zero_quat.pose.qw = 0.0f;
  CHECK(!cdmp::coop3663::accepted_pose(zero_quat).has_value());
  CHECK(!feed.publish(zero_quat));

  cdmp::coop3663::PlayerSample tiny = synthetic(7, 1, 10);
  tiny.pose.qw = 1.0e-8f;
  CHECK(!feed.publish(tiny));

  cdmp::coop3663::PlayerSample extreme = synthetic(7, 1, 10);
  extreme.pose.qw = 1.0e20f;
  CHECK(!feed.publish(extreme));

  cdmp::coop3663::PlayerSample broken = synthetic(7, 1, 10);
  broken.pose.qw = NAN;
  CHECK(!feed.publish(broken));
  CHECK(backend.events.empty());

  cdmp::coop3663::PlayerSample scaled = synthetic(7, 1, 10);
  scaled.pose.qw = 1.5f;
  const auto normalized = cdmp::coop3663::accepted_pose(scaled);
  CHECK(normalized.has_value());
  CHECK(near(normalized->qw, 1.0f));
  CHECK(feed.publish(scaled));
  CHECK(backend.events.size() == 1);
  CHECK(backend.events[0].has_pose);
  CHECK(near(backend.events[0].pose.qw, 1.0f));
  CHECK(near(backend.events[0].pose.x, 1.0f));
}

void test_initial_pose_and_failed_placement() {
  PlaceFailsOnce failing;
  cdmp::coop3663::Feed feed(failing);
  CHECK(feed.begin_session(4, 1));
  CHECK(!feed.publish(synthetic(4, 1, 10)));
  CHECK(!failing.inner.alive({4, 1}));
  CHECK(feed.publish(synthetic(4, 1, 10)));
  CHECK(failing.inner.events.size() == 1);
  CHECK(failing.inner.events[0].op == cdmp::coop3663::ActorOp::Create);
  CHECK(failing.inner.events[0].has_pose);
  CHECK(failing.inner.events[0].pose.x == 1.0f);
  CHECK(failing.inner.events[0].pose.qw == 1.0f);
}

void test_session_fence_clock_and_capacity() {
  cdmp::coop3663::FakeActorBackend backend;
  cdmp::coop3663::Feed feed(backend);
  CHECK(feed.begin_session(4, 1));
  CHECK(feed.publish(synthetic(4, 1, 100)));
  CHECK(!feed.publish(synthetic(4, 2, 300)));
  CHECK(backend.alive({4, 1}));
  CHECK(!backend.alive({4, 2}));

  CHECK(feed.begin_session(4, 2));
  cdmp::coop3663::PlayerSample restarted = synthetic(4, 2, 1);
  restarted.pose.x = 8.0f;
  CHECK(feed.publish(restarted));
  CHECK(!backend.alive({4, 1}));
  CHECK(backend.alive({4, 2}));
  CHECK(backend.events.back().op == cdmp::coop3663::ActorOp::Create);
  CHECK(backend.events.back().pose.x == 8.0f);
  CHECK(!feed.publish(synthetic(4, 1, 300)));
  CHECK(backend.alive({4, 2}));

  CHECK(!feed.publish(synthetic(4, 2, 1)));
  CHECK(feed.disconnect(4));
  CHECK(!backend.alive({4, 2}));
  CHECK(!feed.publish(synthetic(4, 2, 50)));
  CHECK(!feed.begin_session(4, 2));
  CHECK(feed.begin_session(4, 3));
  CHECK(feed.publish(synthetic(4, 3, 1)));
  CHECK(backend.alive({4, 3}));

  cdmp::coop3663::Feed limited(backend);
  for (uint64_t peer = 10; peer < 10 + cdmp::coop3663::kMaxPeers; ++peer)
    CHECK(limited.begin_session(peer, 1));
  CHECK(limited.active_count() == cdmp::coop3663::kMaxPeers);
  CHECK(!limited.begin_session(99, 1));
  CHECK(limited.disconnect(10));
  CHECK(limited.begin_session(99, 1));
  CHECK(limited.active_count() == cdmp::coop3663::kMaxPeers);
  for (int repeat = 0; repeat < 100; ++repeat) {
    CHECK(limited.begin_session(99, static_cast<uint32_t>(1000 + repeat)));
  }
  CHECK(limited.active_count() == cdmp::coop3663::kMaxPeers);
  CHECK(limited.retired_count() == cdmp::coop3663::kMaxRetired);
  limited.disconnect_all();
  CHECK(limited.active_count() == 0);
}

void test_literal_header_bound_includes_equality() {
  auto check = [](uint16_t declared, uint8_t flag, bool expect) {
    std::vector<uint8_t> bytes(static_cast<size_t>(declared) + 5, 0);
    bytes[3] = static_cast<uint8_t>(declared & 0xFF);
    bytes[4] = static_cast<uint8_t>(declared >> 8);
    cdmp::coop3663::AckBytes frame;
    frame.complete_length = static_cast<uint16_t>(declared + 5);
    frame.bytes = bytes;
    frame.bound_flag = flag;
    CHECK(cdmp::coop3663::ack_header_ok(frame) == expect);
  };
  check(0x7FC0, 0, true);
  check(0x7FC1, 0, true);
  check(0x7FC2, 0, false);
  check(0x7FC2, 1, true);
  cdmp::coop3663::AckBytes short_span;
  uint8_t tiny[4] = {};
  short_span.complete_length = 5;
  short_span.bytes = tiny;
  CHECK(!cdmp::coop3663::ack_header_ok(short_span));
  uint8_t mismatch[6] = {0, 0, 0, 2, 0, 0};
  cdmp::coop3663::AckBytes bad;
  bad.complete_length = 6;
  bad.bytes = mismatch;
  bad.bound_flag = 1;
  CHECK(!cdmp::coop3663::ack_header_ok(bad));
}

void test_file_proven_spatial_fields() {
  std::vector<uint8_t> bytes(244, 0);
  bytes[0] = 0xC0; bytes[1] = 0x0B; // File descriptor 3008.
  bytes[3] = 239;
  auto put = [&](size_t at, uint32_t word) {
    for (size_t i = 0; i < 4; ++i)
      bytes[at + i] = static_cast<uint8_t>(word >> (8 * i));
  };
  put(5, 0x12345678);
  put(201, std::bit_cast<uint32_t>(-8124.25f));
  put(205, std::bit_cast<uint32_t>(553.5f));
  put(209, std::bit_cast<uint32_t>(-1748.75f));
  cdmp::coop3663::AckBytes frame{244, bytes, 0};
  auto observation = cdmp::coop3663::decode_move_ack_spatial_fields(frame);
  CHECK(observation.has_value());
  if (observation) {
    CHECK(observation->raw_word0 == 0x12345678);
    CHECK(observation->spatial[0] == -8124.25f);
    CHECK(observation->spatial[1] == 553.5f);
    CHECK(observation->spatial[2] == -1748.75f);
  }
  frame.bytes = std::span<const uint8_t>(bytes).first(243);
  CHECK(!cdmp::coop3663::decode_move_ack_spatial_fields(frame));
  frame.bytes = bytes;
  bytes[0] = 0;
  CHECK(!cdmp::coop3663::decode_move_ack_spatial_fields(frame));
  bytes[0] = 0xC0;
  put(205, 0x7FC00000); // NaN.
  CHECK(!cdmp::coop3663::decode_move_ack_spatial_fields(frame));
  put(205, 0x7F800000); // Infinity.
  CHECK(!cdmp::coop3663::decode_move_ack_spatial_fields(frame));
  frame.complete_length = 243;
  bytes[3] = 238;
  CHECK(!cdmp::coop3663::decode_move_ack_spatial_fields(frame));
}

std::vector<uint8_t> frame_bytes(uint16_t opcode, const std::vector<uint8_t> &payload) {
  std::vector<uint8_t> bytes(5 + payload.size(), 0);
  bytes[0] = static_cast<uint8_t>(opcode);
  bytes[1] = static_cast<uint8_t>(opcode >> 8);
  bytes[3] = static_cast<uint8_t>(payload.size());
  bytes[4] = static_cast<uint8_t>(payload.size() >> 8);
  for (size_t i = 0; i < payload.size(); ++i) bytes[5 + i] = payload[i];
  return bytes;
}
cdmp::coop3663::AckBytes view(const std::vector<uint8_t> &bytes) {
  return {static_cast<uint16_t>(bytes.size()), bytes, 0};
}

void test_spawn_record_boundaries() {
  using cdmp::coop3663::split_actor_load_file_records;
  // Two independent length-prefixed bodies. Body internals are opaque.
  const std::vector<uint8_t> payload{5,0, 1,0x78,0x56,0x34,0x12,
                                   7,0, 2,0xEF,0xCD,0xAB,0x90,0xA5,0x5A};
  auto bytes = frame_bytes(2005, payload);
  auto records = split_actor_load_file_records(view(bytes));
  CHECK(records && records->size() == 2);
  if (records && records->size() == 2) {
    CHECK((*records)[0].raw_kind == 1);
    CHECK((*records)[0].raw_key == 0x12345678);
    CHECK((*records)[1].raw_key == 0x90ABCDEF);
    CHECK((*records)[1].body.size() == 7);
    CHECK((*records)[1].body[6] == 0x5A);
    CHECK((*records)[0].body.data() == bytes.data() + 7);
  }
  // Every incomplete prefix of the valid payload must fail, except the
  // exact first-record boundary, which is independently a complete list.
  for (size_t n = 1; n < payload.size(); ++n) {
    auto shortened = frame_bytes(2005, {payload.begin(), payload.begin() + n});
    CHECK(split_actor_load_file_records(view(shortened)).has_value() == (n == 7));
  }
  bytes[5] = 0;
  CHECK(!split_actor_load_file_records(view(bytes)));
  bytes[5] = 0xFF; bytes[6] = 0xFF;
  CHECK(!split_actor_load_file_records(view(bytes)));
  auto wrong = frame_bytes(3037, payload);
  CHECK(!split_actor_load_file_records(view(wrong)));
  auto empty = frame_bytes(2005, {});
  auto zero = split_actor_load_file_records(view(empty));
  CHECK(zero && zero->empty());
  std::vector<uint8_t> many;
  for (size_t i = 0; i < cdmp::coop3663::kMaxActorRecords + 1; ++i)
    many.insert(many.end(), {5,0,1,1,0,0,0});
  auto over = frame_bytes(2005, many);
  CHECK(!split_actor_load_file_records(view(over)));
  many.resize(cdmp::coop3663::kMaxActorRecords * 7);
  auto limit = frame_bytes(2005, many);
  auto at_limit = split_actor_load_file_records(view(limit));
  CHECK(at_limit && at_limit->size() == cdmp::coop3663::kMaxActorRecords);
}

void test_remove_actor_layout() {
  using cdmp::coop3663::decode_remove_actor_fields;
  const std::vector<uint8_t> payload{0xA1,0x78,0x56,0x34,0x12,0xB2,
                                   0x44,0x33,0x22,0x11,0x88,0x77,0x66,0x55};
  auto bytes = frame_bytes(3037, payload);
  auto fields = decode_remove_actor_fields(view(bytes));
  CHECK(fields && fields->raw_kind == 0xA1 && fields->raw_word1 == 0x12345678);
  CHECK(fields && fields->raw_reason == 0xB2 && fields->raw_keys.size() == 2);
  if (fields && fields->raw_keys.size() == 2) {
    CHECK(fields->raw_keys[0] == 0x11223344);
    CHECK(fields->raw_keys[1] == 0x55667788);
  }
  for (size_t n = 0; n < payload.size(); ++n) {
    auto shortened = frame_bytes(3037, {payload.begin(), payload.begin() + n});
    CHECK(decode_remove_actor_fields(view(shortened)).has_value() == (n == 6 || n == 10));
  }
  bytes.pop_back();
  CHECK(!decode_remove_actor_fields(view(bytes)));
  auto wrong = frame_bytes(2005, payload);
  CHECK(!decode_remove_actor_fields(view(wrong)));
  auto over = frame_bytes(3037, std::vector<uint8_t>(6 + 4 * (cdmp::coop3663::kMaxActorRecords + 1)));
  CHECK(!decode_remove_actor_fields(view(over)));
}

void test_rotation_field_validation() {
  std::vector<uint8_t> payload(239, 0);
  auto put = [&](size_t at, float value) {
    const auto word = std::bit_cast<uint32_t>(value);
    for (size_t i = 0; i < 4; ++i) payload[at + i] = static_cast<uint8_t>(word >> (8*i));
  };
  auto decode = [&] {
    auto bytes = frame_bytes(3008, payload);
    return cdmp::coop3663::decode_move_ack_transform_fields(view(bytes));
  };
  CHECK(!decode()); // Zero rotation must not become identity.
  for (size_t i = 0; i < 4; ++i) put(180 + i * 4, 0.75f);
  auto valid = decode();
  CHECK(valid.has_value());
  if (valid) for (float value : valid->rotation) CHECK(near(value, 0.5f));
  put(180, NAN); CHECK(!decode());
  put(180, INFINITY); CHECK(!decode());
  put(180, 1e30f); CHECK(!decode());
  for (size_t i = 0; i < 4; ++i) put(180 + i * 4, 1e-30f);
  CHECK(!decode());
  put(192, 1.0f);
  put(196, INFINITY);
  CHECK(!decode());
}

void test_offline_lifecycle_fixture() {
  // Explicit fixture mapping: wire key 77 belongs to owned test peer 42.
  // This does not assert a cross-PC or native actor identity relationship.
  auto spawn_bytes = frame_bytes(2005, {5,0,2,77,0,0,0});
  auto spawn = cdmp::coop3663::split_actor_load_file_records(view(spawn_bytes));
  CHECK(spawn && spawn->size() == 1 && (*spawn)[0].raw_key == 77);
  cdmp::coop3663::FakeActorBackend backend;
  cdmp::coop3663::Feed feed(backend);
  CHECK(feed.begin_session(42, 1));
  std::vector<uint8_t> payload(239, 0);
  auto put = [&](size_t at, float value) {
    const uint32_t word = std::bit_cast<uint32_t>(value);
    for (size_t i=0; i<4; ++i) payload[at+i] = static_cast<uint8_t>(word >> (8*i));
  };
  put(192, 1.0f);
  put(196, 10.0f); put(200, 20.0f); put(204, 30.0f);
  auto publish_fixture = [&](uint64_t timestamp) {
    auto bytes = frame_bytes(3008, payload);
    auto fields = cdmp::coop3663::decode_move_ack_transform_fields(view(bytes));
    CHECK(fields.has_value());
    if (!fields) return false;
    auto sample = synthetic(42, 1, timestamp);
    const auto &p = fields->movement.spatial;
    const auto &q = fields->rotation;
    sample.pose = {p[0], p[1], p[2], q[0], q[1], q[2], q[3]};
    return feed.publish(sample);
  };
  CHECK(publish_fixture(1));
  CHECK(backend.alive({42,1}));
  CHECK(backend.events.back().pose.x == 10.0f);
  put(196, 11.0f);
  CHECK(publish_fixture(2));
  CHECK(backend.events.back().op == cdmp::coop3663::ActorOp::Move);
  CHECK(backend.events.back().pose.x == 11.0f);
  auto remove_bytes = frame_bytes(3037, {2,0,0,0,0,6,77,0,0,0});
  auto remove = cdmp::coop3663::decode_remove_actor_fields(view(remove_bytes));
  CHECK(remove && remove->raw_keys.size() == 1 && remove->raw_keys[0] == 77);
  CHECK(feed.disconnect(42));
  CHECK(!backend.alive({42,1}));
  CHECK(!publish_fixture(3));
}

void test_body_relation_tail() {
  using cdmp::coop3663::decode_body_relation_tail;
  // Isolated fixture after a hypothetical component endpoint, not a spawn.
  std::vector<uint8_t> bytes{0x78,0x56,0x34,0x12,2,0,77,0,0,0,88,0,0,0};
  auto decoded = decode_body_relation_tail(bytes);
  CHECK(decoded.has_value());
  if (decoded) {
    CHECK(decoded->raw_relation == 0x12345678);
    CHECK(decoded->raw_keys == std::vector<uint32_t>({77,88}));
  }
  for (size_t n=0; n<bytes.size(); ++n)
    CHECK(!decode_body_relation_tail(std::span<const uint8_t>(bytes).first(n)));
  bytes.push_back(0);
  CHECK(!decode_body_relation_tail(bytes));
  bytes.resize(6);
  bytes[4]=0;
  CHECK(decode_body_relation_tail(bytes).has_value());
  bytes[5]=1; // 256, maximum diagnostic count, not a game limit.
  bytes.resize(6+256*4, 0);
  CHECK(decode_body_relation_tail(bytes).has_value());
  bytes[4]=1;
  bytes.resize(6+257*4, 0);
  CHECK(!decode_body_relation_tail(bytes));
  bytes[4]=255; bytes[5]=255;
  CHECK(!decode_body_relation_tail(bytes));
  // Preserve duplicate/raw keys; no guessed ownership or deduplication.
  bytes={0,0,0,0,2,0,255,255,255,255,255,255,255,255};
  decoded=decode_body_relation_tail(bytes);
  CHECK(decoded && decoded->raw_keys == std::vector<uint32_t>({0xFFFFFFFF,0xFFFFFFFF}));
}

void test_character_control_fields() {
  using cdmp::coop3663::decode_character_control_fields;
  // Synthetic isolated components. No native actor or resource resolution.
  for (unsigned mode = 0; mode < 8; ++mode) {
    std::vector<uint8_t> bytes{0x78,0x56,0x34,0x12};
    auto word = [&](uint32_t value) {
      for (unsigned j=0; j<4; ++j) bytes.push_back(static_cast<uint8_t>(value >> (8*j)));
    };
    std::array<size_t,2> slots{};
    for (unsigned i=0; i<2; ++i) {
      const bool skip = (mode & (1u << i)) != 0;
      bytes.push_back(skip ? 255 : 0); // Native branch is nonzero, not ==1.
      slots[i]=bytes.size();
      if (!skip) bytes.insert(bytes.end(),14,static_cast<uint8_t>(0xA0+i));
    }
    bytes.push_back((mode & 4) ? 2 : 0);
    if (mode & 4) bytes.push_back(0x5A);
    const size_t resources=bytes.size();
    bytes.insert(bytes.end(),16,0xC3); bytes.push_back(0x42);
    const size_t string_count=bytes.size();
    word(3); const size_t text=bytes.size();
    bytes.insert(bytes.end(),{'a',0,'z'}); // Preserve embedded zero bytes.
    const size_t suffix=bytes.size(); bytes.insert(bytes.end(),14,0xD4);
    std::array<size_t,4> entries{};
    for (unsigned i=0; i<4; ++i) {
      entries[i]=bytes.size(); word(i % 2 ? 0x12345678 : 0);
      if (i % 2) bytes.insert(bytes.end(),27,static_cast<uint8_t>(i));
    }
    const size_t extra_count=bytes.size(); word(2);
    word(0); word(1); bytes.insert(bytes.end(),27,0xEE);
    const size_t endpoint=bytes.size();
    auto parsed=decode_character_control_fields(bytes);
    CHECK(parsed.has_value());
    if (parsed) {
      CHECK(parsed->raw_word0 == 0x12345678);
      CHECK(parsed->consumed == endpoint);
      CHECK(parsed->optional_value.has_value() == ((mode & 4) != 0));
      if (parsed->optional_value) CHECK(*parsed->optional_value == 0x5A);
      CHECK(parsed->raw_resource_ids.data() == bytes.data()+resources);
      CHECK(parsed->raw_byte == 0x42);
      CHECK(parsed->raw_string_bytes.data() == bytes.data()+text);
      CHECK(parsed->raw_string_bytes.size() == 3 && parsed->raw_string_bytes[1] == 0);
      CHECK(parsed->raw_suffix.data() == bytes.data()+suffix);
      for (unsigned i=0; i<2; ++i) {
        CHECK(parsed->slot_data[i].size() == ((mode & (1u<<i)) ? 0u : 14u));
        if (!parsed->slot_data[i].empty()) CHECK(parsed->slot_data[i].data() == bytes.data()+slots[i]);
      }
      for (unsigned i=0; i<4; ++i) {
        CHECK(parsed->fixed_entries[i].data() == bytes.data()+entries[i]);
        CHECK(parsed->fixed_entries[i].size() == (i % 2 ? 31u : 4u));
      }
      CHECK(parsed->extra_entries.size() == 2);
    }
    for (size_t n=0; n<endpoint; ++n)
      CHECK(!decode_character_control_fields(std::span<const uint8_t>(bytes).first(n)));
    // A following component/tail must not be consumed as character data.
    bytes.insert(bytes.end(),{77,0,0,0,1,0,88,0,0,0});
    parsed=decode_character_control_fields(bytes);
    CHECK(parsed && parsed->consumed == endpoint);
    if (parsed) {
      auto tail=cdmp::coop3663::decode_body_relation_tail(std::span<const uint8_t>(bytes).subspan(parsed->consumed));
      CHECK(tail && tail->raw_relation == 77 && tail->raw_keys == std::vector<uint32_t>({88}));
    }
    // Exercise resource policy independently of available input bytes.
    auto bad=bytes;
    for (unsigned j=0; j<4; ++j) bad[string_count+j]=255;
    CHECK(!decode_character_control_fields(bad));
    bad=bytes;
    bad[extra_count]=1; bad[extra_count+1]=1; // 257
    CHECK(!decode_character_control_fields(bad));
    // Maximum allowed list of zero-tag entries, with no trailing component.
    bytes.resize(extra_count); word(256);
    bytes.insert(bytes.end(),256*4,0);
    parsed=decode_character_control_fields(bytes);
    CHECK(parsed && parsed->extra_entries.size() == 256 && parsed->consumed == bytes.size());
  }
  std::vector<uint8_t> minimum(62,0);
  minimum[4]=1; minimum[5]=1;
  auto parsed=decode_character_control_fields(minimum);
  CHECK(parsed && parsed->consumed == 62 && parsed->extra_entries.empty());
}

}  // namespace

int main() {
  test_unknown_fields_and_rotations();
  test_initial_pose_and_failed_placement();
  test_session_fence_clock_and_capacity();
  test_literal_header_bound_includes_equality();
  test_file_proven_spatial_fields();
  test_spawn_record_boundaries();
  test_remove_actor_layout();
  test_rotation_field_validation();
  test_offline_lifecycle_fixture();
  test_body_relation_tail();
  test_character_control_fields();
  if (g_failures != 0) {
    std::printf("coop3663 tests: %d failure(s)\n", g_failures);
    return 1;
  }
  std::printf("coop3663 tests ok\n");
  return 0;
}
