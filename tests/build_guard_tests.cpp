// Central build guard (src/mod/game/build_guard.h). Synthetic PE headers only:
// supported 1.0.0.2976, a different readable build, and headers that cannot be
// parsed. No game process and no live executable.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "mod/game/build_guard.h"
#include "mod/game/offsets.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

namespace guard = cdmp::game::build_guard;
using cdmp::game::build_2976::kImageBase;
using cdmp::game::build_2976::kPeTimestamp;

void write_u16(std::vector<uint8_t>& bytes, size_t offset, uint16_t value) {
    std::memcpy(bytes.data() + offset, &value, sizeof value);
}

void write_u32(std::vector<uint8_t>& bytes, size_t offset, uint32_t value) {
    std::memcpy(bytes.data() + offset, &value, sizeof value);
}

// Minimal PE32+ header. e_lfanew sits past the first 0x40 bytes so the mapped
// reader has to copy twice, the way the live path does.
std::vector<uint8_t> make_image(uint32_t timestamp, uint32_t e_lfanew = 0x80, uint32_t size_of_image = 0x100000) {
    std::vector<uint8_t> bytes(static_cast<size_t>(e_lfanew) + 24 + 60, 0);
    write_u16(bytes, 0, 0x5A4D);
    write_u32(bytes, 0x3C, e_lfanew);
    write_u32(bytes, e_lfanew, 0x00004550);
    write_u32(bytes, static_cast<size_t>(e_lfanew) + 8, timestamp);
    write_u16(bytes, static_cast<size_t>(e_lfanew) + 24, 0x020B);
    write_u32(bytes, static_cast<size_t>(e_lfanew) + 24 + 56, size_of_image);
    return bytes;
}

struct Counts {
    int reads = 0;
    int hooks = 0;
    int overlays = 0;
    int game_sources = 0;
    int synthetic = 0;
    int networks = 0;
};

// Same two calls the worker makes: apply_startup for game reads and hooks,
// start_multiplayer for the player source and run_network. A synthetic state is
// sampled only from run_network, which is the network callback.
Counts drive(const guard::StartupDecision& decision, bool net_enabled = true) {
    Counts counts;
    guard::apply_startup(
        decision, [&] { ++counts.reads; }, [&] { ++counts.hooks; }, [&] { ++counts.overlays; });
    guard::start_multiplayer(
        decision, net_enabled, [&] { ++counts.game_sources; }, [&] { ++counts.synthetic; },
        [&] { ++counts.networks; });
    return counts;
}

guard::StartupRequest request(bool in_game, bool guest, bool test_overlay, const guard::ImageIdentity& identity) {
    guard::StartupRequest req;
    req.in_game = in_game;
    req.guest_role = guest;
    req.test_overlay = test_overlay;
    req.identity = identity;
    return req;
}

void check_blocked(const guard::StartupDecision& decision) {
    const Counts counts = drive(decision);
    CHECK(counts.reads == 0);
    CHECK(counts.hooks == 0);
    CHECK(counts.overlays == 0);
    CHECK(counts.game_sources == 0);
    CHECK(counts.synthetic == 0);
    CHECK(counts.networks == 0);
    CHECK(!decision.read_game_addresses);
    CHECK(!decision.install_hooks);
    CHECK(!decision.install_test_overlay);
    CHECK(!decision.run_network);
    CHECK(decision.player_feed == guard::PlayerFeed::Off);
}

