// Game-free checks for the 3576 live-capture identity gate.
// The production diagnostic DLL is loaded here and must refuse this process
// before Capture::start. This executable is not Crimson Desert.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include <windows.h>

#include "pkt3576/candidates.h"
#include "pkt3576/live_api.h"
#include "pkt3576/live_identity.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

struct Image {
    uint8_t header[0x200]{};
    uint8_t send[16]{};
    uint8_t recv[16]{};
    bool fail_header = false;
    bool fail_send = false;
    bool fail_recv = false;
    int send_reads = 0;
    int recv_reads = 0;
    int outside_reads = 0;
    uintptr_t base = static_cast<uintptr_t>(pkt3576::kImageBase);
};

void write_u16(uint8_t* destination, uint16_t value) { std::memcpy(destination, &value, sizeof value); }
void write_u32(uint8_t* destination, uint32_t value) { std::memcpy(destination, &value, sizeof value); }

void build_header(Image& image, uint32_t timestamp, uint32_t size_of_image) {
    std::memset(image.header, 0, sizeof image.header);
    image.header[0] = 'M';
    image.header[1] = 'Z';
    write_u32(image.header + 0x3C, 0x80);
    write_u32(image.header + 0x80, 0x00004550);
    write_u32(image.header + 0x88, timestamp);
    write_u16(image.header + 0x98, 0x020B);
    write_u32(image.header + 0xD0, size_of_image);
    std::memcpy(image.send, pkt3576::kSendEntry, sizeof image.send);
    std::memcpy(image.recv, pkt3576::kRecvEntry, sizeof image.recv);
}

bool image_read(uintptr_t address, void* destination, size_t size, void* user) {
    auto* image = static_cast<Image*>(user);
    if (address >= image->base && address - image->base < sizeof image->header) {
        if (image->fail_header) return false;
        const size_t offset = static_cast<size_t>(address - image->base);
        if (size > sizeof image->header || offset + size > sizeof image->header) return false;
        std::memcpy(destination, image->header + offset, size);
        return true;
    }
    if (address == image->base + static_cast<uintptr_t>(pkt3576::kSendRva) && size == 16) {
        ++image->send_reads;
        if (image->fail_send) return false;
        std::memcpy(destination, image->send, sizeof image->send);
        return true;
    }
    if (address == image->base + static_cast<uintptr_t>(pkt3576::kRecvRva) && size == 16) {
        ++image->recv_reads;
        if (image->fail_recv) return false;
        std::memcpy(destination, image->recv, sizeof image->recv);
        return true;
    }
    ++image->outside_reads;
    return false;
}

pkt3576::Identity identify(Image& image, const wchar_t* name) {
    return pkt3576::identify_image(name, image.base, image_read, &image);
}

