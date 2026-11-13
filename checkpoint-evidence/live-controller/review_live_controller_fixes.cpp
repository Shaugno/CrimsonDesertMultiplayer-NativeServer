// Independent game-free review reproduction. No process is opened.
// The fake image uses real SafetyHook patches on private executable pages.
#include "pkt3576/live_control.cpp"
#include <safetyhook.hpp>

namespace {
struct Image {
    uint8_t header[0x200]{};
    uint8_t* send = nullptr;
    uint8_t* recv = nullptr;
};
void u16(uint8_t* p, uint16_t v) { std::memcpy(p, &v, sizeof v); }
void u32(uint8_t* p, uint32_t v) { std::memcpy(p, &v, sizeof v); }
bool read_image(uintptr_t address, void* output, size_t size, void* user) {
    auto& image = *static_cast<Image*>(user);
    const auto base = static_cast<uintptr_t>(pkt3576::kImageBase);
    if (address >= base && address - base < sizeof image.header &&
        size <= sizeof image.header - (address - base)) {
        std::memcpy(output, image.header + (address - base), size);
        return true;
    }
    if (address == base + pkt3576::kSendRva && size == 16) {
        std::memcpy(output, image.send, size);
        return true;
    }
    if (address == base + pkt3576::kRecvRva && size == 16) {
        std::memcpy(output, image.recv, size);
        return true;
    }
    return false;
}
void callback(SafetyHookContext&) {}
}

int main() {
    Image image;
    image.send = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    image.recv = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    if (!image.send || !image.recv) return 10;
    std::memset(image.send, 0x90, 4096);
    std::memset(image.recv, 0x90, 4096);
    std::memcpy(image.send, pkt3576::kSendEntry, 16);
    std::memcpy(image.recv, pkt3576::kRecvEntry, 16);
    u16(image.header, 0x5A4D);
    u32(image.header + 0x3C, 0x80);
    u32(image.header + 0x80, 0x4550);
    u32(image.header + 0x88, pkt3576::kTimestamp3576);
    u16(image.header + 0x98, 0x20B);
    u32(image.header + 0xD0, pkt3576::kImageSize);
    const auto before = pkt3576::identify_image(pkt3576::kExeName, pkt3576::kImageBase, read_image, &image);
    auto send_hook = safetyhook::create_mid(image.send, callback);
    auto recv_hook = safetyhook::create_mid(image.recv, callback);
    if (!send_hook || !recv_hook) return 11;
    const auto active = pkt3576::identify_image(pkt3576::kExeName, pkt3576::kImageBase, read_image, &image);
    using pkt3576::control::ControlAction;
    const bool can_status = pkt3576::control::control_allowed(false, false, active.kind, true, ControlAction::Status, true);
    const bool can_stop = pkt3576::control::control_allowed(false, false, active.kind, true, ControlAction::Stop, true);
    const bool can_arm = pkt3576::control::control_allowed(false, false, active.kind, true, ControlAction::Start, true);
    const bool can_load = pkt3576::control::control_allowed(false, false, active.kind, true, ControlAction::Load, true);
    const bool absent_stop = pkt3576::control::control_allowed(false, false, active.kind, true, ControlAction::Stop, false);
    u32(image.header + 0x88, pkt3576::kTimestamp2976);
    const auto old_build = pkt3576::identify_image(pkt3576::kExeName, pkt3576::kImageBase, read_image, &image);
    const bool old_stop = pkt3576::control::control_allowed(false, false, old_build.kind, true, ControlAction::Stop, true);
    u32(image.header + 0x88, pkt3576::kTimestamp3576);
    send_hook.reset();
    recv_hook.reset();
    const auto after = pkt3576::identify_image(pkt3576::kExeName, pkt3576::kImageBase, read_image, &image);
    std::printf("before=%s active=%s production_status_allowed=%d production_stop_allowed=%d after=%s\n",
                pkt3576::identity_kind_name(before.kind), pkt3576::identity_kind_name(active.kind),
                can_status, can_stop, pkt3576::identity_kind_name(after.kind));
    std::printf("patched_load_allowed=%d patched_start_allowed=%d absent_module_stop_allowed=%d old_build_stop_allowed=%d\n",
                can_load, can_arm, absent_stop, old_stop);
    VirtualFree(image.send, 0, MEM_RELEASE);
    VirtualFree(image.recv, 0, MEM_RELEASE);
    DWORD parsed = 0;
    const bool overflow = pkt3576::control::parse_pid(L"4294967297", parsed);
    std::printf("overflow_pid_accepted=%d resulting_pid=%lu\n", overflow, parsed);
    bool bounds_ok = true;
    const wchar_t* bad[] = {L"0", L"4294967296", L"4294967297", L"4294967298", L"4294967299",
                            L"4294967300", L"4294967301", L"4294967302", L"4294967303", L"4294967304",
                            L"9999999999999999999999999999999999999999999"};
    for (const auto* text : bad) {
        DWORD result = 23;
        if (pkt3576::control::parse_pid(text, result) || result != 23) bounds_ok = false;
    }
    DWORD maximum = 0;
    bounds_ok = bounds_ok && pkt3576::control::parse_pid(L"4294967295", maximum) && maximum == 0xFFFFFFFFUL;
    const bool fixed = before.kind == pkt3576::IdentityKind::Match &&
        active.kind == pkt3576::IdentityKind::SignatureMismatch && can_status && can_stop &&
        !can_load && !can_arm && !absent_stop && !old_stop &&
        after.kind == pkt3576::IdentityKind::Match && !overflow && parsed == 0 && bounds_ok;
    std::printf("review_regressions=%s\n", fixed ? "passed" : "failed");
    return fixed ? 0 : 12;
}