void test_supported() {
    const auto bytes = make_image(kPeTimestamp);
    const auto identity = guard::identify_image(bytes, kImageBase);
    CHECK(identity.kind == guard::IdentityKind::Supported);
    CHECK(identity.timestamp_known && identity.timestamp == kPeTimestamp);
    CHECK(identity.size_known && identity.size_of_image == 0x100000);
    const std::string text = guard::diagnostic(identity);
    CHECK(text.find("Supported") != std::string::npos);
    CHECK(text.find("1.0.0.2976") != std::string::npos);
    CHECK(text.find("6AB28F00") != std::string::npos);
    CHECK(text.find("140000000") != std::string::npos);

    for (const bool guest : {false, true}) {
        const auto decision = guard::decide_startup(request(true, guest, true, identity));
        const Counts counts = drive(decision);
        CHECK(counts.reads == 1);
        CHECK(counts.hooks == 1);
        CHECK(counts.overlays == 0);
        CHECK(counts.game_sources == 1);
        CHECK(counts.synthetic == 0);
        CHECK(counts.networks == 1);
        CHECK(decision.read_game_addresses);
        CHECK(decision.install_hooks);
        CHECK(!decision.install_test_overlay);
        CHECK(decision.run_network);
        CHECK(decision.player_feed == guard::PlayerFeed::Game);
        CHECK(decision.build == guard::GameBuild::Build2976);
        CHECK(!decision.end_guest_process);
        const Counts quiet = drive(decision, false);
        CHECK(quiet.game_sources == 1);
        CHECK(quiet.synthetic == 0);
        CHECK(quiet.networks == 0);
    }
    std::printf("Supported identity: OK\n");
}

void test_unsupported() {
    const uint32_t installed = 0x6AED2E85;  // 3576 timestamp, but SizeOfImage is not 0x13CC4000
    const auto wrong_time = guard::identify_image(make_image(installed), kImageBase);
    CHECK(wrong_time.kind == guard::IdentityKind::Unsupported);
    CHECK(wrong_time.timestamp == installed);
    const std::string time_text = guard::diagnostic(wrong_time);
    CHECK(time_text.find("Unsupported") != std::string::npos);
    CHECK(time_text.find("1.0.0.2976") != std::string::npos);
    CHECK(time_text.find("6AB28F00") != std::string::npos);
    CHECK(time_text.find("140000000") != std::string::npos);
    CHECK(time_text.find("6AED2E85") != std::string::npos);
    CHECK(time_text.find("stay off") != std::string::npos);

    const auto guest = guard::decide_startup(request(true, true, true, wrong_time));
    check_blocked(guest);
    CHECK(guest.end_guest_process);
    CHECK(!guest.run_network);
    CHECK(guest.player_feed == guard::PlayerFeed::Off);

    const auto other = guard::decide_startup(request(true, false, true, wrong_time));
    check_blocked(other);
    CHECK(!other.end_guest_process);
    CHECK(!other.run_network);
    CHECK(other.player_feed == guard::PlayerFeed::Off);

    const auto wrong_base = guard::identify_image(make_image(kPeTimestamp), kImageBase + 0x1000);
    CHECK(wrong_base.kind == guard::IdentityKind::Unsupported);
    const auto relocated = guard::decide_startup(request(true, true, false, wrong_base));
    check_blocked(relocated);
    CHECK(relocated.end_guest_process);
    CHECK(guard::diagnostic(wrong_base).find("140001000") != std::string::npos);
    std::printf("Unsupported identity: OK\n");
}

