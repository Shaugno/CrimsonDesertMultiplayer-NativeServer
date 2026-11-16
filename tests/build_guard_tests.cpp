// Central build guard (src/mod/game/build_guard.h). Synthetic PE headers only:
// supported 1.0.0.2976, a different readable build, and headers that cannot be
// parsed. No game process and no live executable.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#include "mod/game/build_guard.h"
#include "mod/game/offsets.h"
#include "mod/game/player_layout.h"
#include "mod/game/stat_id.h"
#include "mod/session2/control_filter.h"

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
    int tier_a_reads = 0;
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
    guard::route_3663_reader(decision, [&] { ++counts.tier_a_reads; });
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
    CHECK(counts.tier_a_reads == 0);
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

void test_build_3663_is_recognized_but_unarmed() {
    namespace build = cdmp::game::build_3663;
    const auto bytes = make_image(build::kPeTimestamp, 0x80, build::kSizeOfImage);
    const auto identity = guard::identify_image(bytes, build::kImageBase);
    CHECK(identity.kind == guard::IdentityKind::Build3663);
    const auto text = guard::diagnostic(identity);
    CHECK(text.find("1.0.0.3663") != std::string::npos);
    CHECK(text.find("not armed") != std::string::npos);
    for (const bool guest : {false, true}) {
        const auto decision = guard::decide_startup(request(true, guest, true, identity));
        check_blocked(decision);
        CHECK(decision.build == guard::GameBuild::Build3663);
        CHECK(decision.end_guest_process == guest);
    }
    for (const auto& rejected : {
            guard::identify_image(make_image(build::kPeTimestamp, 0x80, build::kSizeOfImage - 0x1000), build::kImageBase),
            guard::identify_image(make_image(build::kPeTimestamp + 1, 0x80, build::kSizeOfImage), build::kImageBase),
            guard::identify_image(bytes, build::kImageBase + 0x1000)}) {
        CHECK(rejected.kind == guard::IdentityKind::Unsupported);
        check_blocked(guard::decide_startup(request(true, false, false, rejected)));
    }
    // Classification can copy headers only. No adopted gameplay RVA may be read.
    auto read_headers_only = [&](uintptr_t address, void* out, size_t size) {
        CHECK(address == build::kImageBase);
        CHECK(size <= bytes.size());
        if (address != build::kImageBase || size > bytes.size()) return false;
        std::memcpy(out, bytes.data(), size);
        return true;
    };
    CHECK(guard::identify_mapped(build::kImageBase, read_headers_only).kind == guard::IdentityKind::Build3663);
    // Test-host permissions remain independent of the game's recognized identity.
    const auto host = guard::decide_startup(request(false, true, true, identity));
    const auto counts = drive(host);
    CHECK(counts.reads == 0 && counts.hooks == 0 && counts.game_sources == 0);
    CHECK(counts.synthetic == 1 && counts.networks == 1 && counts.overlays == 1);
    CHECK(!host.end_guest_process);
    std::printf("Build 3663 recognized and unarmed: OK\n");
}