void test_match_and_rejections() {
    Image match;
    build_header(match, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    const pkt3576::Identity ok = identify(match, pkt3576::kExeName);
    CHECK(ok.kind == pkt3576::IdentityKind::Match);
    CHECK(ok.timestamp == pkt3576::kTimestamp3576);
    CHECK(ok.size_of_image == pkt3576::kImageSize);
    CHECK(ok.mapped_base == static_cast<uintptr_t>(pkt3576::kImageBase));
    CHECK(ok.send_signature_ok);
    CHECK(ok.recv_signature_ok);
    CHECK(!ok.rejected_2976);
    CHECK(match.send_reads == 1);
    CHECK(match.recv_reads == 1);
    CHECK(pkt3576::identity_allows_hooks(ok));

    Image folded;
    build_header(folded, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    CHECK(identify(folded, L"crimsonDesert.EXE").kind == pkt3576::IdentityKind::Match);

    Image unnamed;
    build_header(unnamed, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    const pkt3576::Identity no_name = identify(unnamed, nullptr);
    CHECK(no_name.kind == pkt3576::IdentityKind::WrongName);
    CHECK(no_name.timestamp_known);
    CHECK(unnamed.send_reads == 0);
    CHECK(unnamed.recv_reads == 0);
    CHECK(!pkt3576::identity_allows_hooks(no_name));

    Image other_name;
    build_header(other_name, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    CHECK(identify(other_name, L"Other.exe").kind == pkt3576::IdentityKind::WrongName);
    CHECK(other_name.send_reads == 0);

    Image old_build;
    build_header(old_build, pkt3576::kTimestamp2976, pkt3576::kImageSize);
    const pkt3576::Identity rejected = identify(old_build, pkt3576::kExeName);
    CHECK(rejected.kind == pkt3576::IdentityKind::WrongTimestamp);
    CHECK(rejected.rejected_2976);
    CHECK(rejected.timestamp == pkt3576::kTimestamp2976);
    CHECK(old_build.send_reads == 0);
    CHECK(old_build.recv_reads == 0);
    CHECK(old_build.outside_reads == 0);
    CHECK(!pkt3576::identity_allows_hooks(rejected));

    Image other_time;
    build_header(other_time, 0x11111111u, pkt3576::kImageSize);
    const pkt3576::Identity timed = identify(other_time, pkt3576::kExeName);
    CHECK(timed.kind == pkt3576::IdentityKind::WrongTimestamp);
    CHECK(!timed.rejected_2976);
    CHECK(other_time.send_reads == 0);

    Image moved;
    build_header(moved, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    moved.base = 0x180000000ull;
    const pkt3576::Identity wrong_base = identify(moved, pkt3576::kExeName);
    CHECK(wrong_base.kind == pkt3576::IdentityKind::WrongBase);
    CHECK(moved.send_reads == 0);
    CHECK(moved.recv_reads == 0);
    CHECK(moved.outside_reads == 0);

    Image small;
    build_header(small, pkt3576::kTimestamp3576, 0x1000u);
    const pkt3576::Identity wrong_size = identify(small, pkt3576::kExeName);
    CHECK(wrong_size.kind == pkt3576::IdentityKind::WrongSize);
    CHECK(wrong_size.size_of_image == 0x1000u);
    CHECK(small.send_reads == 0);
    CHECK(small.recv_reads == 0);

    Image bad_mz;
    build_header(bad_mz, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    bad_mz.header[0] = 0;
    const pkt3576::Identity mz = identify(bad_mz, pkt3576::kExeName);
    CHECK(mz.kind == pkt3576::IdentityKind::Unreadable);
    CHECK(!mz.timestamp_known);
    CHECK(bad_mz.send_reads == 0);

    Image early;
    build_header(early, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    write_u32(early.header + 0x3C, 0x30);
    CHECK(identify(early, pkt3576::kExeName).kind == pkt3576::IdentityKind::Unreadable);

    Image high_e_lfanew;
    build_header(high_e_lfanew, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    write_u32(high_e_lfanew.header + 0x3C, 0x800);
    CHECK(identify(high_e_lfanew, pkt3576::kExeName).kind == pkt3576::IdentityKind::Unreadable);

    Image unread;
    build_header(unread, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    unread.fail_header = true;
    const pkt3576::Identity failed = identify(unread, pkt3576::kExeName);
    CHECK(failed.kind == pkt3576::IdentityKind::Unreadable);
    CHECK(!failed.timestamp_known);
    CHECK(unread.send_reads == 0);

    Image bad_pe;
    build_header(bad_pe, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    write_u32(bad_pe.header + 0x80, 0);
    const pkt3576::Identity pe = identify(bad_pe, pkt3576::kExeName);
    CHECK(pe.kind == pkt3576::IdentityKind::Unreadable);
    CHECK(!pe.timestamp_known);

    Image pe32;
    build_header(pe32, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    write_u16(pe32.header + 0x98, 0x010B);
    const pkt3576::Identity magic = identify(pe32, pkt3576::kExeName);
    CHECK(magic.kind == pkt3576::IdentityKind::Unreadable);
    CHECK(magic.timestamp_known);
    CHECK(!magic.size_known);
    CHECK(pe32.send_reads == 0);

    Image mismatch;
    build_header(mismatch, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    mismatch.send[0] ^= 0xFF;
    const pkt3576::Identity signatures = identify(mismatch, pkt3576::kExeName);
    CHECK(signatures.kind == pkt3576::IdentityKind::SignatureMismatch);
    CHECK(!signatures.send_signature_ok);
    CHECK(signatures.recv_signature_ok);
    CHECK(mismatch.send_reads == 1);
    CHECK(mismatch.recv_reads == 1);
    CHECK(!pkt3576::identity_allows_hooks(signatures));

    Image dropped;
    build_header(dropped, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    dropped.fail_recv = true;
    const pkt3576::Identity unreadable_entry = identify(dropped, pkt3576::kExeName);
    CHECK(unreadable_entry.kind == pkt3576::IdentityKind::Unreadable);
    CHECK(dropped.send_reads == 1);
    CHECK(dropped.recv_reads == 1);
    CHECK(!unreadable_entry.recv_signature_ok);
}

void test_process_read() {
    uint8_t dos[2] = {};
    const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    CHECK(pkt3576::read_mapped_bytes(base, dos, sizeof dos));
    CHECK(dos[0] == 'M' && dos[1] == 'Z');

    void* page = VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_NOACCESS);
    CHECK(page != nullptr);
    uint8_t buffer[16] = {};
    CHECK(!pkt3576::read_mapped_bytes(reinterpret_cast<uintptr_t>(page), buffer, sizeof buffer));
    CHECK(VirtualFree(page, 0, MEM_RELEASE));

    const pkt3576::Identity self = pkt3576::identify_process();
    CHECK(self.kind == pkt3576::IdentityKind::WrongName);
    CHECK(!pkt3576::identity_allows_hooks(self));
    CHECK(std::strcmp(pkt3576::identity_kind_name(self.kind), "wrong-name") == 0);
}

void test_log_directory_rules() {
    const wchar_t* game = L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64\\CrimsonDesert.exe";
    const wchar_t* game_dir = L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64";
    CHECK(!pkt3576::log_directory_allowed(game, L"pkt3576-live-relative-must-not-exist"));
    CHECK(!pkt3576::log_directory_allowed(game, game_dir));
    CHECK(!pkt3576::log_directory_allowed(game, L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64\\logs"));
    CHECK(!pkt3576::log_directory_allowed(L"C:\\Windows\\notepad.exe",
                                         L"D:/SteamLibrary/steamapps/common/Crimson Desert/BIN64"));
    CHECK(!pkt3576::log_directory_allowed(L"C:\\Games\\Custom\\CrimsonDesert.exe", L"C:\\Games\\Custom"));
    CHECK(!pkt3576::log_directory_allowed(L"C:\\Games\\Custom\\CrimsonDesert.exe", L"C:\\Games\\Custom\\out"));
    CHECK(pkt3576::log_directory_allowed(L"C:\\Games\\Custom\\CrimsonDesert.exe", L"C:\\Games\\Other"));
    CHECK(pkt3576::log_directory_allowed(L"C:\\Games\\Custom\\CrimsonDesert.exe", L"C:\\Temp\\pkt3576-live"));

    wchar_t temp[MAX_PATH] = {};
    const DWORD length = GetTempPathW(MAX_PATH, temp);
    CHECK(length > 0 && length < MAX_PATH);
    CHECK(pkt3576::log_directory_allowed(game, temp));
}

std::wstring beside(const wchar_t* file_name) {
    wchar_t path[1024] = {};
    const DWORD length = GetModuleFileNameW(nullptr, path, 1024);
    if (length == 0 || length >= 1024) return {};
    wchar_t* slash = nullptr;
    for (wchar_t* cursor = path; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') slash = cursor;
    }
    if (slash == nullptr) return {};
    *(slash + 1) = L'\0';
    return std::wstring(path) + file_name;
}

std::wstring temp_dir() {
    wchar_t temp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    return std::wstring(temp) + L"pkt3576-live-id-" + std::to_wstring(GetCurrentProcessId());
}

std::string read_file(const std::wstring& path) {
    FILE* file = nullptr;
    if (_wfopen_s(&file, path.c_str(), L"rb") != 0 || file == nullptr) return {};
    std::string text;
    char buffer[256];
    size_t count = 0;
    while ((count = std::fread(buffer, 1, sizeof buffer, file)) > 0) text.append(buffer, count);
    std::fclose(file);
    return text;
}

bool exists_file(const std::wstring& path) { return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES; }

using StartFn = int (*)(const wchar_t*);
using StopFn = int (*)();
using StatusFn = int (*)(int*, int*, int*, unsigned*, unsigned*, unsigned*);

__declspec(noinline) int live_probe() { return 0x3576; }

void test_production_dll_refuses_before_hooks() {
    uint8_t before[16] = {};
    std::memcpy(before, reinterpret_cast<void*>(&live_probe), sizeof before);

    const std::wstring path = beside(L"cdmp_pkt3576_live.dll");
    CHECK(!path.empty());
    HMODULE module = LoadLibraryW(path.c_str());
    if (module == nullptr) {
        std::printf("FAIL LoadLibrary %ls error %lu\n", path.c_str(), GetLastError());
        ++g_failures;
        return;
    }
    auto start = reinterpret_cast<StartFn>(GetProcAddress(module, "pkt3576_live_start"));
    auto stop = reinterpret_cast<StopFn>(GetProcAddress(module, "pkt3576_live_stop"));
    auto status = reinterpret_cast<StatusFn>(GetProcAddress(module, "pkt3576_live_status"));
    CHECK(start != nullptr);
    CHECK(stop != nullptr);
    CHECK(status != nullptr);
    CHECK(GetProcAddress(module, "pkt3576_live_mark") != nullptr);
    CHECK(GetProcAddress(module, "pkt3576_live_fixture_start") == nullptr);
    CHECK(GetProcAddress(module, "pkt3576_live_fixture_arm") == nullptr);
    CHECK(GetProcAddress(module, "pkt3576_live_fixture_set_records_open_hook") == nullptr);
    CHECK(GetProcAddress(module, "pkt3576_live_fixture_set_phase_hold") == nullptr);
    auto report_fn =
        reinterpret_cast<int (*)(Pkt3576LiveReport*)>(GetProcAddress(module, "pkt3576_live_report"));
    CHECK(report_fn != nullptr);

    const std::wstring relative = L"pkt3576-live-relative-must-not-exist";
    CHECK(start(relative.c_str()) == kPkt3576LiveBadLog);
    CHECK(!exists_file(relative));
    const wchar_t* game_dir = L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64";
    const std::wstring game_log = std::wstring(game_dir) + L"\\pkt3576-live-session.txt";
    const bool game_log_existed = exists_file(game_log);
    CHECK(start(game_dir) == kPkt3576LiveBadLog);
    if (!game_log_existed) CHECK(!exists_file(game_log));

    int start_code = 0;
    int stop_code = 0;
    int retained = 1;
    unsigned stored = 1;
    unsigned omitted = 1;
    unsigned attempts = 1;
    CHECK(status(&start_code, &stop_code, &retained, &stored, &omitted, &attempts) == 0);
    CHECK(attempts == 0);
    if (report_fn != nullptr) {
        Pkt3576LiveReport report{};
        CHECK(report_fn(nullptr) == 1);
        CHECK(report_fn(&report) == 0);
        CHECK(report.capture_attempts == 0);
        CHECK(report.retained == 0);
    }

    const std::wstring directory = temp_dir();
    const std::wstring session = directory + L"\\pkt3576-live-session.txt";
    const std::wstring records = directory + L"\\pkt3576-live-records.txt";
    CHECK(start(directory.c_str()) == kPkt3576LiveRefused);
    CHECK(status(&start_code, &stop_code, &retained, &stored, &omitted, &attempts) == 0);
    CHECK(start_code == kPkt3576LiveRefused);
    CHECK(attempts == 0);
    CHECK(retained == 0);
    CHECK(exists_file(session));
    CHECK(!exists_file(records));
    const std::string log = read_file(session);
    if (log.find("startup=refused") == std::string::npos || log.find("kind=wrong-name") == std::string::npos ||
        log.find("capture_attempts=0") == std::string::npos || log.find("fixture=0") == std::string::npos ||
        log.find("game_match=0") == std::string::npos || log.find("game_match=1") != std::string::npos ||
        log.find("qpc_timebase=QueryPerformanceCounter") == std::string::npos ||
        log.find("actor_load_max=unknown") == std::string::npos || log.find("copy_cap=4096") == std::string::npos) {
        std::printf("FAIL session log:\n%s\n", log.c_str());
        ++g_failures;
    }
    if (report_fn != nullptr) {
        Pkt3576LiveReport after{};
        CHECK(report_fn(&after) == 0);
        CHECK(after.qpc_frequency != 0);
        CHECK(after.capture_attempts == 0);
    }
    CHECK(stop() == kPkt3576LiveIdle);
    CHECK(live_probe() == 0x3576);
    uint8_t after[16] = {};
    std::memcpy(after, reinterpret_cast<void*>(&live_probe), sizeof after);
    CHECK(std::memcmp(before, after, sizeof before) == 0);

    CHECK(FreeLibrary(module));
    CHECK(GetModuleHandleW(L"cdmp_pkt3576_live.dll") == nullptr);
}

}  // namespace

int main() {
    test_match_and_rejections();
    test_process_read();
    test_log_directory_rules();
    test_production_dll_refuses_before_hooks();
    if (g_failures != 0) {
        std::printf("pkt3576 live tests failed: %d\n", g_failures);
        return 1;
    }
    std::printf("pkt3576 live tests ok\n");
    return 0;
}
