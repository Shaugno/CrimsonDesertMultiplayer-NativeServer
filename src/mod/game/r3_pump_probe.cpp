// R3 passive candidate probe. No packets, native calls or context modifications.
// Candidate midpoint: R2 native NetworkThread disassembly, B065/B066/B068.
#include "mod/game/r3_pump_probe.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include <array>
#include <atomic>
#include <cstring>
#include <utility>

namespace cdmp::game::r3_pump_probe {
namespace {
constexpr uintptr_t kCandidatePoint = 0x3A8D94;
constexpr uintptr_t kClientNetSlot = build_3663::core_rvas::B002_kClientNetSlot;
uintptr_t base = 0;
safetyhook::MidHook hook;
std::atomic<uint64_t> seen{}, matched{}, ready{}, changed{};
void observe(safetyhook::Context& context) {
    seen.fetch_add(1, std::memory_order_relaxed);
    const auto net = fast_ptr(base + kClientNetSlot);
    if (!net || context.r15 != *net) return;
    matched.fetch_add(1, std::memory_order_relaxed);
    const auto session = fast_ptr(*net + build_3663::tunnel_layout::kClientNet_Session);
    const auto incoming = fast_ptr(*net + build_3663::tunnel_layout::kClientNet_InQueue);
    const auto returned = fast_ptr(*net + build_3663::tunnel_layout::kClientNet_FreeC2S);
    if (fast_ptr(base + kClientNetSlot) != net) {
        changed.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    if (session && incoming && returned) ready.fetch_add(1, std::memory_order_relaxed);
}
}
bool install(const build_guard::ImageIdentity& id) {
    if (!allowed(id,true) || hook) return false;
    base = id.mapped_base;
    constexpr auto& expected = build_3663::tunnel_layout::kNetLoopPumpAob;
    std::array<uint8_t,sizeof(expected)> actual{};
    if (!safe_read(base+kCandidatePoint,actual.data(),actual.size()) ||
        std::memcmp(actual.data(),expected,actual.size()) != 0) return false;
    auto candidate = prepare_mid(base+kCandidatePoint,observe,
        [&](const std::vector<uint8_t>& original) {
            return original.size()==build_3663::tunnel_layout::kNetLoopPumpPatch &&
                std::memcmp(original.data(),expected,original.size())==0;
        },"R3PumpProbe","read-only candidate midpoint");
    if (!candidate) return false;
    hook = std::move(*candidate);
    if (!hook.enable()) { hook = {}; return false; }
    log::info("R3PumpProbe: active at candidate exe+0x{:X}; guarded reads and atomic counters only; B remains off",kCandidatePoint);
    return true;
}
void report() {
    if (hook) log::info("R3PumpProbe: seen {} root_match {} fields_ready {} changed {}",
        seen.load(std::memory_order_relaxed),matched.load(std::memory_order_relaxed),
        ready.load(std::memory_order_relaxed),changed.load(std::memory_order_relaxed));
}
}
