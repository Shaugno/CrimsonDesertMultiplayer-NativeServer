// Game-free tests for the strict mapped-image 3663 live gate.
// The fixture is synthetic memory; no process or game file is opened.

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

#include "pkt3663/assess.h"
#include "pkt3663/candidates.h"
#include "pkt3663/live_identity.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);              \
      ++g_failures;                                                            \
    }                                                                          \
  } while (0)

constexpr uint32_t kPdataVa = 0x1000;
constexpr uint32_t kTextVa = 0x3A8000;
constexpr uint32_t kTextSize = 0x2800;
constexpr uintptr_t kBase = static_cast<uintptr_t>(pkt3663::kImageBase3663);

void put_u16(std::vector<uint8_t> &image, size_t at, uint16_t value) {
  std::memcpy(image.data() + at, &value, sizeof value);
}

void put_u32(std::vector<uint8_t> &image, size_t at, uint32_t value) {
  std::memcpy(image.data() + at, &value, sizeof value);
}

void put_u64(std::vector<uint8_t> &image, size_t at, uint64_t value) {
  std::memcpy(image.data() + at, &value, sizeof value);
}

void put_section(std::vector<uint8_t> &image, size_t at, const char *name,
                 uint32_t virtual_size, uint32_t va, uint32_t raw_size,
                 uint32_t flags) {
  std::memset(image.data() + at, 0, 40);
  std::memcpy(image.data() + at, name, std::strlen(name));
  put_u32(image, at + 8, virtual_size);
  put_u32(image, at + 12, va);
  put_u32(image, at + 16, raw_size);
  put_u32(image, at + 36, flags);
}

template <size_t N>
void put_anchors(std::vector<uint8_t> &image,
                 const std::array<pkt3663::ByteAnchor, N> &anchors) {
  for (const pkt3663::ByteAnchor &anchor : anchors) {
    std::memcpy(image.data() + anchor.rva, anchor.bytes, anchor.size);
  }
}

void put_function(std::vector<uint8_t> &image, size_t index, uint32_t begin,
                  uint32_t end) {
  const size_t row = kPdataVa + index * 12;
  put_u32(image, row, begin);
  put_u32(image, row + 4, end);
  put_u32(image, row + 8, 0);
}

void put_call(std::vector<uint8_t> &image, uint32_t call_rva,
              uint32_t target_rva) {
  const int64_t displacement =
      static_cast<int64_t>(target_rva) - (static_cast<int64_t>(call_rva) + 5);
  image[call_rva] = 0xE8;
  put_u32(image, call_rva + 1,
          static_cast<uint32_t>(static_cast<int32_t>(displacement)));
}

struct Fixture {
  uintptr_t base = kBase;
  std::vector<uint8_t> image;
  uint32_t fail_rva = std::numeric_limits<uint32_t>::max();
  uint32_t fail_size = 0;
  uint64_t reads = 0;
  uint64_t candidate_reads = 0;
};

bool read_fixture(uintptr_t address, void *destination, size_t size,
                  void *user) {
  auto &fixture = *static_cast<Fixture *>(user);
  ++fixture.reads;
  if (destination == nullptr || size == 0 || address < fixture.base)
    return false;
  const uint64_t rva64 = static_cast<uint64_t>(address - fixture.base);
  if (rva64 > std::numeric_limits<uint32_t>::max())
    return false;
  const uint32_t rva = static_cast<uint32_t>(rva64);
  if (size > fixture.image.size() || rva > fixture.image.size() - size)
    return false;
  const uint64_t end = static_cast<uint64_t>(rva) + size;
  if (rva < pkt3663::kReceiveEndRva && end > pkt3663::kPumpRva)
    ++fixture.candidate_reads;
  if (fixture.fail_size != 0) {
    const uint64_t fail_end =
        static_cast<uint64_t>(fixture.fail_rva) + fixture.fail_size;
    if (rva < fail_end && fixture.fail_rva < end)
      return false;
  }
  std::memcpy(destination, fixture.image.data() + rva, size);
  return true;
}

