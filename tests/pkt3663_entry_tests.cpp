// Game-free tests for the 3663 packet-entry file verifier.
// The fixture is a synthetic PE image; no process or game file is opened.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include "pkt3663/assess.h"
#include "pkt3663/candidates.h"
#include "pkt3663/entry_verify.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

constexpr uint32_t kTextVa = 0x3A8000;
constexpr size_t kTextRaw = 0x800;
constexpr size_t kExceptionRaw = 0x400;

void put_u16(std::vector<uint8_t>& image, size_t at, uint16_t value) {
    std::memcpy(image.data() + at, &value, sizeof value);
}

void put_u32(std::vector<uint8_t>& image, size_t at, uint32_t value) {
    std::memcpy(image.data() + at, &value, sizeof value);
}

void put_u64(std::vector<uint8_t>& image, size_t at, uint64_t value) {
    std::memcpy(image.data() + at, &value, sizeof value);
}

void put_section(std::vector<uint8_t>& image, size_t at, const char* name, uint32_t vsize, uint32_t va,
                 uint32_t raw_size, uint32_t raw_ptr, uint32_t flags) {
    std::memset(image.data() + at, 0, 40);
    std::memcpy(image.data() + at, name, std::strlen(name));
    put_u32(image, at + 8, vsize);
    put_u32(image, at + 12, va);
    put_u32(image, at + 16, raw_size);
    put_u32(image, at + 20, raw_ptr);
    put_u32(image, at + 36, flags);
}

size_t text_offset(uint32_t rva) {
    return kTextRaw + static_cast<size_t>(rva - kTextVa);
}

template <size_t N>
void put_anchors(std::vector<uint8_t>& image, const std::array<pkt3663::ByteAnchor, N>& anchors) {
    for (const pkt3663::ByteAnchor& anchor : anchors) {
        std::memcpy(image.data() + text_offset(anchor.rva), anchor.bytes, anchor.size);
    }
}

void put_function(std::vector<uint8_t>& image, size_t index, uint32_t begin, uint32_t end) {
    const size_t row = kExceptionRaw + index * 12;
    put_u32(image, row, begin);
    put_u32(image, row + 4, end);
    put_u32(image, row + 8, 0);
}

void put_call(std::vector<uint8_t>& image, uint32_t call_rva, uint32_t target_rva) {
    const int64_t displacement = static_cast<int64_t>(target_rva) - (static_cast<int64_t>(call_rva) + 5);
    image[text_offset(call_rva)] = 0xE8;
    put_u32(image, text_offset(call_rva) + 1, static_cast<uint32_t>(static_cast<int32_t>(displacement)));
}

std::vector<uint8_t> make_image(uint32_t timestamp = pkt3663::kTimestamp3663,
                                uint64_t image_base = pkt3663::kImageBase3663,
                                uint32_t size_of_image = pkt3663::kSizeOfImage3663) {
    std::vector<uint8_t> image(0x3000, 0);
    image[0] = 'M';
    image[1] = 'Z';
    put_u32(image, 0x3C, 0x80);
    put_u32(image, 0x80, 0x00004550);
    put_u16(image, 0x86, 2);
    put_u32(image, 0x88, timestamp);
    put_u16(image, 0x94, 0xF0);
    put_u16(image, 0x98, 0x20B);
    put_u64(image, 0xB0, image_base);
    put_u32(image, 0xD0, size_of_image);
    put_u32(image, 0xD4, 0x400);
    put_u32(image, 0x104, 16);
    put_u32(image, 0x120, 0x1000);
    put_u32(image, 0x124, 36);
    put_section(image, 0x188, ".pdata", 0x400, 0x1000, 0x400, static_cast<uint32_t>(kExceptionRaw), 0x40000040);
    put_section(image, 0x1B0, ".text", 0x2800, kTextVa, 0x2800, static_cast<uint32_t>(kTextRaw), 0x60000020);

    put_function(image, 0, pkt3663::kPumpRva, pkt3663::kPumpEndRva);
    put_function(image, 1, pkt3663::kSendRva, pkt3663::kSendEndRva);
    put_function(image, 2, pkt3663::kReceiveRva, pkt3663::kReceiveEndRva);
    put_anchors(image, pkt3663::kPumpAnchors);
    put_anchors(image, pkt3663::kSendAnchors);
    put_anchors(image, pkt3663::kReceiveAnchors);
    put_call(image, pkt3663::kPumpReceiveCallRva, pkt3663::kReceiveRva);
    put_call(image, pkt3663::kPumpSendCallRva, pkt3663::kSendRva);
    return image;
}

