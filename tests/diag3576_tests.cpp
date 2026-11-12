// 3576 diagnostic sampler. Fake memory only: no game process, no hooks.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <string>

#include "diag3576/reader.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

struct Scene : diag3576::Memory {
    uint64_t base = diag3576::kExpectedImageBase;
    uint64_t size = 0x13CC4000;
    std::wstring path = L"D:\\SteamLibrary\\CrimsonDesert.exe";
    bool alive_flag = true;
    std::map<uint64_t, uint8_t> bytes;
    std::map<uint64_t, std::string> classes;
    std::function<void(uint64_t, size_t)> before;

    bool alive() const override { return alive_flag; }
    uint64_t module_base() const override { return base; }
    uint64_t module_size() const override { return size; }
    std::wstring module_path() const override { return path; }
    std::string rtti_name(uint64_t object) const override {
        const auto it = classes.find(object);
        return it == classes.end() ? std::string{} : it->second;
    }
    bool read(uint64_t addr, void* out, size_t n) const override {
        if (before) before(addr, n);
        if (!alive_flag) return false;
        auto* dst = static_cast<uint8_t*>(out);
        for (size_t i = 0; i < n; ++i) {
            const auto it = bytes.find(addr + i);
            if (it == bytes.end()) return false;
            dst[i] = it->second;
        }
        return true;
    }

    void poke(uint64_t addr, const void* data, size_t n) {
        const auto* src = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < n; ++i) bytes[addr + i] = src[i];
    }
    void poke_u16(uint64_t addr, uint16_t value) { poke(addr, &value, sizeof value); }
    void poke_u32(uint64_t addr, uint32_t value) { poke(addr, &value, sizeof value); }
    void poke_u64(uint64_t addr, uint64_t value) { poke(addr, &value, sizeof value); }
    void poke_vec(uint64_t addr, diag3576::Vec4 value) {
        float f[4] = {value.x, value.y, value.z, value.w};
        poke(addr, f, sizeof f);
    }
};

constexpr uint64_t kSlotRva = 0x71C09A8;
constexpr uint64_t kManager = 0x100000000100;
constexpr uint64_t kActor = 0x100000000200;
constexpr uint64_t kTable = 0x100000000300;
constexpr uint64_t kCharacter = 0x100000000400;
constexpr uint64_t kInner = 0x100000000500;
constexpr uint64_t kPhysics = 0x100000000600;
constexpr uint64_t kManagerB = 0x100000000700;

void plant_identity(Scene& scene, uint32_t timestamp) {
    scene.poke_u16(scene.base, 0x5A4D);
    scene.poke_u32(scene.base + 0x3C, 0x80);
    scene.poke_u32(scene.base + 0x80, 0x00004550);
    scene.poke_u32(scene.base + 0x88, timestamp);
}

void plant_chain(Scene& scene, uint64_t manager) {
    const uint64_t slot = scene.base + kSlotRva;
    scene.poke_u64(slot, manager);
    scene.poke_u64(manager + diag3576::kHopActor, kActor);
    scene.poke_u64(kActor + diag3576::kHopComponentTable, kTable);
    scene.poke_u64(kTable + diag3576::kHopCharacter, kCharacter);
    scene.poke_u64(kCharacter + diag3576::kHopInner, kInner);
    scene.poke_u64(kInner + diag3576::kHopPhysics, kPhysics);
    scene.classes[manager] = std::string(diag3576::kRttiManager);
    scene.classes[kActor] = std::string(diag3576::kRttiActor);
    scene.classes[kCharacter] = std::string(diag3576::kRttiCharacter);
}

diag3576::Vec4 local_vec() { return {10.f, 20.f, 30.f, 0.f}; }
diag3576::Vec4 origin_vec() { return {-8000.f, 0.f, -1000.f, 0.f}; }

void plant_positions(Scene& scene, diag3576::Vec4 origin, diag3576::Vec4 static_pos) {
    scene.poke_vec(kPhysics + diag3576::kHopLocal, local_vec());
    scene.poke_vec(scene.base + diag3576::kOriginRva, origin);
    scene.poke_vec(scene.base + diag3576::kStaticRva, static_pos);
}