void test_unreadable() {
    const guard::ImageIdentity empty = guard::identify_image({}, kImageBase);
    CHECK(empty.kind == guard::IdentityKind::Unreadable);
    CHECK(!empty.timestamp_known);

    std::vector<uint8_t> bad_dos = make_image(kPeTimestamp);
    bad_dos[0] = 'X';
    CHECK(guard::identify_image(bad_dos, kImageBase).kind == guard::IdentityKind::Unreadable);

    std::vector<uint8_t> bad_lfanew = make_image(kPeTimestamp);
    write_u32(bad_lfanew, 0x3C, 0);
    CHECK(guard::identify_image(bad_lfanew, kImageBase).kind == guard::IdentityKind::Unreadable);
    write_u32(bad_lfanew, 0x3C, 0xFFFFFFF0u);
    CHECK(guard::identify_image(bad_lfanew, kImageBase).kind == guard::IdentityKind::Unreadable);

    std::vector<uint8_t> bad_sig = make_image(kPeTimestamp);
    write_u32(bad_sig, 0x80, 0);
    CHECK(guard::identify_image(bad_sig, kImageBase).kind == guard::IdentityKind::Unreadable);

    std::vector<uint8_t> truncated = make_image(kPeTimestamp);
    truncated.resize(0x80 + 24);
    CHECK(guard::identify_image(truncated, kImageBase).kind == guard::IdentityKind::Unreadable);

    std::vector<uint8_t> not_plus = make_image(kPeTimestamp);
    write_u16(not_plus, 0x80 + 24, 0x010B);
    CHECK(guard::identify_image(not_plus, kImageBase).kind == guard::IdentityKind::Unsupported);

    for (const guard::ImageIdentity& identity : {empty, guard::identify_image(bad_dos, kImageBase),
                                                 guard::identify_image(bad_sig, kImageBase),
                                                 guard::identify_image(truncated, kImageBase)}) {
        const std::string text = guard::diagnostic(identity);
        CHECK(text.find("unreadable") != std::string::npos);
        CHECK(text.find("1.0.0.2976") != std::string::npos);
        CHECK(text.find("6AB28F00") != std::string::npos);
        CHECK(text.find("140000000") != std::string::npos);
        const auto decision = guard::decide_startup(request(true, true, true, identity));
        check_blocked(decision);
        CHECK(decision.end_guest_process);
        CHECK(!decision.run_network);
    }
    std::printf("Unreadable identity: OK\n");
}

void test_mapped_reader() {
    const auto bytes = make_image(kPeTimestamp);
    auto read_ok = [&](uintptr_t addr, void* out, size_t size) {
        if (addr != kImageBase || addr + size < addr || size > bytes.size()) return false;
        std::memcpy(out, bytes.data(), size);
        return true;
    };
    const auto supported = guard::identify_mapped(kImageBase, read_ok);
    CHECK(supported.kind == guard::IdentityKind::Supported);

    auto read_fail = [](uintptr_t, void*, size_t) { return false; };
    CHECK(guard::identify_mapped(kImageBase, read_fail).kind == guard::IdentityKind::Unreadable);

    auto read_dos_only = [&](uintptr_t addr, void* out, size_t size) {
        if (addr != kImageBase || size > 0x40) return false;
        std::memcpy(out, bytes.data(), size);
        return true;
    };
    const auto partial = guard::identify_mapped(kImageBase, read_dos_only);
    CHECK(partial.kind == guard::IdentityKind::Unreadable);
    CHECK(!partial.timestamp_known);

    const auto counts = drive(guard::decide_startup(request(true, false, false, partial)));
    CHECK(counts.reads == 0);
    CHECK(counts.hooks == 0);
    CHECK(counts.synthetic == 0);
    CHECK(counts.networks == 0);
    std::printf("Mapped reader: OK\n");
}

void test_host_keeps_explicit_paths() {
    const auto unsupported = guard::identify_image(make_image(0x6AED2E85), 0x140000000);
    const auto with_overlay = guard::decide_startup(request(false, true, true, unsupported));
    const Counts overlay_counts = drive(with_overlay);
    CHECK(overlay_counts.reads == 0);
    CHECK(overlay_counts.hooks == 0);
    CHECK(overlay_counts.overlays == 1);
    CHECK(overlay_counts.game_sources == 0);
    CHECK(overlay_counts.synthetic == 1);
    CHECK(overlay_counts.networks == 1);
    CHECK(with_overlay.install_test_overlay);
    CHECK(!with_overlay.read_game_addresses);
    CHECK(!with_overlay.install_hooks);
    CHECK(with_overlay.run_network);
    CHECK(with_overlay.player_feed == guard::PlayerFeed::Synthetic);
    CHECK(!with_overlay.end_guest_process);

    const auto plain = guard::decide_startup(request(false, false, false, guard::identify_image({}, 0)));
    const Counts plain_counts = drive(plain);
    CHECK(plain_counts.reads == 0);
    CHECK(plain_counts.hooks == 0);
    CHECK(plain_counts.overlays == 0);
    CHECK(plain_counts.synthetic == 1);
    CHECK(plain_counts.networks == 1);
    CHECK(plain.run_network);
    CHECK(plain.player_feed == guard::PlayerFeed::Synthetic);
    CHECK(!plain.end_guest_process);
    const Counts net_off = drive(plain, false);
    CHECK(net_off.synthetic == 1);
    CHECK(net_off.networks == 0);
    CHECK(net_off.reads == 0);
    CHECK(net_off.hooks == 0);
    std::printf("Test host: OK\n");
}