void test_3663_tier_dependency_matrix() {
    namespace build = cdmp::game::build_3663;
    const auto identity = guard::identify_image(make_image(build::kPeTimestamp, 0x80, build::kSizeOfImage),
                                               build::kImageBase);
    // All 256 request/evidence combinations: missing or disabled lower tiers
    // must deny every higher tier, independently of that higher tier's proof.
    for (unsigned requests = 0; requests < 16; ++requests) {
        for (unsigned evidence = 0; evidence < 16; ++evidence) {
            auto mask = [](unsigned x) { return guard::TierPermissions{
                (x & 1) != 0, (x & 2) != 0, (x & 4) != 0, (x & 8) != 0}; };
            const auto out = guard::select_3663_tiers(identity, mask(requests), mask(evidence));
            const unsigned both = requests & evidence;
            CHECK(out.tier_a_reads == ((both & 1) == 1));
            CHECK(out.tier_b_tunnel == ((both & 3) == 3));
            CHECK(out.tier_c_join == ((both & 7) == 7));
            CHECK(out.tier_d_lifecycle == ((both & 15) == 15));
            guard::StartupDecision routed;
            routed.build = guard::GameBuild::Build3663;
            static_cast<guard::TierPermissions&>(routed) = out;
            const auto effects = drive(routed);
            CHECK(effects.tier_a_reads == (out.tier_a_reads ? 1 : 0));
            CHECK(effects.reads == 0 && effects.hooks == 0 && effects.overlays == 0);
            CHECK(effects.game_sources == 0 && effects.synthetic == 0 && effects.networks == 0);
        }
    }
    for (unsigned fault = 0; fault < 7; ++fault) {
        auto bad = identity;
        switch (fault) {
            case 0: bad.size_of_image -= 0x1000; break;
            case 1: ++bad.timestamp; break;
            case 2: bad.mapped_base += 0x1000; break;
            case 3: bad.size_known = false; break;
            case 4: bad.timestamp_known = false; break;
            case 5: bad.kind = guard::IdentityKind::Unsupported; break;
            case 6: bad.kind = guard::IdentityKind::Build3576; break;
        }
        const auto denied = guard::select_3663_tiers(bad, {true,true,true,true}, {true,true,true,true});
        CHECK(!denied.tier_a_reads && !denied.tier_b_tunnel && !denied.tier_c_join && !denied.tier_d_lifecycle);
    }
    const auto actual = guard::decide_startup(request(true, false, false, identity));
    CHECK(!actual.tier_a_reads && !actual.tier_b_tunnel && !actual.tier_c_join && !actual.tier_d_lifecycle);
    check_blocked(actual);
    CHECK(cdmp::game::player_layout(identity) == nullptr);
    const auto legacy = guard::identify_image(make_image(kPeTimestamp), kImageBase);
    CHECK(cdmp::game::player_layout(legacy) == &cdmp::game::kPlayerLayout2976);
    CHECK(drive(guard::decide_startup(request(true,false,false,legacy))).tier_a_reads == 0);
    const auto unknown = guard::identify_image(make_image(kPeTimestamp + 1), kImageBase);
    CHECK(cdmp::game::player_layout(unknown) == nullptr);
    std::printf("3663 tier request/evidence and identity matrices: OK\n");
}

void test_native_health_selection() {
    // Fictional process memory only. Different registry chains select one
    // key; dirty ID padding and a neighbouring stamina stat must not select HP.
    constexpr uintptr_t base = 0x10000, entries = base + 0x3000;
    std::vector<uint8_t> memory(0x4000, 0xA5);
    auto put = [&](uintptr_t address, auto value) {
        std::memcpy(memory.data() + address - base, &value, sizeof value);
    };
    auto read = [&](uintptr_t address, void* out, size_t size) {
        if (address < base || address - base > memory.size() || size > memory.size() - (address - base))
            return false;
        std::memcpy(out, memory.data() + address - base, size);
        return true;
    };
    cdmp::game::HealthDefinitionLayout layout{0x100, 0x108, 0, 8, 0x10, 0x78};
    for (uintptr_t root : {base + 0x1000, base + 0x2000}) {
        put(root, root + 0x100);
        put(root + 0x108, root + 0x200);
        put(root + 0x210, root + 0x300);
        put(root + 0x378, uint16_t{7});
    }
    put(base + layout.normal_slot, base + 0x1000);
    put(base + layout.alternate_slot, base + 0x2000);
    auto sample = [&] { return cdmp::game::consensus_health_definition(base, layout, read); };
    const auto first = sample();
    CHECK(first && first->normal.key == 7 && first->alternate.key == 7);
    put(base + 0x2378, uint16_t{22});
    CHECK(!sample());
    put(base + 0x2378, uint16_t{7});
    put(base + layout.alternate_slot, uintptr_t{0});
    CHECK(!sample());
    put(base + layout.alternate_slot, base + 0x2000);
    put(base + 0x2210, base + 0x2400);
    put(base + 0x2478, uint16_t{7});
    const auto changed = sample();
    CHECK(first && changed && *first != *changed); // Same key, changed owning object.
    put(base + 0x2210, base + 0x2300);
    auto absent = layout;
    absent.normal_slot = 0;
    CHECK(!cdmp::game::consensus_health_definition(base, absent, read));
    CHECK(!cdmp::game::health_address(UINTPTR_MAX - 4, 8, 2));
    CHECK(!cdmp::game::health_address(0x7FFFFFFFFFFF, 0, 2));
    CHECK(!cdmp::game::health_address(0, 0, 2));
    put(base + 0x1378, uint16_t{0});
    put(base + 0x2378, uint16_t{0});
    CHECK(sample() && sample()->normal.key == 0); // Zero is a valid native key.
    put(base + 0x1378, uint16_t{7});
    put(base + 0x2378, uint16_t{7});
    CHECK(!cdmp::game::consensus_health_definition(base, layout,
        [](uintptr_t, void*, size_t) { return false; }));

    put(entries, uint16_t{7});
    put(entries + 8, int64_t{12000});
    put(entries + 0x18, int64_t{20000});
    put(entries + 0x90, uint16_t{22});
    auto health = [&](uint16_t key = 7, uint32_t count = 2, float units = 1000.0f) {
        return cdmp::game::read_unique_native_health(entries, count, 0x90, 0, 8, 0x18, key, units, read);
    };
    auto value = health();
    CHECK(value && value->current == 12.0f && value->maximum == 20.0f);
    CHECK(!health(9));
    put(entries + 0x90, uint16_t{7});
    CHECK(!health());
    put(entries + 0x90, uint16_t{22});
    for (int64_t invalid : {int64_t{-1}, int64_t{20001}}) {
        put(entries + 8, invalid);
        CHECK(!health());
    }
    put(entries + 8, int64_t{0});
    CHECK(health() && health()->current == 0); // Dead/empty current HP is valid.
    put(entries + 0x18, int64_t{0});
    CHECK(!health());
    put(entries + 0x18, int64_t{20000});
    CHECK(!health(7, 0) && !health(7, 65));
    CHECK(!health(7, 2, 0) && !health(7, 2, std::numeric_limits<float>::quiet_NaN()));
    CHECK(!health(7, 2, std::numeric_limits<float>::denorm_min()));
    CHECK(!cdmp::game::read_unique_native_health(entries, 1, 8, 0, 8, 0x18, 7, 1000, read));
    CHECK(!cdmp::game::read_unique_native_health(0x7FFFFFFFFFF0, 1, 0x90, 0, 8, 0x18, 7, 1000, read));
    CHECK(!cdmp::game::kPlayerLayout2976.native_hp_definitions);
    std::printf("Native HP consensus, chain-change, ambiguity and bounds: OK (synthetic only)\n");
}

}  // namespace