void test_complete_fixture_is_ready_but_not_adopted() {
    const pkt3663::EntryVerification result = pkt3663::verify_entries(make_image());
    CHECK(result.status == pkt3663::EntryStatus::VerifiedFileCandidates);
    CHECK(result.identity_ok);
    CHECK(result.send_range_ok && result.receive_range_ok && result.pump_range_ok);
    CHECK(result.send_anchors_ok && result.receive_anchors_ok && result.pump_anchors_ok);
    CHECK(result.role_calls_ok);
    CHECK(result.passive_capture_ready);
    CHECK(!result.adopted);
    CHECK(result.failed_anchor_rva == 0);
    CHECK(std::strcmp(pkt3663::entry_status_name(result.status), "verified-file-candidates") == 0);
}

void test_old_builds_cannot_reach_candidates() {
    const uint32_t old[] = {pkt3663::kTimestamp2976, pkt3663::kTimestamp3576};
    for (uint32_t timestamp : old) {
        const pkt3663::EntryVerification result = pkt3663::verify_entries(make_image(timestamp));
        CHECK(result.status == pkt3663::EntryStatus::WrongIdentity);
        CHECK(!result.identity_ok);
        CHECK(!result.send_range_ok && !result.receive_range_ok && !result.pump_range_ok);
        CHECK(!result.send_anchors_ok && !result.receive_anchors_ok && !result.pump_anchors_ok);
        CHECK(!result.passive_capture_ready);
        CHECK(!result.adopted);
    }
}

void test_wrong_base_and_size_fail_before_candidates() {
    pkt3663::EntryVerification result =
        pkt3663::verify_entries(make_image(pkt3663::kTimestamp3663, 0x180000000ULL, pkt3663::kSizeOfImage3663));
    CHECK(result.status == pkt3663::EntryStatus::WrongIdentity);
    CHECK(!result.passive_capture_ready);
    result = pkt3663::verify_entries(make_image(pkt3663::kTimestamp3663, pkt3663::kImageBase3663, 0x1234000));
    CHECK(result.status == pkt3663::EntryStatus::WrongIdentity);
    CHECK(!result.passive_capture_ready);
    result = pkt3663::verify_entries(make_image(pkt3663::kTimestamp3663, pkt3663::kImageBase3663, 0x7FFFFFFF));
    CHECK(result.status == pkt3663::EntryStatus::WrongIdentity);
    CHECK(!result.passive_capture_ready);
}

void test_function_ranges_are_required() {
    std::vector<uint8_t> image = make_image();
    put_u32(image, 0x124, 0);
    pkt3663::EntryVerification result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::MissingFunctionTable);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u32(image, kExceptionRaw + 12 + 4, pkt3663::kSendEndRva + 1);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::SendRangeMismatch);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u32(image, kExceptionRaw + 24 + 4, pkt3663::kReceiveEndRva + 1);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::ReceiveRangeMismatch);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u32(image, kExceptionRaw + 4, pkt3663::kPumpEndRva + 1);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::PumpRangeMismatch);
    CHECK(!result.passive_capture_ready);
}

void test_every_candidate_group_fails_closed() {
    std::vector<uint8_t> image = make_image();
    image[text_offset(pkt3663::kSendAnchors[1].rva)] ^= 0x01;
    pkt3663::EntryVerification result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::SendAnchorMismatch);
    CHECK(result.failed_anchor_rva == pkt3663::kSendAnchors[1].rva);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    image[text_offset(pkt3663::kReceiveAnchors[2].rva)] ^= 0x01;
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::ReceiveAnchorMismatch);
    CHECK(result.failed_anchor_rva == pkt3663::kReceiveAnchors[2].rva);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    image[text_offset(pkt3663::kPumpAnchors[1].rva) + pkt3663::kPumpAnchors[1].size - 1] ^= 0x01;
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::PumpAnchorMismatch);
    CHECK(result.failed_anchor_rva == pkt3663::kPumpAnchors[1].rva);
    CHECK(!result.passive_capture_ready);
}

