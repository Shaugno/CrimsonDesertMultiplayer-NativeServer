// cdmp_host.exe - loads cdmp.asi into a plain process (no game) so the
// mod's startup, config, logging and networking can be tested anywhere.
//
//   cdmp_host <path\to\cdmp.asi> [seconds=6]
//
// The mod writes cdmp.log / cdmp.ini next to the .asi, exactly as in game.

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cwchar>

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) {
        std::printf("Usage: cdmp_host <path\\to\\cdmp.asi> [seconds]\n");
        return 2;
    }
    const int seconds = argc >= 3 ? _wtoi(argv[2]) : 6;

    // The mod refuses to start in any process but the game unless this is set.
    SetEnvironmentVariableW(L"CDMP_TEST_HOST", L"1");

    HMODULE mod = LoadLibraryW(argv[1]);
    if (!mod) {
        std::printf("LoadLibrary failed (error %lu)\n", GetLastError());
        return 1;
    }
    std::printf("Loaded mod, running for %d s...\n", seconds);
    bool tier_ok=true;
    if(argc>=4 && _wcsicmp(argv[3],L"--require-3663-v1")==0){
        using MaskFn=uint32_t(*)();
        const auto mask_fn=reinterpret_cast<MaskFn>(GetProcAddress(mod,"cdmp_3663_tier_mask"));
        const uint32_t mask=mask_fn?mask_fn():0;
        std::printf("3663 compiled tier mask 0x%X (metadata only)\n",mask);
        tier_ok=mask==15;
    }
    Sleep(static_cast<DWORD>(seconds) * 1000);

    using ShutdownFn = void (*)();
    auto shutdown = reinterpret_cast<ShutdownFn>(GetProcAddress(mod, "cdmp_shutdown"));
    if (!shutdown) {
        std::printf("cdmp_shutdown export missing\n");
        return 1;
    }
    shutdown();
    FreeLibrary(mod);
    std::printf("Mod stopped and unloaded cleanly\n");
    return tier_ok?0:3;
}
