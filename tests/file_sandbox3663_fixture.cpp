// Dedicated child process: hooks live until ExitProcess. The unhooked parent
// compares SHA256 manifests of the original tree and reads sandbox effects.
#include "mod/session2/file_sandbox3663.h"
#include <windows.h>
#include <winternl.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
namespace {
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
int fixture(int argc,wchar_t** argv) {
    if(argc!=7)return 1;
    const std::wstring root=argv[1],backup=argv[2],sandbox=argv[3],manifest=argv[4],hash=argv[5],mode=argv[6];
    HANDLE before=CreateFileW((root+L"\\source.save").c_str(),GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    if(before==INVALID_HANDLE_VALUE)return 2;
    HANDLE old_mapping=CreateFileMappingW(before,nullptr,PAGE_READWRITE,0,0,nullptr);
    if(!old_mapping)return 3;
    if(mode==L"premapped" && !MapViewOfFile(old_mapping,FILE_MAP_WRITE,0,0,0))return 30;
    std::string ascii;for(wchar_t c:hash)ascii+=static_cast<char>(c);
    bool installed=cdmp::session2::file_sandbox3663::install({root,backup,sandbox,manifest,ascii});
    if(mode==L"refuse" || mode==L"premapped")return installed?4:0;
    if(!installed || !cdmp::session2::file_sandbox3663::verified()){
        std::printf("Install failed: %s\n",cdmp::session2::file_sandbox3663::failure_reason().c_str());return 5;}
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
    std::fflush(stdout);ExitProcess(static_cast<UINT>(code));
}