void test_build_3576_stays_off_the_2976_path() {
    using cdmp::game::build_3576::kImageBase;
    using cdmp::game::build_3576::kPeTimestamp;
    using cdmp::game::build_3576::kSizeOfImage;
    // The unadopted send candidate. This test fails if classification reads it.
    constexpr uintptr_t kSendRva = 0x3A6310;

    const auto wrong_size = guard::identify_image(make_image(kPeTimestamp), kImageBase);
    CHECK(wrong_size.kind == guard::IdentityKind::Unsupported);
    CHECK(wrong_size.size_of_image == 0x100000);

    const auto image = make_image(kPeTimestamp, 0x80, kSizeOfImage);
    const auto identity = guard::identify_image(image, kImageBase);
    CHECK(identity.kind == guard::IdentityKind::Build3576);
    CHECK(identity.timestamp == kPeTimestamp);
    CHECK(identity.size_of_image == kSizeOfImage);
    const std::string text = guard::diagnostic(identity);
    CHECK(text.find("1.0.0.3576") != std::string::npos);
    CHECK(text.find("6AED2E85") != std::string::npos);
    CHECK(text.find("not armed") != std::string::npos);

    // The 2976 timestamp is classified from the header alone, including when
    // the synthetic image carries the 3576 size. No entry signature is read.
    const auto old_build = guard::identify_image(make_image(0x6AB28F00, 0x80, kSizeOfImage), kImageBase);
    CHECK(old_build.kind == guard::IdentityKind::Supported);
    CHECK(old_build.kind != guard::IdentityKind::Build3576);

    int entry_reads = 0;
    auto read_headers = [&](uintptr_t addr, void* out, size_t size) {
        if (addr == kImageBase + kSendRva) {
            ++entry_reads;
            return false;
        }
        if (addr != kImageBase || size > image.size()) return false;
        std::memcpy(out, image.data(), size);
        return true;
    };
    CHECK(guard::identify_mapped(kImageBase, read_headers).kind == guard::IdentityKind::Build3576);
    const auto old_bytes = make_image(0x6AB28F00);
    auto read_old = [&](uintptr_t addr, void* out, size_t size) {
        if (addr == kImageBase + kSendRva) {
            ++entry_reads;
            return false;
        }
        if (addr != kImageBase || size > old_bytes.size()) return false;
        std::memcpy(out, old_bytes.data(), size);
        return true;
    };
    CHECK(guard::identify_mapped(kImageBase, read_old).kind == guard::IdentityKind::Supported);
    CHECK(entry_reads == 0);

    const auto host = guard::decide_startup(request(true, false, false, identity));
    check_blocked(host);
    CHECK(host.build == guard::GameBuild::Build3576);
    CHECK(!host.end_guest_process);
    int rva_calls = 0;
    guard::apply_startup(
        host, [&] { rva_calls += static_cast<int>(cdmp::game::build_2976::kWorldSystemSlot); },
        [&] { rva_calls += static_cast<int>(cdmp::game::build_2976::kStaticPlayerPos); }, [] {});
    CHECK(rva_calls == 0);

    const auto guest = guard::decide_startup(request(true, true, false, identity));
    check_blocked(guest);
    CHECK(guest.build == guard::GameBuild::Build3576);
    CHECK(guest.end_guest_process);
    std::printf("Build 3576 identity: OK\n");
}

}  // namespace

int main() {
    test_supported();
    test_unsupported();
    test_unreadable();
    test_mapped_reader();
    test_host_keeps_explicit_paths();
    test_build_3576_stays_off_the_2976_path();
    if (g_failures == 0) std::printf("All build guard tests passed\n");
    return g_failures == 0 ? 0 : 1;
}