void test_all_discriminating_signature_bytes_fail_closed() {
    for (const pkt3663::ByteAnchor* anchor : {&pkt3663::kSendAnchors[0], &pkt3663::kReceiveAnchors[0],
                                              &pkt3663::kPumpAnchors[0]}) {
        for (size_t i = 0; i < anchor->size; ++i) {
            std::vector<uint8_t> image = make_image();
            image[text_offset(anchor->rva) + i] ^= 0x01;
            const pkt3663::EntryVerification result = pkt3663::verify_entries(image);
            CHECK(!result.passive_capture_ready);
            CHECK(!result.adopted);
        }
    }
}

void test_role_calls_are_decoded_not_wildcarded() {
    const uint32_t calls[] = {pkt3663::kPumpReceiveCallRva, pkt3663::kPumpSendCallRva};
    for (uint32_t call : calls) {
        for (size_t i = 0; i < 5; ++i) {
            std::vector<uint8_t> image = make_image();
            image[text_offset(call) + i] ^= 0x01;
            const pkt3663::EntryVerification result = pkt3663::verify_entries(image);
            CHECK(result.status == pkt3663::EntryStatus::CallTargetMismatch);
            CHECK(!result.role_calls_ok);
            CHECK(!result.passive_capture_ready);
        }
    }
}

void test_full_signatures_must_be_unique_and_fixed() {
    std::vector<uint8_t> image = make_image();
    constexpr uint32_t duplicate_rva = kTextVa + 0x100;
    std::memcpy(image.data() + text_offset(duplicate_rva), pkt3663::kSendEntry, sizeof pkt3663::kSendEntry);
    pkt3663::EntryVerification result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::AmbiguousAnchor);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    std::memcpy(image.data() + text_offset(duplicate_rva), pkt3663::kSendEntry, sizeof pkt3663::kSendEntry);
    image[text_offset(pkt3663::kSendRva)] ^= 0x01;
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::SendAnchorMismatch);
    CHECK(!result.passive_capture_ready);
}

void test_malformed_or_non_executable_images_fail_closed() {
    std::vector<uint8_t> image = make_image();
    put_u32(image, 0x1B0 + 36, 0x40000040);
    pkt3663::EntryVerification result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::NotExecutable);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u32(image, 0x1B0 + 16, 0x4000);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::MalformedImage);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u32(image, 0x188 + 12, kTextVa);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::MalformedImage);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u16(image, 0x86, 3);
    put_section(image, 0x1D8, ".bss", 0x1000, kTextVa + 0x100, 0, 0, 0xC0000080);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::MalformedImage);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u32(image, 0x124, 48);
    put_function(image, 2, pkt3663::kSendRva, pkt3663::kSendEndRva);
    put_function(image, 3, pkt3663::kReceiveRva, pkt3663::kReceiveEndRva);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::MalformedImage);
    CHECK(!result.passive_capture_ready);

    image = make_image();
    put_u32(image, 0x124, 48);
    put_function(image, 2, pkt3663::kSendRva, pkt3663::kSendEndRva + 1);
    put_function(image, 3, pkt3663::kReceiveRva, pkt3663::kReceiveEndRva);
    result = pkt3663::verify_entries(image);
    CHECK(result.status == pkt3663::EntryStatus::MalformedImage);
    CHECK(!result.passive_capture_ready);
}

}  // namespace

int main() {
    test_complete_fixture_is_ready_but_not_adopted();
    test_old_builds_cannot_reach_candidates();
    test_wrong_base_and_size_fail_before_candidates();
    test_function_ranges_are_required();
    test_every_candidate_group_fails_closed();
    test_all_discriminating_signature_bytes_fail_closed();
    test_role_calls_are_decoded_not_wildcarded();
    test_full_signatures_must_be_unique_and_fixed();
    test_malformed_or_non_executable_images_fail_closed();
    if (g_failures != 0) {
        std::printf("%d failures\n", g_failures);
        return 1;
    }
    std::printf("pkt3663 entry tests ok\n");
    return 0;
}