int main() {
    {
        uint8_t padded_id[8] = {0,0,0xAB,0xCD,0xEF,0x12,0x34,0x56};
        size_t copied = 0;
        auto read = [&](uintptr_t, void* out, size_t size) {
            copied = size;
            std::memcpy(out, padded_id, size);
            return true;
        };
        CHECK(cdmp::game::read_stat_id(1,2,read) == 0);
        CHECK(copied == 2);
        CHECK(cdmp::game::read_stat_id(1,8,read) != 0);
        CHECK(copied == 8);
        copied = 0;
        CHECK(!cdmp::game::read_stat_id(1,4,read));
        CHECK(copied == 0);
        CHECK(!cdmp::game::read_stat_id(1,2,[](uintptr_t,void*,size_t) { return false; }));
    }
    test_3663_tier_dependency_matrix();
    test_native_health_selection();
    {
        using cdmp::session2::blocked_native_control;
        constexpr cdmp::session2::NativeControlOpcodes legacy{2790,2038,2004,2125};
        constexpr cdmp::session2::NativeControlOpcodes target{2447,2042,2004,2150};
        CHECK(blocked_native_control(0, {}));
        CHECK(blocked_native_control(2001, {}));
        CHECK(blocked_native_control(65535, {}));
        CHECK(blocked_native_control(2001, {2447,2042,2004,0}));
        for (auto native : {legacy, target}) {
            CHECK(blocked_native_control(native.reserve_shutdown, native));
            CHECK(blocked_native_control(native.shutdown_timer, native));
            CHECK(blocked_native_control(native.pointer_request, native));
            CHECK(blocked_native_control(native.set_field_time, native));
            for (uint16_t id = 1001; id <= 1008; ++id) CHECK(blocked_native_control(id, native));
            CHECK(!blocked_native_control(1000, native) && !blocked_native_control(1009, native));
            CHECK(!blocked_native_control(0, native) && !blocked_native_control(65535, native));
            CHECK(!blocked_native_control(2001, native)); // Legitimate guest logout remains eligible.
        }
        std::printf("Native control IDs and closed connection interval policy: OK (synthetic only)\n");
    }
    test_supported();
    test_unsupported();
    test_unreadable();
    test_mapped_reader();
    test_host_keeps_explicit_paths();
    test_build_3576_stays_off_the_2976_path();
    test_build_3663_is_recognized_but_unarmed();
    if (g_failures == 0) std::printf("All build guard tests passed\n");
    return g_failures == 0 ? 0 : 1;
}
