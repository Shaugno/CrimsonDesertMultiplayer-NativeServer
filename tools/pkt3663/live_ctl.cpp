// External controller for cdmp_pkt3663_live.dll.
// Requires an explicit PID. Does not search for or launch Crimson Desert.

#include "pkt3663/live_control.h"

#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

#ifdef PKT3663_LIVE_CTL_FIXTURE
constexpr bool kFixtureBuild = true;
constexpr const char *kFixtureArmExport = "pkt3663_live_fixture_arm";
#else
constexpr bool kFixtureBuild = false;
constexpr const char *kFixtureArmExport = nullptr;
#endif

int main() {
  int argc = 0;
  wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  const int code =
      pkt3663::control::run_command(argc, argv, kFixtureBuild, kFixtureArmExport);
  if (argv != nullptr)
    LocalFree(argv);
  return code;
}