Fixture make_fixture(uint32_t timestamp = pkt3663::kTimestamp3663,
                     uint64_t preferred_base = kBase,
                     uint32_t image_size = pkt3663::kSizeOfImage3663) {
  Fixture fixture;
  fixture.image.resize(static_cast<size_t>(kTextVa) + kTextSize, 0);
  auto &image = fixture.image;
  image[0] = 'M';
  image[1] = 'Z';
  put_u32(image, 0x3C, 0x80);
  put_u32(image, 0x80, 0x00004550);
  put_u16(image, 0x86, 2);
  put_u32(image, 0x88, timestamp);
  put_u16(image, 0x94, 0xF0);
  put_u16(image, 0x98, 0x20B);
  put_u64(image, 0xB0, preferred_base);
  put_u32(image, 0xD0, image_size);
  put_u32(image, 0xD4, 0x400);
  put_u32(image, 0x104, 16);
  put_u32(image, 0x120, kPdataVa);
  put_u32(image, 0x124, 36);
  put_section(image, 0x188, ".pdata", 0x400, kPdataVa, 0x400, 0x40000040);
  put_section(image, 0x1B0, ".text", kTextSize, kTextVa, kTextSize, 0x60000020);
  put_function(image, 0, pkt3663::kPumpRva, pkt3663::kPumpEndRva);
  put_function(image, 1, pkt3663::kSendRva, pkt3663::kSendEndRva);
  put_function(image, 2, pkt3663::kReceiveRva, pkt3663::kReceiveEndRva);
  put_anchors(image, pkt3663::kPumpAnchors);
  put_anchors(image, pkt3663::kSendAnchors);
  put_anchors(image, pkt3663::kReceiveAnchors);
  put_call(image, pkt3663::kPumpReceiveCallRva, pkt3663::kReceiveRva);
  put_call(image, pkt3663::kPumpSendCallRva, pkt3663::kSendRva);
  return fixture;
}

pkt3663::Identity identify(Fixture &fixture,
                           const wchar_t *name = L"CrimsonDesert.exe") {
  return pkt3663::identify_image(name, fixture.base, &read_fixture, &fixture);
}

void test_exact_mapped_fixture_matches() {
  Fixture fixture = make_fixture();
  const pkt3663::Identity identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::Match);
  CHECK(pkt3663::identity_allows_hooks(identity));
  CHECK(identity.timestamp_known &&
        identity.timestamp == pkt3663::kTimestamp3663);
  CHECK(identity.preferred_base_known &&
        identity.preferred_base == pkt3663::kImageBase3663);
  CHECK(identity.size_known &&
        identity.size_of_image == pkt3663::kSizeOfImage3663);
  CHECK(identity.sections_ok && identity.function_table_ok);
  CHECK(identity.pump_range_ok && identity.send_range_ok &&
        identity.receive_range_ok);
  CHECK(identity.pump_anchors_ok && identity.send_anchors_ok &&
        identity.receive_anchors_ok);
  CHECK(identity.anchors_unique && identity.role_calls_ok);
  CHECK(identity.failed_anchor_rva == 0);
  CHECK(fixture.candidate_reads != 0);
  CHECK(std::strcmp(pkt3663::identity_kind_name(identity.kind), "match") == 0);
}

void test_old_builds_stop_before_candidates() {
  for (uint32_t timestamp :
       {pkt3663::kTimestamp2976, pkt3663::kTimestamp3576, 0x12345678u}) {
    Fixture fixture = make_fixture(timestamp);
    const pkt3663::Identity identity = identify(fixture);
    CHECK(identity.kind == pkt3663::IdentityKind::WrongTimestamp);
    CHECK(!pkt3663::identity_allows_hooks(identity));
    CHECK(fixture.candidate_reads == 0);
    CHECK(identity.rejected_2976 == (timestamp == pkt3663::kTimestamp2976));
    CHECK(identity.rejected_3576 == (timestamp == pkt3663::kTimestamp3576));
    CHECK(!identity.sections_ok && !identity.function_table_ok);
  }
}

void test_other_identity_fields_stop_before_candidates() {
  Fixture fixture = make_fixture();
  pkt3663::Identity identity = identify(fixture, L"not-the-game.exe");
  CHECK(identity.kind == pkt3663::IdentityKind::WrongName);
  CHECK(fixture.candidate_reads == 0);

  fixture = make_fixture(pkt3663::kTimestamp3663, 0x180000000ULL);
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::WrongBase);
  CHECK(fixture.candidate_reads == 0);

  fixture = make_fixture();
  fixture.base = static_cast<uintptr_t>(0x180000000ULL);
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::WrongBase);
  CHECK(fixture.candidate_reads == 0);

  fixture = make_fixture(pkt3663::kTimestamp3663, kBase, 0x1234000);
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::WrongSize);
  CHECK(fixture.candidate_reads == 0);
}

void test_unreadable_ranges_fail_closed() {
  Fixture fixture = make_fixture();
  fixture.fail_rva = pkt3663::kSendAnchors[2].rva;
  fixture.fail_size = static_cast<uint32_t>(pkt3663::kSendAnchors[2].size);
  pkt3663::Identity identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);
  CHECK(!pkt3663::identity_allows_hooks(identity));
  CHECK(!identity.send_anchors_ok);

  fixture = make_fixture();
  fixture.fail_rva = kPdataVa + 12;
  fixture.fail_size = 12;
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);
  CHECK(!identity.function_table_ok);
}

