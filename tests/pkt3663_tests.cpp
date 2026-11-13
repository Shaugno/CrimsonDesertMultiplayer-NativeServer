// Game-free tests for the offline 3663 file assessment.
// No game process is opened. The fixture image is synthetic.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include "pkt3663/assess.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

void put_u16(std::vector<uint8_t>& image, size_t at, uint16_t value) {
    std::memcpy(image.data() + at, &value, sizeof value);
}

void put_u32(std::vector<uint8_t>& image, size_t at, uint32_t value) {
    std::memcpy(image.data() + at, &value, sizeof value);
}

void put_u64(std::vector<uint8_t>& image, size_t at, uint64_t value) {
    std::memcpy(image.data() + at, &value, sizeof value);
}

void put_section(std::vector<uint8_t>& image, size_t at, const char* name, uint32_t vsize, uint32_t va, uint32_t raw_size,
                 uint32_t raw_ptr, uint32_t flags) {
    std::memset(image.data() + at, 0, 40);
    std::memcpy(image.data() + at, name, std::strlen(name));
    put_u32(image, at + 8, vsize);
    put_u32(image, at + 12, va);
    put_u32(image, at + 16, raw_size);
    put_u32(image, at + 20, raw_ptr);
    put_u32(image, at + 36, flags);
}

std::vector<uint8_t> make_image(uint32_t timestamp, uint64_t image_base, uint32_t size_of_image, bool message, bool alloc,
                                bool former) {
    std::vector<uint8_t> image(0x900, 0);
    image[0] = 'M';
    image[1] = 'Z';
    put_u32(image, 0x3C, 0x80);
    put_u32(image, 0x80, 0x00004550);
    put_u16(image, 0x86, 3);
    put_u32(image, 0x88, timestamp);
    put_u16(image, 0x94, 0xF0);
    put_u16(image, 0x98, 0x20B);
    put_u64(image, 0xB0, image_base);
    put_u32(image, 0xD0, size_of_image);
    put_u32(image, 0xD4, 0x200);
    put_section(image, 0x188, ".rdata", 0x200, 0x1000, 0x200, 0x400, 0x40000040);
    put_section(image, 0x1B0, ".text", 0x200, 0x2000, 0x200, 0x600, 0x60000020);
    put_section(image, 0x1D8, ".data", 0x100, 0x3000, 0x100, 0x800, 0xC0000040);

    if (message) {
        const char* mangled = ".?AVMoveActorReq@@";
        std::memcpy(image.data() + 0x410, mangled, std::strlen(mangled) + 1);
        put_u32(image, 0x480, 1);
        put_u32(image, 0x480 + 12, 0x1000);
        put_u32(image, 0x480 + 20, 0x1080);
        put_u64(image, 0x4F8, image_base + 0x1080);
        put_u64(image, 0x500, image_base + 0x2100);
        put_u64(image, 0x508, image_base + 0x2110);
        put_u64(image, 0x510, image_base + 0x2120);

        const int32_t lea_rel = static_cast<int32_t>(0x1100 - (0x2000 + 7));
        const int32_t mov_rel = static_cast<int32_t>(0x3000 - (0x2007 + 7));
        image[0x600] = 0x48;
        image[0x601] = 0x8D;
        image[0x602] = 0x05;
        put_u32(image, 0x603, static_cast<uint32_t>(lea_rel));
        image[0x607] = 0x48;
        image[0x608] = 0x89;
        image[0x609] = 0x05;
        put_u32(image, 0x60A, static_cast<uint32_t>(mov_rel));

        put_u64(image, 0x800, 0x142000000ULL);
        put_u32(image, 0x808, 9);
        put_u16(image, 0x80C, 0x2222);
        put_u32(image, 0x814, 0xABCDEF00);
        put_u16(image, 0x818, 7);
        put_u16(image, 0x81A, 8);
    }
    if (alloc) {
        const uint8_t mov_ecx[] = {0xB9, 0x30, 0x80, 0x00, 0x00};
        std::memcpy(image.data() + 0x620, mov_ecx, sizeof mov_ecx);
    }
    if (former) {
        const uint8_t sample[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C,
                                  0x24, 0x20, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41};
        std::memcpy(image.data() + 0x640, sample, sizeof sample);
    }
    return image;
}

const pkt3663::MessageHit* find_key(const pkt3663::Assessment& result, const char* key) {
    for (const pkt3663::MessageHit& hit : result.messages) {
        if (std::strcmp(hit.key, key) == 0) return &hit;
    }
    return nullptr;
}

void test_old_timestamps_are_not_scanned() {
    const uint32_t old[] = {pkt3663::kTimestamp2976, pkt3663::kTimestamp3576};
    for (uint32_t timestamp : old) {
        const std::vector<uint8_t> image =
            make_image(timestamp, pkt3663::kImageBase3663, pkt3663::kSizeOfImage3663, true, true, true);
        const pkt3663::Assessment result = pkt3663::assess(image);
        CHECK(result.kind == (timestamp == pkt3663::kTimestamp2976 ? pkt3663::FileKind::Rejected2976
                                                                  : pkt3663::FileKind::Rejected3576));
        CHECK(!result.scanned);
        CHECK(!result.adopted);
        CHECK(result.messages.empty());
        CHECK(result.alloc_site_count == 0);
        CHECK(result.alloc_sites.empty());
        CHECK(result.former_sample_count == 0);
        CHECK(result.former_samples.empty());
    }
}

