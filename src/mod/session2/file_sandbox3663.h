#pragma once
#include <cstdint>
#include <string>
namespace cdmp::session2::file_sandbox3663 {
struct Setup {
    std::wstring real, backup, sandbox, manifest;
    std::string manifest_sha256;
};
// Process lifetime hooks. Intended for the game or a dedicated fixture child,
// never cdmp_host (which unloads the ASI).
bool install(const Setup& setup);
bool install_from_environment();
bool verified();
std::string describe();
std::string failure_reason();
#ifdef CDMP_SANDBOX_FIXTURE
void fixture_abort();
#endif
}
