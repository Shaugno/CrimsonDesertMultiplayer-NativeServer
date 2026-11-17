// Dedicated child process: hooks live until ExitProcess. The unhooked parent
// compares SHA256 manifests of the original tree and reads sandbox effects.
#include "mod/session2/file_sandbox3663.h"
#include <windows.h>
#include <winternl.h>
#include <shlobj.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
namespace {
int steam_probe(const std::wstring& dll){
    const auto slash=dll.find_last_of(L"\\/");
    if(slash==std::wstring::npos || _wcsicmp(dll.substr(slash+1).c_str(),L"steam_api64.dll"))return 42;
    SetEnvironmentVariableW(L"SteamAppId",L"3321460");SetEnvironmentVariableW(L"SteamGameId",L"3321460");
    // No RestartAppIfNecessary, game launch, game native calls or Steam messages.
    HMODULE module=LoadLibraryW(dll.c_str());
    if(!module){std::printf("Steam DLL load failed Win32 %lu\n",GetLastError());return 43;}
    const auto init=reinterpret_cast<int(__cdecl*)(char*)>(GetProcAddress(module,"SteamAPI_InitFlat"));
    const auto shutdown=reinterpret_cast<void(__cdecl*)()>(GetProcAddress(module,"SteamAPI_Shutdown"));
    if(!init || !shutdown){std::printf("Steam flat-init exports unavailable\n");return 44;}
    std::array<char,1024> error{};int result=init(error.data());
    std::printf("SteamAPI_InitFlat result %d error '%s' Win32 %lu\n",result,error.data(),GetLastError());
    std::printf("File layer: %s\n",cdmp::session2::file_sandbox3663::describe().c_str());
    if(!result)shutdown();
    std::fflush(stdout);return result?45:0;
}
bool write(const std::wstring& path,const char* text) {
    HANDLE f=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,CREATE_ALWAYS,0,nullptr);
    if(f==INVALID_HANDLE_VALUE)return false;
    DWORD n=0;bool ok=WriteFile(f,text,static_cast<DWORD>(strlen(text)),&n,nullptr) && n==strlen(text);
    CloseHandle(f);return ok;
}
std::string read(const std::wstring& path) {
    HANDLE f=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    if(f==INVALID_HANDLE_VALUE)return {};
    char b[128]{};DWORD n=0;bool ok=ReadFile(f,b,sizeof(b),&n,nullptr)!=FALSE;CloseHandle(f);return ok?std::string(b,n):std::string();
}
DWORD WINAPI small_stack_mapping(void* value){
    const auto* path=static_cast<const wchar_t*>(value);
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
    if(file==INVALID_HANDLE_VALUE)return 53;
    HANDLE section=CreateFileMappingW(file,nullptr,PAGE_READONLY,0,0,nullptr);
    if(!section){CloseHandle(file);return 54;}
    auto view=static_cast<const char*>(MapViewOfFile(section,FILE_MAP_READ,0,0,0));
    DWORD result=view && view[0]=='o'?0:55;
    if(view)UnmapViewOfFile(view);CloseHandle(section);CloseHandle(file);return result;
}
int profile_fixture(const std::wstring& root,const std::wstring& backup,const std::wstring& sandbox,
    const std::wstring& manifest,const std::string& hash,const wchar_t* steam_dll){
    using namespace cdmp::session2::file_sandbox3663;
    if(!install({root,backup,sandbox,manifest,hash,false,true}) || !verified() || file_hooks_active())return 60;
    wchar_t local[MAX_PATH]{};
    if(SHGetFolderPathW(nullptr,CSIDL_LOCAL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,local)!=S_OK)return 61;
    const auto resolved=std::wstring(local)+L"\\Pearl Abyss\\CD\\save";
    if(read(resolved+L"\\source.save")!="original")return 62;
    if(!write(resolved+L"\\profile-created.save","isolated"))return 63;
    if(!MoveFileExW((resolved+L"\\profile-created.save").c_str(),(resolved+L"\\profile-renamed.save").c_str(),0))return 64;
    if(!write(resolved+L"\\profile-delete.save","delete") || !DeleteFileW((resolved+L"\\profile-delete.save").c_str()))return 65;
    const auto path=resolved+L"\\source.save";
    HANDLE thread=CreateThread(nullptr,128u*1024,&small_stack_mapping,const_cast<wchar_t*>(path.c_str()),STACK_SIZE_PARAM_IS_A_RESERVATION,nullptr);
    if(!thread || WaitForSingleObject(thread,10000)!=WAIT_OBJECT_0)return 66;
    DWORD result=~0u;if(!GetExitCodeThread(thread,&result))return 67;CloseHandle(thread);if(result)return 68;
    HANDLE page=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,4096,nullptr);
    if(!page)return 69;void* view=MapViewOfFile(page,FILE_MAP_WRITE,0,0,4096);
    if(!view)return 70;std::memset(view,0x51,4096);UnmapViewOfFile(view);CloseHandle(page);
    HANDLE pipe=CreateFileW(L"\\\\.\\pipe\\",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);
    if(pipe==INVALID_HANDLE_VALUE || GetFileType(pipe)!=FILE_TYPE_PIPE)return 71;CloseHandle(pipe);
    if(file_hooks_active())return 72;
    std::printf("R5 profile fixture PASS: native save-root create/write/rename/delete, IPC, 128 KB thread; %s\n",describe().c_str());
    return steam_dll?steam_probe(steam_dll):0;
}
int fixture(int argc,wchar_t** argv) {
    if(argc!=7 && argc!=8)return 1;
    const std::wstring root=argv[1],backup=argv[2],sandbox=argv[3],manifest=argv[4],hash=argv[5],mode=argv[6];
    if(mode==L"steam-baseline")return argc==8?steam_probe(argv[7]):1;
    if(mode==L"profile" || mode==L"steam-profile"){
        std::string digest;for(wchar_t c:hash)digest+=static_cast<char>(c);
        return profile_fixture(root,backup,sandbox,manifest,digest,mode==L"steam-profile" && argc==8?argv[7]:nullptr);
    }
    HANDLE before=CreateFileW((root+L"\\source.save").c_str(),GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    if(before==INVALID_HANDLE_VALUE)return 2;
    HANDLE old_mapping=CreateFileMappingW(before,nullptr,PAGE_READWRITE,0,0,nullptr);
    if(!old_mapping)return 3;
    // Steam-style IPC: OpenFileMapping grants MAP_READ/WRITE, not
    // SECTION_QUERY. Both objects are owned by this fixture; no Steam needed.
    const std::wstring ipc_name=L"Local\\CDMP-R4-IPC-"+std::to_wstring(GetCurrentProcessId());
    HANDLE ipc=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,4096,ipc_name.c_str());
    if(!ipc)return 35;
    auto ipc_before=static_cast<char*>(MapViewOfFile(ipc,FILE_MAP_WRITE,0,0,4096));
    if(!ipc_before)return 36;
    ipc_before[0]='I';UnmapViewOfFile(ipc_before);
    const std::wstring pipe_name=L"\\\\.\\pipe\\CDMP-R4-PIPE-"+std::to_wstring(GetCurrentProcessId());
    HANDLE pipe=CreateNamedPipeW(pipe_name.c_str(),PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,1,64,64,0,nullptr);
    OVERLAPPED connect{};connect.hEvent=CreateEventW(nullptr,TRUE,FALSE,nullptr);
    if(pipe==INVALID_HANDLE_VALUE || !connect.hEvent)return 46;
    if(!ConnectNamedPipe(pipe,&connect) && GetLastError()!=ERROR_IO_PENDING)return 47;
    if(mode==L"premapped" && !MapViewOfFile(old_mapping,FILE_MAP_WRITE,0,0,0))return 30;
    std::string ascii;for(wchar_t c:hash)ascii+=static_cast<char>(c);
    bool installed=cdmp::session2::file_sandbox3663::install({root,backup,sandbox,manifest,ascii,mode==L"host-drift"});
    if(mode==L"refuse" || mode==L"premapped")return installed?4:0;
    if(!installed || !cdmp::session2::file_sandbox3663::verified()){
        std::printf("Install failed: %s\n",cdmp::session2::file_sandbox3663::failure_reason().c_str());return 5;}
    HANDLE pipe_client=CreateFileW(pipe_name.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
    if(pipe_client==INVALID_HANDLE_VALUE){std::printf("Named pipe open failed Win32 %lu\n",GetLastError());return 48;}
    if(WaitForSingleObject(connect.hEvent,2000)!=WAIT_OBJECT_0)return 49;
    using NativeOpen=NTSTATUS(NTAPI*)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES,PIO_STATUS_BLOCK,ULONG,ULONG);
    auto native_open=reinterpret_cast<NativeOpen>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtOpenFile"));
    UNICODE_STRING empty_name{};OBJECT_ATTRIBUTES pipe_attributes{};pipe_attributes.Length=sizeof(pipe_attributes);
    pipe_attributes.RootDirectory=pipe_client;pipe_attributes.ObjectName=&empty_name;
    IO_STATUS_BLOCK pipe_io{};HANDLE pipe_reopen=nullptr;
    if(!native_open)return 50;
    const auto reopen_status=native_open(&pipe_reopen,GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE,&pipe_attributes,&pipe_io,
        FILE_SHARE_READ|FILE_SHARE_WRITE,0x20);
    // A second connection to our occupied one-instance endpoint may be
    // refused by the kernel. Preserve PIPE_NOT_AVAILABLE, not ACCESS_DENIED.
    if(reopen_status!=static_cast<NTSTATUS>(0xC00000ACUL) &&
        (reopen_status<0 || GetFileType(pipe_reopen)!=FILE_TYPE_PIPE))return 50;
    if(pipe_reopen)CloseHandle(pipe_reopen);
    HANDLE pipe_root=CreateFileW(L"\\\\.\\pipe\\",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);
    if(pipe_root==INVALID_HANDLE_VALUE || GetFileType(pipe_root)!=FILE_TYPE_PIPE)return 51;
    pipe_attributes.RootDirectory=pipe_root;pipe_reopen=nullptr;
    const auto root_status=native_open(&pipe_reopen,GENERIC_READ|SYNCHRONIZE,&pipe_attributes,&pipe_io,FILE_SHARE_READ|FILE_SHARE_WRITE,0x20);
    if(root_status<0 || GetFileType(pipe_reopen)!=FILE_TYPE_PIPE){std::printf("Pipe root reopen NT status %08lX\n",static_cast<unsigned long>(root_status));return 52;}
    CloseHandle(pipe_reopen);CloseHandle(pipe_root);
    CloseHandle(pipe_client);DisconnectNamedPipe(pipe);CloseHandle(pipe);CloseHandle(connect.hEvent);
    if(mode==L"steam-sandbox")return argc==8?steam_probe(argv[7]):1;
    HANDLE ipc_open=OpenFileMappingW(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,ipc_name.c_str());
    if(!ipc_open)return 37;
    auto ipc_after=static_cast<char*>(MapViewOfFile(ipc_open,FILE_MAP_READ|FILE_MAP_WRITE,0,0,4096));
    if(!ipc_after){std::printf("Named pagefile MAP-only view failed: Win32 %lu\n",GetLastError());return 38;}
    if(ipc_after[0]!='I')return 39;
    ipc_after[0]='P';UnmapViewOfFile(ipc_after);CloseHandle(ipc_open);CloseHandle(ipc);
    // Removing SECTION_QUERY must not turn a protected file section into IPC.
    HANDLE file_map_only=nullptr;
    if(!DuplicateHandle(GetCurrentProcess(),old_mapping,GetCurrentProcess(),&file_map_only,FILE_MAP_READ|FILE_MAP_WRITE,FALSE,0))return 40;
    if(auto p=MapViewOfFile(file_map_only,FILE_MAP_WRITE,0,0,0)){UnmapViewOfFile(p);return 41;}
    CloseHandle(file_map_only);
    using CopyChunk=NTSTATUS(NTAPI*)(HANDLE,HANDLE,HANDLE,PIO_STATUS_BLOCK,ULONG,PLARGE_INTEGER,PLARGE_INTEGER,PULONG,PULONG,ULONG);
    auto copy_chunk=reinterpret_cast<CopyChunk>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtCopyFileChunk"));
    IO_STATUS_BLOCK io{};LARGE_INTEGER zero{};
    if(copy_chunk && copy_chunk(before,before,nullptr,&io,1,&zero,&zero,nullptr,nullptr,0)!=static_cast<NTSTATUS>(0xC0000022UL))return 31;
    DWORD n=0;char old{};
    using VectorIO=NTSTATUS(NTAPI*)(HANDLE,HANDLE,PIO_APC_ROUTINE,PVOID,PIO_STATUS_BLOCK,PVOID,ULONG,PLARGE_INTEGER,PULONG);
    for(const char* name:{"NtReadFileScatter","NtWriteFileGather"}) {
        auto fn=reinterpret_cast<VectorIO>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),name));
        if(!fn || fn(before,nullptr,nullptr,nullptr,&io,nullptr,4096,&zero,nullptr)!=static_cast<NTSTATUS>(0xC0000022UL))return 33;
    }
    if(WriteFile(before,"X",1,&n,nullptr) || ReadFile(before,&old,1,&n,nullptr))return 6;
    LARGE_INTEGER original_size{};
    if(GetFileSizeEx(before,&original_size) || FlushFileBuffers(before))return 34;
    FILE_END_OF_FILE_INFO end{};end.EndOfFile.QuadPart=1;
    if(SetFileInformationByHandle(before,FileEndOfFileInfo,&end,sizeof(end)))return 7;
    if(auto p=MapViewOfFile(old_mapping,FILE_MAP_WRITE,0,0,0)){UnmapViewOfFile(p);return 8;}
    CloseHandle(old_mapping);CloseHandle(before);
    const auto small_path=root+L"\\source.save";
    HANDLE small_thread=CreateThread(nullptr,128u*1024,&small_stack_mapping,const_cast<wchar_t*>(small_path.c_str()),STACK_SIZE_PARAM_IS_A_RESERVATION,nullptr);
    if(!small_thread || WaitForSingleObject(small_thread,10000)!=WAIT_OBJECT_0)return 56;
    DWORD small_result=~0u;if(!GetExitCodeThread(small_thread,&small_result))return 57;CloseHandle(small_thread);
    if(small_result)return static_cast<int>(small_result);
    if(mode==L"teardown")return 0; // ordinary CRT exit must safely remove native hooks
    if(read(root+L"\\source.save")!="original")return 9;
    if(!write(root+L"\\created.save","guest") || read(root+L"\\created.save")!="guest")return 10;
    HANDLE f=CreateFileW((root+L"\\created.save").c_str(),DELETE|GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    if(f==INVALID_HANDLE_VALUE)return 11;
    std::wstring destination=root+L"\\renamed.save";
    // KernelBase on this OS scans the Win32 name through its terminator when
    // building the counted native name. Keep the terminator outside the count.
    std::vector<uint8_t> buffer(offsetof(FILE_RENAME_INFO,FileName)+(destination.size()+1)*sizeof(wchar_t));
    auto* rename=reinterpret_cast<FILE_RENAME_INFO*>(buffer.data());rename->ReplaceIfExists=FALSE;
    rename->RootDirectory=nullptr;rename->FileNameLength=static_cast<DWORD>(destination.size()*sizeof(wchar_t));
    std::memcpy(rename->FileName,destination.data(),rename->FileNameLength);
    if(!SetFileInformationByHandle(f,FileRenameInfo,rename,static_cast<DWORD>(buffer.size())))return 12;
    CloseHandle(f);
    if(read(destination)!="guest")return 13;
    BOOL copied=CopyFileW(destination.c_str(),(root+L"\\copied.save").c_str(),TRUE);

    if(!copied)return 14;
    if(read(root+L"\\copied.save")!="guest")return 32;
    if(!DeleteFileW((root+L"\\copied.save").c_str()))return 15;
    if(!CreateDirectoryW((root+L"\\newdir").c_str(),nullptr))return 16;
    if(!write(root+L"\\newdir\\nested.save","nested"))return 17;
    if(!SetCurrentDirectoryW(root.c_str()) || !write(L"relative.save","relative"))return 18;
    if(!SetCurrentDirectoryW(backup.c_str()))return 19;
    if(!write(root+L"\\newdir\\..\\dot.save","dot"))return 20;
    if(!write(L"\\\\?\\"+root+L"\\extended.save","extended"))return 21;
    if(MoveFileExW(destination.c_str(),nullptr,MOVEFILE_DELAY_UNTIL_REBOOT))return 22;
    if(CreateHardLinkW((root+L"\\link.save").c_str(),destination.c_str(),nullptr))return 23;
    std::wstring external=root+L"\\..\\escape.save";
    if(MoveFileExW(destination.c_str(),external.c_str(),0))return 24;
    // Memory mapped writes through a NEW protected-path open must modify only its sandbox copy.
    HANDLE mapped=CreateFileW((root+L"\\source.save").c_str(),GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    if(mapped==INVALID_HANDLE_VALUE)return 25;
    HANDLE section=CreateFileMappingW(mapped,nullptr,PAGE_READWRITE,0,0,nullptr);if(!section)return 26;
    auto view=static_cast<char*>(MapViewOfFile(section,FILE_MAP_WRITE,0,0,0));if(!view)return 27;
    view[0]='G';FlushViewOfFile(view,1);UnmapViewOfFile(view);CloseHandle(section);CloseHandle(mapped);
    if(read(root+L"\\source.save")!="Griginal")return 28;
    std::printf("R4 file sandbox fixture PASS: %s\n",cdmp::session2::file_sandbox3663::describe().c_str());
    return 0;
}
}
int wmain(int argc,wchar_t** argv) {
    AddVectoredExceptionHandler(1,[](PEXCEPTION_POINTERS e)->LONG {
        if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
            cdmp::session2::file_sandbox3663::fixture_abort();
            MEMORY_BASIC_INFORMATION memory{};wchar_t module[512]{};
            VirtualQuery(e->ExceptionRecord->ExceptionAddress,&memory,sizeof(memory));
            GetModuleFileNameW(static_cast<HMODULE>(memory.AllocationBase),module,512);
            std::printf("Fixture AV address %p module offset %llX, stage %s\n",e->ExceptionRecord->ExceptionAddress,
                static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<uintptr_t>(memory.AllocationBase)),
                cdmp::session2::file_sandbox3663::failure_reason().c_str());
            std::fflush(stdout);
            ExitProcess(101);
        }
        return EXCEPTION_CONTINUE_SEARCH;
    });
    int code=fixture(argc,argv);if(code)std::printf("R4 sandbox fixture FAILED %d Win32 %lu\n",code,GetLastError());
    std::fflush(stdout);
    if(argc>=7 && std::wstring(argv[6])==L"teardown")return code;
    ExitProcess(static_cast<UINT>(code));
}