void test_wrong_size_and_base_are_not_scanned() {
    const std::vector<uint8_t> wrong_size =
        make_image(pkt3663::kTimestamp3663, pkt3663::kImageBase3663, 0x1000, true, true, false);
    const pkt3663::Assessment size_result = pkt3663::assess(wrong_size);
    CHECK(size_result.kind == pkt3663::FileKind::RejectedOther);
    CHECK(!size_result.scanned);
    CHECK(size_result.alloc_sites.empty());

    const std::vector<uint8_t> wrong_base =
        make_image(pkt3663::kTimestamp3663, 0x180000000ULL, pkt3663::kSizeOfImage3663, true, true, false);
    const pkt3663::Assessment base_result = pkt3663::assess(wrong_base);
    CHECK(base_result.kind == pkt3663::FileKind::RejectedOther);
    CHECK(!base_result.scanned);
}

void test_unreadable_image_is_not_scanned() {
    const std::vector<uint8_t> image(16, 0);
    const pkt3663::Assessment result = pkt3663::assess(image);
    CHECK(result.kind == pkt3663::FileKind::Unreadable);
    CHECK(!result.scanned);
    CHECK(result.messages.empty());
}

void test_3663_fixture_reads_its_own_bytes() {
    const std::vector<uint8_t> image =
        make_image(pkt3663::kTimestamp3663, pkt3663::kImageBase3663, pkt3663::kSizeOfImage3663, true, true, true);
    const pkt3663::Assessment result = pkt3663::assess(image);
    CHECK(result.kind == pkt3663::FileKind::Build3663);
    CHECK(result.scanned);
    CHECK(!result.adopted);
    CHECK(result.messages.size() == 9);

    const pkt3663::MessageHit* req = find_key(result, "MoveActorReq");
    const pkt3663::MessageHit* ack = find_key(result, "MoveActorAck");
    CHECK(req != nullptr && req->found);
    CHECK(ack != nullptr && !ack->found);
    CHECK(req->classes.size() == 1);
    CHECK(req->classes[0].exact_name);
    CHECK(req->classes[0].mangled == ".?AVMoveActorReq@@");
    CHECK(req->classes[0].type_descriptor_rva == 0x1000);
    CHECK(req->classes[0].col_rvas.size() == 1 && req->classes[0].col_rvas[0] == 0x1080);
    CHECK(req->classes[0].vtables.size() == 1);
    CHECK(req->classes[0].vtables[0].rva == 0x1100);
    CHECK(req->classes[0].vtables[0].slot2 && req->classes[0].vtables[0].slot2_rva == 0x2120);
    CHECK(req->classes[0].instances.size() == 1);
    CHECK(req->classes[0].instances[0].rva == 0x3000);
    CHECK(req->classes[0].instances[0].kind_u32 == 9);
    CHECK(req->classes[0].instances[0].opcode_u16 == 0x2222);
    CHECK(req->classes[0].instances[0].flag_u32 == 0xABCDEF00);
    CHECK(req->classes[0].instances[0].min_u16 == 7);
    CHECK(req->classes[0].instances[0].max_u16 == 8);

    CHECK(result.alloc_site_count == 1);
    CHECK(result.alloc_sites.size() == 1);
    CHECK(result.alloc_sites[0].rva == 0x2020);
    CHECK(std::strcmp(result.alloc_sites[0].encoding, "mov_ecx") == 0);
    CHECK(result.former_sample_count == 1);
    CHECK(result.former_send_count == 1);
    CHECK(result.former_recv_count == 0);
    CHECK(result.former_samples.size() == 1);
    CHECK(std::strcmp(result.former_samples[0].label, "former_3576_send_sample") == 0);
    CHECK(result.former_samples[0].rva == 0x2040);
}

void test_longer_type_name_is_not_exact() {
    std::vector<uint8_t> image =
        make_image(pkt3663::kTimestamp3663, pkt3663::kImageBase3663, pkt3663::kSizeOfImage3663, true, false, false);
    const char* mangled = ".?AVTrocTrManagementRemoveActorAck@pa@@";
    std::memset(image.data() + 0x410, 0, 64);
    std::memcpy(image.data() + 0x410, mangled, std::strlen(mangled) + 1);
    const pkt3663::Assessment result = pkt3663::assess(image);
    const pkt3663::MessageHit* remove = find_key(result, "RemoveActorAck");
    const pkt3663::MessageHit* req = find_key(result, "MoveActorReq");
    CHECK(remove != nullptr && remove->found);
    CHECK(remove->classes.size() == 1);
    CHECK(!remove->classes[0].exact_name);
    CHECK(remove->classes[0].mangled == mangled);
    CHECK(req != nullptr && !req->found);
}

}  // namespace

int main() {
    test_old_timestamps_are_not_scanned();
    test_wrong_size_and_base_are_not_scanned();
    test_unreadable_image_is_not_scanned();
    test_3663_fixture_reads_its_own_bytes();
    test_longer_type_name_is_not_exact();
    if (g_failures != 0) {
        std::printf("%d failures\n", g_failures);
        return 1;
    }
    std::printf("pkt3663 tests ok\n");
    return 0;
}