Scene make_scene() {
    Scene scene;
    plant_identity(scene, diag3576::kDiagnosticTimestamp);
    plant_chain(scene, kManager);
    const diag3576::Vec4 world = diag3576::add(local_vec(), origin_vec());
    diag3576::Vec4 static_pos = world;
    static_pos.y -= 0.25f;
    plant_positions(scene, origin_vec(), static_pos);
    return scene;
}

void test_addition_leaves_y() {
    const diag3576::Vec4 local = local_vec();
    const diag3576::Vec4 origin = origin_vec();
    const diag3576::Vec4 world = diag3576::add(local, origin);
    CHECK(world.x == -7990.f);
    CHECK(world.y == 20.f);
    CHECK(world.z == -970.f);
    CHECK(world.w == 0.f);

    diag3576::Vec4 static_pos = world;
    static_pos.y -= 0.25f;
    const diag3576::Vec4 residual = diag3576::sub(static_pos, world);
    CHECK(residual.x == 0.f);
    CHECK(residual.y == -0.25f);
    CHECK(residual.z == 0.f);
    CHECK(residual.w == 0.f);
    CHECK(residual.y != 0.f);

    Scene scene = make_scene();
    const diag3576::Sample sample = diag3576::sample_position(scene, scene.base + kSlotRva);
    CHECK(sample.status == diag3576::SampleStatus::Coherent);
    CHECK(sample.attempts == 1);
    CHECK(sample.residual && sample.residual->y == -0.25f);
    CHECK(sample.world && *sample.world == world);
    CHECK(sample.pointers.manager == kManager);
    CHECK(sample.pointers.local_field == kPhysics + diag3576::kHopLocal);

    scene.poke_vec(scene.base + diag3576::kStaticRva, diag3576::Vec4{100000.f, -50.f, 8.f, 0.f});
    const diag3576::Sample wide = diag3576::sample_position(scene, scene.base + kSlotRva);
    CHECK(wide.status == diag3576::SampleStatus::Coherent);
    CHECK(wide.residual && wide.residual->x == 100000.f - world.x);
    const std::string line = diag3576::format_sample(wide, "2026-11-12 11:18:52.528");
    CHECK(line.find("status=coherent") != std::string::npos);
    CHECK(line.find("player-ready=unresolved") != std::string::npos);
    CHECK(line.find("loading=unresolved") != std::string::npos);
    CHECK(line.find("residual=") != std::string::npos);
    const std::string banner = diag3576::format_banner(kSlotRva);
    CHECK(banner.find("experimental") != std::string::npos);
    CHECK(banner.find("not a production offset") != std::string::npos);
    CHECK(banner.find("guard stays closed") != std::string::npos);
}

void test_origin_change() {
    Scene scene = make_scene();
    const diag3576::Vec4 next_origin{-10000.f, 0.f, -4000.f, 0.f};
    const diag3576::Vec4 next_world = diag3576::add(local_vec(), next_origin);
    diag3576::Vec4 next_static = next_world;
    next_static.y -= 0.25f;

    int origin_reads = 0;
    scene.before = [&](uint64_t addr, size_t) {
        if (addr != scene.base + diag3576::kOriginRva) return;
        ++origin_reads;
        if (origin_reads == 2) {
            scene.poke_vec(scene.base + diag3576::kOriginRva, next_origin);
            scene.poke_vec(scene.base + diag3576::kStaticRva, next_static);
        }
    };
    const diag3576::Sample changed = diag3576::sample_position(scene, scene.base + kSlotRva);
    CHECK(changed.status == diag3576::SampleStatus::Coherent);
    CHECK(changed.attempts == 2);
    CHECK(changed.recovered_from == diag3576::SampleStatus::OriginChanged);
    CHECK(changed.origin && *changed.origin == next_origin);
    CHECK(changed.world && *changed.world == next_world);
    CHECK(changed.residual && changed.residual->y == -0.25f);

    scene.before = nullptr;
    scene.poke_vec(scene.base + diag3576::kOriginRva, origin_vec());
    scene.poke_vec(scene.base + diag3576::kStaticRva, diag3576::add(local_vec(), origin_vec()));
    int flips = 0;
    scene.before = [&](uint64_t addr, size_t) {
        if (addr != scene.base + diag3576::kOriginRva) return;
        ++flips;
        if (flips % 2 == 0) scene.poke_vec(addr, next_origin);
        else scene.poke_vec(addr, origin_vec());
    };
    const diag3576::Sample stuck = diag3576::sample_position(scene, scene.base + kSlotRva);
    CHECK(stuck.status == diag3576::SampleStatus::OriginChanged);
    CHECK(stuck.attempts == diag3576::kMaxSampleAttempts);
    CHECK(stuck.origin && stuck.origin_after);
    CHECK(!(*stuck.origin == *stuck.origin_after));
    CHECK(stuck.residual && stuck.residual->y == stuck.static_pos->y - stuck.world->y);
}