void test_every_anchor_is_required_and_unique() {
  std::vector<const pkt3663::ByteAnchor *> anchors;
  for (const auto &anchor : pkt3663::kPumpAnchors)
    anchors.push_back(&anchor);
  for (const auto &anchor : pkt3663::kSendAnchors)
    anchors.push_back(&anchor);
  for (const auto &anchor : pkt3663::kReceiveAnchors)
    anchors.push_back(&anchor);

  constexpr uint32_t duplicate_rva = kTextVa + 0x100;
  for (const pkt3663::ByteAnchor *anchor : anchors) {
    Fixture fixture = make_fixture();
    fixture.image[anchor->rva + anchor->size - 1] ^= 0x01;
    pkt3663::Identity identity = identify(fixture);
    CHECK(identity.kind == pkt3663::IdentityKind::AnchorMismatch);
    // Some anchors intentionally overlap a longer entry signature. The
    // first failed anchor is still exact evidence, but need not be the
    // shorter anchor whose shared byte this test changed.
    CHECK(identity.failed_anchor_rva != 0);
    CHECK(!pkt3663::identity_allows_hooks(identity));

    fixture = make_fixture();
    std::memcpy(fixture.image.data() + duplicate_rva, anchor->bytes,
                anchor->size);
    identity = identify(fixture);
    CHECK(identity.kind == pkt3663::IdentityKind::AmbiguousAnchor);
    CHECK(!identity.anchors_unique);
    CHECK(!pkt3663::identity_allows_hooks(identity));
  }
}

void test_sections_and_pdata_fail_closed() {
  Fixture fixture = make_fixture();
  put_u32(fixture.image, 0x1B0 + 36, 0x40000040);
  pkt3663::Identity identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::NotExecutable);
  CHECK(!pkt3663::identity_allows_hooks(identity));

  fixture = make_fixture();
  put_section(fixture.image, 0x1B0, ".text", kTextSize, 0x1100, kTextSize,
              0x60000020);
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::MalformedImage);
  CHECK(!identity.sections_ok);

  fixture = make_fixture();
  put_u32(fixture.image, kPdataVa + 12 + 4, pkt3663::kSendEndRva + 1);
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::FunctionRangeMismatch);
  CHECK(!identity.send_range_ok);

  fixture = make_fixture();
  put_u32(fixture.image, 0x124, 0);
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::MissingFunctionTable);

  fixture = make_fixture();
  put_u32(fixture.image, 0x124, 48);
  put_function(fixture.image, 3, pkt3663::kReceiveRva, pkt3663::kReceiveEndRva);
  identity = identify(fixture);
  CHECK(identity.kind == pkt3663::IdentityKind::MalformedImage);
  CHECK(!pkt3663::identity_allows_hooks(identity));
}

void test_pump_calls_are_decoded() {
  for (uint32_t call :
       {pkt3663::kPumpReceiveCallRva, pkt3663::kPumpSendCallRva}) {
    for (size_t byte = 0; byte < 5; ++byte) {
      Fixture fixture = make_fixture();
      fixture.image[call + byte] ^= 0x01;
      const pkt3663::Identity identity = identify(fixture);
      CHECK(identity.kind == pkt3663::IdentityKind::CallTargetMismatch);
      CHECK(!identity.role_calls_ok);
      CHECK(!pkt3663::identity_allows_hooks(identity));
    }
  }
}

void test_second_verification_catches_a_change() {
  Fixture fixture = make_fixture();
  CHECK(pkt3663::identity_allows_hooks(identify(fixture)));
  fixture.image[pkt3663::kReceiveAnchors[0].rva + 8] ^= 0x01;
  const pkt3663::Identity changed = identify(fixture);
  CHECK(changed.kind == pkt3663::IdentityKind::AnchorMismatch);
  CHECK(!pkt3663::identity_allows_hooks(changed));
}

void plant_receive_e9_patch(Fixture &fixture) {
  fixture.image[pkt3663::kReceiveRva] = 0xE9;
  fixture.image[pkt3663::kReceiveRva + 1] = 0xCC;
  fixture.image[pkt3663::kReceiveRva + 2] = 0xCC;
  fixture.image[pkt3663::kReceiveRva + 3] = 0xCC;
  fixture.image[pkt3663::kReceiveRva + 4] = 0xCC;
}

