#include "mod/session2/guest_crash3663.h"
#include <windows.h>

namespace capture = cdmp::session2::guest_crash3663;
namespace {
HANDLE chained{};

LONG WINAPI previous(EXCEPTION_POINTERS*) {
    const char marker[] = "previous filter called\n";
    DWORD written{};
    WriteFile(chained, marker, sizeof marker - 1, &written, nullptr);
    return EXCEPTION_CONTINUE_SEARCH;
}

__declspec(noinline) void write_inaccessible(volatile LONG* value) {
    *value = 42;
}

bool handled_write(volatile LONG* value) {
    __try {
        write_inaccessible(value);
        return false;
    } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
               EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        return true;
    }
}
}

int wmain(int argc, wchar_t** argv) {
    if (argc != 2) return 1;
    SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
    chained = CreateFileW(L"prior-filter.txt", GENERIC_WRITE, FILE_SHARE_READ,
                          nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (chained == INVALID_HANDLE_VALUE) return 2;
    SetUnhandledExceptionFilter(previous);
    if (!capture::install(argv[1])) return 3;
    auto* page = static_cast<volatile LONG*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS));
    if (!page || !handled_write(page)) return 4;
    if (capture::packet(capture::Direction::ToClient, 1, 5) != 0) return 5;
    capture::arm();
    capture::returned(capture::packet(capture::Direction::ToHost, 3081, 22));
    capture::returned(capture::packet(capture::Direction::ToClient, 2364, 65));
    capture::note_null_playback(0xA0100001,0,0xA0100001,0xA01001BE,1,"host");
    capture::note_playback_skip(0xA0100001);
    if (!handled_write(page)) return 6;
    (void)capture::packet(capture::Direction::ToClient, 2005, 100);
    write_inaccessible(page);
    return 7;
}