void test_pointer_replacement() {
    Scene scene = make_scene();
    plant_chain(scene, kManagerB);
    plant_chain(scene, kManager);
    int slot_reads = 0;
    scene.before = [&](uint64_t addr, size_t n) {
        if (addr != scene.base + kSlotRva || n != sizeof(uint64_t)) return;
        ++slot_reads;
        scene.poke_u64(addr, slot_reads % 2 == 0 ? kManagerB : kManager);
    };
    const diag3576::Sample sample = diag3576::sample_position(scene, scene.base + kSlotRva);
    CHECK(sample.status == diag3576::SampleStatus::PointersChanged);
    CHECK(sample.attempts == diag3576::kMaxSampleAttempts);
    CHECK(sample.pointers.manager == kManager);
    CHECK(sample.local.has_value());
}

void test_invalid_reads() {
    Scene scene = make_scene();
    scene.before = [&](uint64_t addr, size_t) {
        if (addr == scene.base + diag3576::kStaticRva) scene.bytes.erase(addr);
    };
    const diag3576::Sample unreadable = diag3576::sample_position(scene, scene.base + kSlotRva);
    CHECK(unreadable.status == diag3576::SampleStatus::Unreadable);
    CHECK(unreadable.attempts == diag3576::kMaxSampleAttempts);

    Scene nan_scene = make_scene();
    diag3576::Vec4 broken = local_vec();
    broken.y = std::nanf("");
    nan_scene.poke_vec(kPhysics + diag3576::kHopLocal, broken);
    const diag3576::Sample non_finite = diag3576::sample_position(nan_scene, nan_scene.base + kSlotRva);
    CHECK(non_finite.status == diag3576::SampleStatus::NonFinite);
    CHECK(non_finite.attempts == diag3576::kMaxSampleAttempts);
    CHECK(non_finite.residual && !diag3576::finite(*non_finite.residual));

    Scene wrong = make_scene();
    plant_identity(wrong, diag3576::kProductionTimestamp);
    const diag3576::IdentityCheck rejected = diag3576::check_identity(wrong);
    CHECK(!rejected.accepted);
    CHECK(rejected.timestamp == diag3576::kProductionTimestamp);

    Scene shifted = make_scene();
    shifted.base = 0x150000000;
    plant_identity(shifted, diag3576::kDiagnosticTimestamp);
    CHECK(!diag3576::check_identity(shifted).accepted);
    CHECK(diag3576::check_identity(make_scene()).accepted);
    CHECK(diag3576::check_slot(make_scene(), kSlotRva).empty());
    CHECK(!diag3576::check_slot(make_scene(), 0x71C09A9).empty());
}

void test_process_exit() {
    Scene scene = make_scene();
    scene.alive_flag = false;
    const diag3576::Sample gone = diag3576::sample_position(scene, scene.base + kSlotRva);
    CHECK(gone.status == diag3576::SampleStatus::ProcessExited);
    CHECK(gone.attempts == 1);
    CHECK(!diag3576::check_identity(scene).accepted);

    Scene mid = make_scene();
    mid.before = [&](uint64_t addr, size_t) {
        if (addr == kPhysics + diag3576::kHopLocal) mid.alive_flag = false;
    };
    const diag3576::Sample during = diag3576::sample_position(mid, mid.base + kSlotRva);
    CHECK(during.status == diag3576::SampleStatus::ProcessExited);
    CHECK(during.attempts == 1);
    const std::string line = diag3576::format_sample(during, "2026-11-12 11:18:52.528");
    CHECK(line.find("status=process-exited") != std::string::npos);
    CHECK(line.find("player-ready=unresolved") != std::string::npos);
}

}  // namespace

int main() {
    test_addition_leaves_y();
    test_origin_change();
    test_pointer_replacement();
    test_invalid_reads();
    test_process_exit();
    if (g_failures) {
        std::printf("%d failure(s)\n", g_failures);
        return 1;
    }
    std::puts("diag3576 tests ok");
    return 0;
}