void test_send_site_recheck_masks_only_receive_patch_window() {
  Fixture fixture = make_fixture();
  plant_receive_e9_patch(fixture);
  CHECK(identify(fixture).kind == pkt3663::IdentityKind::AnchorMismatch);
  pkt3663::Identity identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 5);
  CHECK(identity.kind == pkt3663::IdentityKind::Match);
  CHECK(pkt3663::identity_allows_hooks(identity));

  fixture = make_fixture();
  uint8_t ff25_patch[15] = {0xFF, 0x25, 0, 0, 0, 0};
  const uint64_t fake_stub = 0x1122334455667788ull;
  std::memcpy(ff25_patch + 6, &fake_stub, sizeof fake_stub);
  ff25_patch[14] = 0x90;
  std::memcpy(fixture.image.data() + pkt3663::kReceiveRva, ff25_patch,
              sizeof ff25_patch);
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture, ff25_patch,
      sizeof ff25_patch);
  CHECK(identity.kind == pkt3663::IdentityKind::Match);

  fixture.image[pkt3663::kReceiveRva + 10] ^= 0x01;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture, ff25_patch,
      sizeof ff25_patch);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);

  fixture = make_fixture();
  fixture.image[pkt3663::kReceiveRva] = 0xFF;
  fixture.image[pkt3663::kReceiveRva + 1] = 0x25;
  std::memset(fixture.image.data() + pkt3663::kReceiveRva + 2, 0, 4);
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 6);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);

  fixture = make_fixture();
  plant_receive_e9_patch(fixture);
  fixture.image[pkt3663::kReceiveRva + 5] = 0xCC;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 6);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);

  fixture = make_fixture();
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture, nullptr, 0);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);

  fixture = make_fixture();
  fixture.image[pkt3663::kReceiveRva] = 0x90;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 5);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);

  fixture = make_fixture();
  plant_receive_e9_patch(fixture);
  uint8_t expected_patch[5] = {};
  std::memcpy(expected_patch, fixture.image.data() + pkt3663::kReceiveRva,
              sizeof expected_patch);
  fixture.image[pkt3663::kReceiveRva + 2] ^= 0x01;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      expected_patch, sizeof expected_patch);
  CHECK(identity.kind == pkt3663::IdentityKind::Unreadable);

  fixture = make_fixture();
  plant_receive_e9_patch(fixture);
  fixture.image[pkt3663::kReceiveRva + 5] ^= 0x01;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 5);
  CHECK(identity.kind == pkt3663::IdentityKind::AnchorMismatch);

  fixture = make_fixture();
  plant_receive_e9_patch(fixture);
  fixture.image[pkt3663::kSendAnchors[1].rva] ^= 0x01;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 5);
  CHECK(identity.kind == pkt3663::IdentityKind::AnchorMismatch);

  fixture = make_fixture();
  plant_receive_e9_patch(fixture);
  fixture.image[pkt3663::kPumpAnchors[0].rva] ^= 0x01;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 5);
  CHECK(identity.kind == pkt3663::IdentityKind::AnchorMismatch);

  fixture = make_fixture();
  plant_receive_e9_patch(fixture);
  fixture.image[pkt3663::kPumpSendCallRva] = 0x90;
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 5);
  CHECK(identity.kind == pkt3663::IdentityKind::CallTargetMismatch);

  fixture = make_fixture(pkt3663::kTimestamp3576);
  plant_receive_e9_patch(fixture);
  identity = pkt3663::identify_image_after_receive_patch(
      L"CrimsonDesert.exe", fixture.base, &read_fixture, &fixture,
      fixture.image.data() + pkt3663::kReceiveRva, 5);
  CHECK(identity.kind == pkt3663::IdentityKind::WrongTimestamp);
  CHECK(fixture.candidate_reads == 0);
}

void test_current_test_process_is_not_accepted() {
  const pkt3663::Identity identity = pkt3663::identify_process();
  CHECK(!pkt3663::identity_allows_hooks(identity));
  CHECK(identity.kind != pkt3663::IdentityKind::Match);
}

} // namespace

int main() {
  test_exact_mapped_fixture_matches();
  test_old_builds_stop_before_candidates();
  test_other_identity_fields_stop_before_candidates();
  test_unreadable_ranges_fail_closed();
  test_every_anchor_is_required_and_unique();
  test_sections_and_pdata_fail_closed();
  test_pump_calls_are_decoded();
  test_second_verification_catches_a_change();
  test_send_site_recheck_masks_only_receive_patch_window();
  test_current_test_process_is_not_accepted();
  if (g_failures != 0) {
    std::printf("%d failures\n", g_failures);
    return 1;
  }
  std::printf("pkt3663 live identity tests ok\n");
  return 0;
}
