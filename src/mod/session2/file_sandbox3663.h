#pragma once
#include <cstdint>
#include <string>
namespace cdmp::session2::file_sandbox3663 {
struct Setup {
    std::wstring real, backup, sandbox, manifest;
    std::string manifest_sha256;
    bool source_may_change = false; // concurrent real-save host; backup/sandbox still match frozen manifest
    bool profile_redirect = false; // native CSIDL_LOCAL_APPDATA resolves sandbox; installs zero file hooks
};
// Process lifetime hooks. Intended for the game or a dedicated fixture child,
// never cdmp_host (which unloads the ASI).
bool install(const Setup& setup);
bool install_from_environment();
bool verified();
bool file_hooks_active();
std::string describe();
// Periodic worker report. No-op when this process has no NT save hooks.
void report_refusals();
std::string failure_reason();
#ifdef CDMP_SANDBOX_FIXTURE
void fixture_abort();
#endif
}
