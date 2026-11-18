// R4 file-only sandbox. NT path APIs cover the Win32/CRT frontends; old real
// handles are denied. Internal bypass is private and used only for read-only
// validation/query operations, never for the fixture/self-check mutations.
#include "mod/session2/file_sandbox3663.h"
#define NTDDI_VERSION 0x0A000002 // QueryVirtualMemoryInformation (Windows 10 RS1)
#include <windows.h>
#include <winternl.h>
#include <shlobj.h>
#include <psapi.h>
#include <bcrypt.h>
#include <safetyhook.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <source_location>
#include <string_view>
#include <tuple>
#include <vector>
#include "mod/log.h"

namespace cdmp::session2::file_sandbox3663 {
namespace {
constexpr NTSTATUS denied=static_cast<NTSTATUS>(0xC0000022UL);
constexpr ULONG dont_reparse=0x1000, open_by_id=0x2000;
std::wstring real_root,sandbox_root;
std::atomic<bool> active{},ready{};
bool profile_method=false;
std::atomic<uint64_t> redirected{},refused{};
std::array<std::atomic<uint64_t>,1024> refusal_sites{};
constexpr size_t max_name_samples=20, max_name_chars=256;
struct RefusedName {
    std::array<wchar_t,max_name_chars> text{};
    USHORT original_bytes{};
    bool relative{};
    bool operator==(const RefusedName&) const = default;
};
SRWLOCK name_sample_lock=SRWLOCK_INIT;
std::array<RefusedName,max_name_samples> name_samples{};
std::atomic<size_t> names_sampled{};
std::atomic<uint64_t> named_refusals{};
std::string failure="not installed";
thread_local unsigned audit_depth=0;
struct ErrorState { DWORD value=GetLastError(); ~ErrorState(){SetLastError(value);} };
struct Audit { Audit(){++audit_depth;} ~Audit(){--audit_depth;} };
using FileId=std::tuple<DWORD,DWORD,DWORD>;
std::set<FileId> real_ids;
std::set<FileId> owned_log_ids;
std::vector<std::pair<std::wstring,std::wstring>> volumes;
HANDLE pinned_real=INVALID_HANDLE_VALUE,pinned_sandbox=INVALID_HANDLE_VALUE;
SafetyHookInline h_create,h_open,h_delete,h_attrs,h_fullattrs,h_set,h_read,h_write,h_section,h_section_ex,
    h_map,h_map_ex,h_scatter,h_gather,h_copy,h_byname,h_querydir,h_querydir_ex,h_fsctl,h_queryinfo,h_setea,h_queryea,h_setsecurity,h_querysecurity,h_flush,h_flush_ex,h_lock,h_unlock,h_move_w,h_move_a;
using Apc=PIO_APC_ROUTINE;
bool checked_copy(const void* from,void* to,size_t size) noexcept {
    __try { std::memcpy(to,from,size);return true; }
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
bool under(const std::wstring& path,const std::wstring& root) {
    if(path.size()<root.size() || CompareStringOrdinal(path.data(),static_cast<int>(root.size()),
        root.data(),static_cast<int>(root.size()),TRUE)!=CSTR_EQUAL)return false;
    return path.size()==root.size() || path[root.size()]==L'\\' || path[root.size()]==L':';
}
std::wstring final_name(HANDLE file) {
    Audit audit;
    std::vector<wchar_t> buffer(32768);
    DWORD n=GetFinalPathNameByHandleW(file,buffer.data(),static_cast<DWORD>(buffer.size()),FILE_NAME_NORMALIZED);
    if(!n || n>=buffer.size())return {};
    std::wstring name(buffer.data(),n);
    if(name.rfind(L"\\\\?\\UNC\\",0)==0)return {};
    if(name.rfind(L"\\\\?\\",0)==0)name.erase(0,4);
    while(name.size()>3 && name.back()==L'\\')name.pop_back();
    return name;
}
FileId file_id(HANDLE file) {
    Audit audit;BY_HANDLE_FILE_INFORMATION info{};
    if(!GetFileInformationByHandle(file,&info))throw 1;
    return {info.dwVolumeSerialNumber,info.nFileIndexHigh,info.nFileIndexLow};
}
bool bad_handle(HANDLE file,bool mutating=false) {
    if(!file || file==INVALID_HANDLE_VALUE)return true;
    Audit audit;
    if(GetFileType(file)!=FILE_TYPE_DISK)return false;
    const auto name=final_name(file);
    if(name.empty() || under(name,real_root))return true;
    BY_HANDLE_FILE_INFORMATION info{};
    if(!GetFileInformationByHandle(file,&info)){
        return true;
    }
    if(real_ids.contains({info.dwVolumeSerialNumber,info.nFileIndexHigh,info.nFileIndexLow}))return true;
    const FileId id{info.dwVolumeSerialNumber,info.nFileIndexHigh,info.nFileIndexLow};
    return info.nNumberOfLinks>1 && (mutating || under(name,sandbox_root)) && !owned_log_ids.contains(id);
}
std::wstring widen(const std::string& s) {
    if(s.empty())return {};
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),nullptr,0);
    if(!n)throw 1;
    std::wstring out(n,L'\0');
    if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),out.data(),n))throw 1;
    return out;
}
std::string sha256(const std::filesystem::path& path) {
    Audit audit;BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE hash=nullptr;
    if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)throw 1;
    DWORD size=0,got=0;
    if(BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&size),sizeof(size),&got,0)<0){BCryptCloseAlgorithmProvider(alg,0);throw 1;}
    std::vector<uint8_t> object(size);std::array<uint8_t,32> digest{};
    if(BCryptCreateHash(alg,&hash,object.data(),size,nullptr,0,0)<0){BCryptCloseAlgorithmProvider(alg,0);throw 1;}
    bool ok=true;std::ifstream input(path,std::ios::binary);std::array<char,65536> block{};
    if(!input)ok=false;
    while(input){input.read(block.data(),block.size());auto n=input.gcount();
        if(n && BCryptHashData(hash,reinterpret_cast<PUCHAR>(block.data()),static_cast<ULONG>(n),0)<0){ok=false;break;}}
    ok=ok && input.eof() && BCryptFinishHash(hash,digest.data(),static_cast<ULONG>(digest.size()),0)>=0;
    BCryptDestroyHash(hash);BCryptCloseAlgorithmProvider(alg,0);if(!ok)throw 1;
    constexpr char hex[]="0123456789ABCDEF";std::string out;
    for(auto b:digest){out+=hex[b>>4];out+=hex[b&15];}return out;
}
bool plain_tree(const std::wstring& root) {
    Audit audit;
    auto check=[](const std::filesystem::path& p){
        DWORD a=GetFileAttributesW(p.c_str());
        if(a==INVALID_FILE_ATTRIBUTES || (a&FILE_ATTRIBUTE_REPARSE_POINT))return false;
        HANDLE h=CreateFileW(p.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
            nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);
        if(h==INVALID_HANDLE_VALUE)return false;
        BY_HANDLE_FILE_INFORMATION info{};bool ok=GetFileInformationByHandle(h,&info) && info.nNumberOfLinks==1;
        CloseHandle(h);return ok;
    };
    if(!check(root))return false;
    for(const auto& entry:std::filesystem::recursive_directory_iterator(root))if(!check(entry.path()))return false;
    return true;
}

bool equal_ordinal(std::wstring_view a,std::wstring_view b) {
    return a.size()==b.size() && CompareStringOrdinal(a.data(),static_cast<int>(a.size()),
        b.data(),static_cast<int>(b.size()),TRUE)==CSTR_EQUAL;
}

bool netbt_endpoint(std::wstring_view path) {
    // Adapter-specific Windows device name. A full GUID is required and a
    // suffix (including a path separator or alternate stream) never qualifies.
    constexpr std::wstring_view prefix=L"\\Device\\NetBT_Tcpip_{";
    if(path.size()!=prefix.size()+37 || !equal_ordinal(path.substr(0,prefix.size()),prefix) ||
        path.back()!=L'}')return false;
    const auto guid=path.substr(prefix.size(),36);
    for(size_t i=0;i<guid.size();++i){
        const wchar_t c=guid[i];
        if(i==8 || i==13 || i==18 || i==23){
            if(c!=L'-')return false;
        }else if(!((c>=L'0' && c<=L'9') || (c>=L'a' && c<=L'f') || (c>=L'A' && c<=L'F'))){
            return false;
        }
    }
    return true;
}

std::wstring network_device_target(const std::wstring& path) {
    if(netbt_endpoint(path))return path;
    constexpr std::wstring_view prefix=L"\\??\\";
    if(!path.starts_with(prefix))return {};
    const auto alias=std::wstring_view(path).substr(prefix.size());
    if(alias.empty() || alias.size()>255 || alias.find_first_of(L"\\/:")!=alias.npos)return {};

    // A DOS device alias can point to a file or volume. Approve the resolved
    // native network endpoint, then use that endpoint for the actual open.
    // Never reopen through the mutable alias or trust a broad Device prefix.
    Audit audit;
    std::array<wchar_t,1024> mapping{};
    if(!QueryDosDeviceW(path.c_str()+prefix.size(),mapping.data(),
        static_cast<DWORD>(mapping.size())))return {};
    constexpr std::wstring_view endpoints[]{
        L"\\Device\\Nsi", L"\\Device\\Ndis", L"\\Device\\Afd", L"\\Device\\Tcp", L"\\Device\\Tcp6",
        L"\\Device\\Udp", L"\\Device\\Udp6", L"\\Device\\Ip", L"\\Device\\Ip6"
    };
    const std::wstring_view current(mapping.data());
    for(const auto endpoint:endpoints)if(equal_ordinal(current,endpoint))return std::wstring(current);
    return {};
}

// Resolve an existing ancestor before appending missing leaf components. The
// actual native call uses that absolute name with OBJ_DONT_REPARSE, so a
// post-check reparse substitution fails instead of escaping the sandbox.
std::wstring canonical(std::wstring path,bool mutation) {
    Audit audit;
    if(path.rfind(L"\\??\\",0)==0)path.erase(0,4);
    if(path.rfind(L"\\\\?\\",0)==0)path.erase(0,4);
    if(path.rfind(L"GLOBALROOT\\",0)==0)path=L"\\"+path.substr(11);
    if(path.rfind(L"\\SystemRoot\\",0)==0) {
        std::vector<wchar_t> windows(32768);
        const auto length=GetWindowsDirectoryW(windows.data(),static_cast<UINT>(windows.size()));
        if(!length || length>=windows.size())throw 1;
        path=std::wstring(windows.data(),length)+path.substr(11);
    }
    if(path.rfind(L"\\Device\\",0)==0) {
        bool mapped=false;
        for(const auto& [nt,dos]:volumes)if(under(path,nt)){path=dos+path.substr(nt.size());mapped=true;break;}
        if(!mapped) {
            if(path.rfind(L"\\Device\\Harddisk",0)==0 || path.rfind(L"\\Device\\Mup",0)==0)throw 1;
            return {}; // non-filesystem device (AFD, graphics, pipes, console)
        }
    }
    if(path.rfind(L"UNC\\",0)==0 || path.rfind(L"\\\\",0)==0)throw 1;
    if(path.size()<3 || path[1]!=L':')throw 1;
    std::vector<wchar_t> full(32768);
    DWORD count=GetFullPathNameW(path.c_str(),static_cast<DWORD>(full.size()),full.data(),nullptr);
    if(!count || count>=full.size())throw 1;
    path.assign(full.data(),count);std::wstring tail;
    for(size_t depth=0;depth<256;++depth) {
        HANDLE h=CreateFileW(path.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
            nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);
        if(h!=INVALID_HANDLE_VALUE) {
            auto normalized=final_name(h);BY_HANDLE_FILE_INFORMATION info{};
            bool ok=GetFileInformationByHandle(h,&info)!=FALSE;
            const bool alias=ok && real_ids.contains({info.dwVolumeSerialNumber,info.nFileIndexHigh,info.nFileIndexLow}) &&
                !under(normalized,real_root);
            const FileId id{info.dwVolumeSerialNumber,info.nFileIndexHigh,info.nFileIndexLow};
            const bool links=ok && info.nNumberOfLinks>1 && (mutation || under(normalized,sandbox_root)) && !owned_log_ids.contains(id);
            CloseHandle(h);
            if(normalized.empty() || !ok || alias || links)throw 1;
            return normalized+tail;
        }
        const auto slash=path.find_last_of(L'\\');if(slash==std::wstring::npos || slash<2)throw 1;
        tail=path.substr(slash)+tail;path.resize(slash==2?3:slash);
    }
    throw 1;
}
struct Name {
    OBJECT_ATTRIBUTES attributes{};UNICODE_STRING unicode{};std::wstring storage;
    bool disk=false,hit=false,device=false;

    void bind_absolute() {
        if(storage.size()>32766)throw 1;
        unicode.Length=static_cast<USHORT>(storage.size()*sizeof(wchar_t));
        unicode.MaximumLength=unicode.Length;
        unicode.Buffer=storage.data();
        attributes.RootDirectory=nullptr;
        attributes.ObjectName=&unicode;
        attributes.Attributes|=dont_reparse;
    }

    explicit Name(POBJECT_ATTRIBUTES given,bool mutation=false) {
        if(!given || !checked_copy(given,&attributes,sizeof(attributes)) || attributes.Length!=sizeof(attributes))throw 1;
        UNICODE_STRING name{};
        if(!attributes.ObjectName || !checked_copy(attributes.ObjectName,&name,sizeof(name)) ||
            name.Length%2 || name.Length>65532 || (name.Length && !name.Buffer))throw 1;
        storage.resize(name.Length/2);
        if(name.Length && !checked_copy(name.Buffer,storage.data(),name.Length))throw 1;
        if(storage.find(L'\0')!=std::wstring::npos)throw 1;
        if(attributes.RootDirectory && storage.empty()){
            Audit audit;
            // Native reopen of an existing pipe endpoint has no disk name.
            // Only the kernel's PIPE handle type qualifies; disk/unknown
            // handles still require normal save-path/identity validation.
            if(GetFileType(attributes.RootDirectory)==FILE_TYPE_PIPE)return;
        }
        if(attributes.RootDirectory && (storage.empty() || storage[0]!=L'\\')) {
            auto parent=final_name(attributes.RootDirectory);if(parent.empty())throw 1;
            storage=parent+L"\\"+storage;
        }
        constexpr wchar_t pipe_prefix[]=L"\\??\\pipe\\";
        if(storage.size()>=std::size(pipe_prefix)-1 &&
            CompareStringOrdinal(storage.data(),std::size(pipe_prefix)-1,pipe_prefix,std::size(pipe_prefix)-1,TRUE)==CSTR_EQUAL){
            // A DOS pipe name is not a disk path. Bind it to the explicit
            // native pipe device rather than passing a mutable DOS alias
            // through or attempting a second client open during validation.
            storage=L"\\Device\\NamedPipe\\"+storage.substr(std::size(pipe_prefix)-1);
            bind_absolute();
            device=true;return;
        }
        if(!attributes.RootDirectory){
            auto target=network_device_target(storage);
            if(!target.empty()){
                storage=std::move(target);
                bind_absolute();
                device=true;
                return;
            }
        }
        auto resolved=canonical(storage,mutation);
        if(resolved.empty())return;
        disk=true;
        if(under(resolved,real_root)){resolved=sandbox_root+resolved.substr(real_root.size());hit=true;++redirected;}
        storage=L"\\??\\"+resolved;
        bind_absolute();
    }
    POBJECT_ATTRIBUTES arg(POBJECT_ATTRIBUTES original){return disk||device?&attributes:original;}
};
bool enabled(){return active.load(std::memory_order_acquire) && !audit_depth;}
NTSTATUS deny(std::source_location site=std::source_location::current()){
    ++refused;if(site.line()<refusal_sites.size())++refusal_sites[site.line()];return denied;
}
void log_refused_name(const char* operation,POBJECT_ATTRIBUTES oa,std::source_location site) noexcept {
    ++named_refusals;
    if(!log::is_open() || names_sampled.load(std::memory_order_relaxed)>=max_name_samples)return;
    OBJECT_ATTRIBUTES attrs{};
    UNICODE_STRING name{};
    if(!checked_copy(oa,&attrs,sizeof(attrs)) || attrs.Length!=sizeof(attrs) ||
        !checked_copy(attrs.ObjectName,&name,sizeof(name)) || name.Length%sizeof(wchar_t))return;
    RefusedName sample{};
    sample.original_bytes=name.Length;
    sample.relative=attrs.RootDirectory!=nullptr;
    const auto length=std::min(sample.text.size(),static_cast<size_t>(name.Length)/sizeof(wchar_t));
    if(length && !checked_copy(name.Buffer,sample.text.data(),length*sizeof(wchar_t)))return;
    // No blocking lock on a hooked thread. Release it before queuing to the
    // async logger so nothing that writes a log can hold the sampling lock.
    if(!TryAcquireSRWLockExclusive(&name_sample_lock))return;
    const auto used=names_sampled.load(std::memory_order_relaxed);
    const auto end=name_samples.begin()+used;
    const bool fresh=used<max_name_samples && std::find(name_samples.begin(),end,sample)==end;
    if(fresh){
        name_samples[used]=sample;
        names_sampled.store(used+1,std::memory_order_relaxed);
    }
    ReleaseSRWLockExclusive(&name_sample_lock);
    if(!fresh)return;
    for(auto& c:sample.text)if(c<L' ' || (c>=0x7f && c<=0x9f))c=L'?';
    char utf8[max_name_chars*3]{};
    const int bytes=length?WideCharToMultiByte(CP_UTF8,0,sample.text.data(),static_cast<int>(length),
        utf8,sizeof(utf8),nullptr,nullptr):0;
    const std::string_view display=length==0?"<empty>":bytes>0?std::string_view(utf8,bytes):"<invalid UTF-16>";
    try {
        log::warn("3663 sandbox denied object: api={} line={} root_relative={} name='{}'{}",
            operation,site.line(),sample.relative,display,length*sizeof(wchar_t)<sample.original_bytes?" [truncated]":"");
    }catch(...){ /* Diagnostics do not change the NT refusal. */ }
}
NTSTATUS deny_name(const char* operation,POBJECT_ATTRIBUTES oa,
    std::source_location site=std::source_location::current()){
    const auto result=deny(site);
    log_refused_name(operation,oa,site);
#ifdef CDMP_SANDBOX_FIXTURE
    // The owned fixture also emits all readable refused names to stderr.
    Audit audit;
    try {
        OBJECT_ATTRIBUTES attrs{};
        UNICODE_STRING name{};
        if(checked_copy(oa,&attrs,sizeof(attrs)) && attrs.Length==sizeof(attrs) &&
            checked_copy(attrs.ObjectName,&name,sizeof(name)) && !(name.Length%2)){
            std::wstring text(name.Length/sizeof(wchar_t),L'\0');
            if(text.empty() || checked_copy(name.Buffer,text.data(),name.Length)){
                std::fwprintf(stderr,L"Fixture refusal %hs line=%u name='%ls' root=%p\n",
                    operation,static_cast<unsigned>(site.line()),text.c_str(),attrs.RootDirectory);
                return result;
            }
        }
    }catch(...){}
    std::fwprintf(stderr,L"Fixture refusal %hs line=%u: unreadable object name\n",
        operation,static_cast<unsigned>(site.line()));
#endif
    return result;
}
NTSTATUS NTAPI create_file(PHANDLE out,ACCESS_MASK access,POBJECT_ATTRIBUTES oa,PIO_STATUS_BLOCK io,
    PLARGE_INTEGER alloc,ULONG attr,ULONG share,ULONG disposition,ULONG options,PVOID ea,ULONG ea_size) {
    ErrorState error_state;
    if(!enabled())return h_create.unsafe_stdcall<NTSTATUS>(out,access,oa,io,alloc,attr,share,disposition,options,ea,ea_size);
    try {
        if(options&open_by_id)return deny_name("NtCreateFile",oa);
        const bool mutation=(access&(GENERIC_WRITE|GENERIC_ALL|MAXIMUM_ALLOWED|DELETE|FILE_WRITE_DATA|FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES|WRITE_DAC|WRITE_OWNER)) || disposition!=1 || (options&0x1000);
        Name name(oa,mutation);
        return h_create.unsafe_stdcall<NTSTATUS>(out,access|(name.disk?FILE_READ_ATTRIBUTES:0),name.arg(oa),io,alloc,attr,share,disposition,options,ea,ea_size);
    }catch(...){return deny_name("NtCreateFile",oa);}
}
NTSTATUS NTAPI open_file(PHANDLE out,ACCESS_MASK access,POBJECT_ATTRIBUTES oa,PIO_STATUS_BLOCK io,ULONG share,ULONG options) {
    ErrorState error_state;
    if(!enabled())return h_open.unsafe_stdcall<NTSTATUS>(out,access,oa,io,share,options);
    try {if(options&open_by_id)return deny_name("NtOpenFile",oa);Name name(oa,(access&(GENERIC_WRITE|GENERIC_ALL|DELETE|FILE_WRITE_DATA|FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES))!=0);
        return h_open.unsafe_stdcall<NTSTATUS>(out,access|(name.disk?FILE_READ_ATTRIBUTES:0),name.arg(oa),io,share,options);
    }catch(...){return deny_name("NtOpenFile",oa);}
}
NTSTATUS NTAPI delete_file(POBJECT_ATTRIBUTES oa) {
    ErrorState error_state;
    if(!enabled())return h_delete.unsafe_stdcall<NTSTATUS>(oa);
    try{Name name(oa,true);return h_delete.unsafe_stdcall<NTSTATUS>(name.arg(oa));}catch(...){return deny_name("NtDeleteFile",oa);}
}
NTSTATUS NTAPI query_attrs(POBJECT_ATTRIBUTES oa,PVOID info) {
    ErrorState error_state;
    if(!enabled())return h_attrs.unsafe_stdcall<NTSTATUS>(oa,info);
    try{Name name(oa);return h_attrs.unsafe_stdcall<NTSTATUS>(name.arg(oa),info);}catch(...){return deny_name("NtQueryAttributesFile",oa);}
}
NTSTATUS NTAPI query_fullattrs(POBJECT_ATTRIBUTES oa,PVOID info) {
    ErrorState error_state;
    if(!enabled())return h_fullattrs.unsafe_stdcall<NTSTATUS>(oa,info);
    try{Name name(oa);return h_fullattrs.unsafe_stdcall<NTSTATUS>(name.arg(oa),info);}catch(...){return deny_name("NtQueryFullAttributesFile",oa);}
}
NTSTATUS NTAPI read_file(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,PVOID buffer,ULONG length,PLARGE_INTEGER offset,PULONG key) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f))return deny();}catch(...){return deny();}
    return h_read.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,buffer,length,offset,key);
}
NTSTATUS NTAPI write_file(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,PVOID buffer,ULONG length,PLARGE_INTEGER offset,PULONG key) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f,true))return deny();}catch(...){return deny();}
    return h_write.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,buffer,length,offset,key);
}
// Scatter/gather has the same native call shape; buffers remain opaque.
NTSTATUS NTAPI scatter(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,PVOID segments,ULONG length,PLARGE_INTEGER offset,PULONG key) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f))return deny();}catch(...){return deny();}
    return h_scatter.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,segments,length,offset,key);
}
NTSTATUS NTAPI gather(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,PVOID segments,ULONG length,PLARGE_INTEGER offset,PULONG key) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f,true))return deny();}catch(...){return deny();}
    return h_gather.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,segments,length,offset,key);
}
NTSTATUS NTAPI copy_chunk(HANDLE source,HANDLE destination,HANDLE event,PIO_STATUS_BLOCK io,ULONG length,PLARGE_INTEGER from,PLARGE_INTEGER to,PULONG source_key,PULONG destination_key,ULONG flags) {
    ErrorState error_state;
    try{if(enabled() && (bad_handle(source) || bad_handle(destination,true)))return deny();}catch(...){return deny();}
    return h_copy.unsafe_stdcall<NTSTATUS>(source,destination,event,io,length,from,to,source_key,destination_key,flags);
}
NTSTATUS NTAPI query_byname(POBJECT_ATTRIBUTES oa,PIO_STATUS_BLOCK io,PVOID info,ULONG length,ULONG kind) {
    ErrorState error_state;
    if(!enabled())return h_byname.unsafe_stdcall<NTSTATUS>(oa,io,info,length,kind);
    try{Name name(oa);return h_byname.unsafe_stdcall<NTSTATUS>(name.arg(oa),io,info,length,kind);}catch(...){return deny_name("NtQueryInformationByName",oa);}
}
NTSTATUS NTAPI query_info(HANDLE file,PIO_STATUS_BLOCK io,PVOID info,ULONG size,ULONG kind) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(file))return deny();}catch(...){return deny();}
    return h_queryinfo.unsafe_stdcall<NTSTATUS>(file,io,info,size,kind);
}
NTSTATUS NTAPI set_ea(HANDLE file,PIO_STATUS_BLOCK io,PVOID info,ULONG size) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(file,true))return deny();}catch(...){return deny();}
    return h_setea.unsafe_stdcall<NTSTATUS>(file,io,info,size);
}
NTSTATUS NTAPI query_ea(HANDLE file,PIO_STATUS_BLOCK io,PVOID info,ULONG size,BOOLEAN single,PVOID list,ULONG list_size,PULONG index,BOOLEAN restart) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(file))return deny();}catch(...){return deny();}
    return h_queryea.unsafe_stdcall<NTSTATUS>(file,io,info,size,single,list,list_size,index,restart);
}
NTSTATUS NTAPI set_security(HANDLE file,ULONG kind,PVOID descriptor) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(file,true))return deny();}catch(...){return deny();}
    return h_setsecurity.unsafe_stdcall<NTSTATUS>(file,kind,descriptor);
}
NTSTATUS NTAPI query_security(HANDLE file,ULONG kind,PVOID descriptor,ULONG size,PULONG needed) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(file))return deny();}catch(...){return deny();}
    return h_querysecurity.unsafe_stdcall<NTSTATUS>(file,kind,descriptor,size,needed);
}
NTSTATUS NTAPI flush(HANDLE file,PIO_STATUS_BLOCK io) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(file,true))return deny();}catch(...){return deny();}
    return h_flush.unsafe_stdcall<NTSTATUS>(file,io);
}
NTSTATUS NTAPI flush_ex(HANDLE file,ULONG flags,PVOID parameters,ULONG size,PIO_STATUS_BLOCK io) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(file,true))return deny();}catch(...){return deny();}
    return h_flush_ex.unsafe_stdcall<NTSTATUS>(file,flags,parameters,size,io);
}
NTSTATUS NTAPI lock_file(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,PLARGE_INTEGER offset,PLARGE_INTEGER size,ULONG key,BOOLEAN immediate,BOOLEAN exclusive) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f))return deny();}catch(...){return deny();}
    return h_lock.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,offset,size,key,immediate,exclusive);
}
NTSTATUS NTAPI unlock_file(HANDLE f,PIO_STATUS_BLOCK io,PLARGE_INTEGER offset,PLARGE_INTEGER size,ULONG key) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f))return deny();}catch(...){return deny();}
    return h_unlock.unsafe_stdcall<NTSTATUS>(f,io,offset,size,key);
}
struct Rename { ULONG flags;HANDLE root;ULONG bytes;wchar_t name[1]; };
static_assert(offsetof(Rename,name)==20);
NTSTATUS NTAPI set_info(HANDLE file,PIO_STATUS_BLOCK io,PVOID info,ULONG length,ULONG kind) {
    ErrorState error_state;
    if(!enabled())return h_set.unsafe_stdcall<NTSTATUS>(file,io,info,length,kind);
    try {
        if(bad_handle(file,true))return deny();
        const bool rename=kind==10 || kind==56 || kind==65 || kind==66;
        const bool link=kind==11 || kind==57 || kind==72 || kind==73;
        if(rename || link) {
            Rename header{};
            if(length<20 || !checked_copy(info,&header,20) || header.bytes%2 || header.bytes>65532 || header.bytes>length-20)return deny();
            std::wstring target(header.bytes/2,L'\0');
            if(header.bytes && !checked_copy(static_cast<uint8_t*>(info)+20,target.data(),header.bytes))return deny();
            if(!header.root && (target.empty() || (target[0]!=L'\\' && !(target.size()>2 && target[1]==L':')))) {
                auto current=final_name(file);auto at=current.find_last_of(L'\\');if(at==std::wstring::npos)return deny();
                target=current.substr(0,at+1)+target;
            }
            UNICODE_STRING us{static_cast<USHORT>(target.size()*2),static_cast<USHORT>(target.size()*2),target.data()};
            OBJECT_ATTRIBUTES oa{};InitializeObjectAttributes(&oa,&us,OBJ_CASE_INSENSITIVE,header.root,nullptr);
            Name mapped(&oa,true);if(!mapped.disk)return deny();
            auto destination=mapped.storage.substr(4),source=final_name(file);
            if(under(source,sandbox_root)!=under(destination,sandbox_root) || link)return deny();
            std::vector<uint8_t> replacement(20+mapped.unicode.Length);
            header.root=nullptr;header.bytes=mapped.unicode.Length;
            std::memcpy(replacement.data(),&header,20);std::memcpy(replacement.data()+20,mapped.storage.data(),header.bytes);
            return h_set.unsafe_stdcall<NTSTATUS>(file,io,replacement.data(),static_cast<ULONG>(replacement.size()),kind);
        }
        return h_set.unsafe_stdcall<NTSTATUS>(file,io,info,length,kind);
    }catch(...){return deny();}
}
NTSTATUS NTAPI section(PHANDLE out,ACCESS_MASK access,POBJECT_ATTRIBUTES oa,PLARGE_INTEGER size,ULONG protect,ULONG flags,HANDLE file) {
    ErrorState error_state;
    try{if(enabled() && file && bad_handle(file,(protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))!=0))return deny();}catch(...){return deny();}
    return h_section.unsafe_stdcall<NTSTATUS>(out,access,oa,size,protect,flags,file);
}
NTSTATUS NTAPI section_ex(PHANDLE out,ACCESS_MASK access,POBJECT_ATTRIBUTES oa,PLARGE_INTEGER size,ULONG protect,ULONG flags,HANDLE file,PVOID extra,ULONG count) {
    ErrorState error_state;
    try{if(enabled() && file && bad_handle(file,(protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))!=0))return deny();}catch(...){return deny();}
    return h_section_ex.unsafe_stdcall<NTSTATUS>(out,access,oa,size,protect,flags,file,extra,count);
}
bool mapped_real(HANDLE process,PVOID base,HANDLE section_handle) {
    // Game IO threads can reserve only 128 KB. Nested 64 KB name arrays in
    // this callback and canonical() exhausted that stack before title.
    Audit audit;std::vector<wchar_t> name(32768);
    // Named IPC is commonly opened with SECTION_MAP_READ/WRITE only.
    // NtQuerySection then fails for lack of SECTION_QUERY even though the
    // view is valid. Classify its backing from the mapped region, as the
    // startup scan already does; only proven pagefile views bypass file
    // checks. Unknown and file-backed sections remain fail-closed.
    const auto query_region=reinterpret_cast<decltype(&QueryVirtualMemoryInformation)>(
        GetProcAddress(GetModuleHandleW(L"kernelbase.dll"),"QueryVirtualMemoryInformation"));
    WIN32_MEMORY_REGION_INFORMATION detail{};
    if(query_region && query_region(process,base,MemoryRegionInfo,&detail,sizeof(detail),nullptr) && detail.MappedPageFile)return false;
    DWORD n=K32GetMappedFileNameW(process,base,name.data(),static_cast<DWORD>(name.size()));
    if(n && n<name.size()) {auto path=canonical(std::wstring(name.data(),n),false);return path.empty() || under(path,real_root);}
    struct Basic { PVOID base;ULONG attributes;LARGE_INTEGER size; } basic{};
    using Query=NTSTATUS(NTAPI*)(HANDLE,ULONG,PVOID,ULONG,PULONG);
    auto query=reinterpret_cast<Query>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtQuerySection"));
    return !query || query(section_handle,0,&basic,sizeof(basic),nullptr)<0 || (basic.attributes&SEC_FILE)!=0;
}
NTSTATUS unmap_denied(HANDLE process,PVOID* base) {
    using Unmap=NTSTATUS(NTAPI*)(HANDLE,PVOID);
    PVOID address=nullptr;if(!checked_copy(base,&address,sizeof(address)))return deny();
    auto fn=reinterpret_cast<Unmap>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtUnmapViewOfSection"));
    if(!fn || fn(process,address)<0)TerminateProcess(GetCurrentProcess(),0xCD64);
    PVOID none=nullptr;(void)checked_copy(&none,base,sizeof(none));return deny();
}
NTSTATUS NTAPI map_section(HANDLE section_handle,HANDLE process,PVOID* base,ULONG_PTR zero,SIZE_T commit,PLARGE_INTEGER offset,PSIZE_T size,ULONG inherit,ULONG allocation,ULONG protect) {
    ErrorState error_state;
    auto result=h_map.unsafe_stdcall<NTSTATUS>(section_handle,process,base,zero,commit,offset,size,inherit,allocation,protect);
    if(result>=0 && enabled())try{PVOID address=nullptr;if(!checked_copy(base,&address,sizeof(address)) || mapped_real(process,address,section_handle))return unmap_denied(process,base);}catch(...){return unmap_denied(process,base);}
    return result;
}
NTSTATUS NTAPI map_section_ex(HANDLE section_handle,HANDLE process,PVOID* base,PLARGE_INTEGER offset,PSIZE_T size,ULONG allocation,ULONG protect,PVOID extra,ULONG count) {
    ErrorState error_state;
    auto result=h_map_ex.unsafe_stdcall<NTSTATUS>(section_handle,process,base,offset,size,allocation,protect,extra,count);
    if(result>=0 && enabled())try{PVOID address=nullptr;if(!checked_copy(base,&address,sizeof(address)) || mapped_real(process,address,section_handle))return unmap_denied(process,base);}catch(...){return unmap_denied(process,base);}
    return result;
}
NTSTATUS NTAPI query_dir(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,PVOID info,ULONG length,ULONG kind,BOOLEAN single,PUNICODE_STRING pattern,BOOLEAN restart) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f))return deny();}catch(...){return deny();}
    return h_querydir.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,info,length,kind,single,pattern,restart);
}
NTSTATUS NTAPI query_dir_ex(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,PVOID info,ULONG length,ULONG kind,ULONG flags,PUNICODE_STRING pattern) {
    ErrorState error_state;
    try{if(enabled() && bad_handle(f))return deny();}catch(...){return deny();}
    return h_querydir_ex.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,info,length,kind,flags,pattern);
}
NTSTATUS NTAPI fsctl(HANDLE f,HANDLE e,Apc apc,PVOID ctx,PIO_STATUS_BLOCK io,ULONG code,PVOID input,ULONG in_size,PVOID output,ULONG out_size) {
    ErrorState error_state;
    try{if(enabled() && (bad_handle(f,true) || code==0x900A4 || code==0x900AC || code==0x98344))return deny();}catch(...){return deny();}
    return h_fsctl.unsafe_stdcall<NTSTATUS>(f,e,apc,ctx,io,code,input,in_size,output,out_size);
}
BOOL WINAPI move_w(LPCWSTR from,LPCWSTR to,DWORD flags) {
    if(enabled() && (flags&MOVEFILE_DELAY_UNTIL_REBOOT)){++refused;SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    return h_move_w.unsafe_stdcall<BOOL>(from,to,flags);
}
BOOL WINAPI move_a(LPCSTR from,LPCSTR to,DWORD flags) {
    if(enabled() && (flags&MOVEFILE_DELAY_UNTIL_REBOOT)){++refused;SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    return h_move_a.unsafe_stdcall<BOOL>(from,to,flags);
}
struct Spec {const char* name;void* callback;SafetyHookInline* hook;bool optional=false;};
bool hooks() {
    HMODULE nt=GetModuleHandleW(L"ntdll.dll"),kb=GetModuleHandleW(L"kernelbase.dll");
    const Spec specs[]{
        {"NtCreateFile",reinterpret_cast<void*>(create_file),&h_create},{"NtOpenFile",reinterpret_cast<void*>(open_file),&h_open},
        {"NtDeleteFile",reinterpret_cast<void*>(delete_file),&h_delete},{"NtQueryAttributesFile",reinterpret_cast<void*>(query_attrs),&h_attrs},
        {"NtQueryFullAttributesFile",reinterpret_cast<void*>(query_fullattrs),&h_fullattrs},{"NtSetInformationFile",reinterpret_cast<void*>(set_info),&h_set},
        {"NtReadFile",reinterpret_cast<void*>(read_file),&h_read},{"NtWriteFile",reinterpret_cast<void*>(write_file),&h_write},
        {"NtCreateSection",reinterpret_cast<void*>(section),&h_section},{"NtCreateSectionEx",reinterpret_cast<void*>(section_ex),&h_section_ex,true},
        {"NtMapViewOfSection",reinterpret_cast<void*>(map_section),&h_map},{"NtMapViewOfSectionEx",reinterpret_cast<void*>(map_section_ex),&h_map_ex,true},
        {"NtQueryDirectoryFile",reinterpret_cast<void*>(query_dir),&h_querydir},{"NtQueryDirectoryFileEx",reinterpret_cast<void*>(query_dir_ex),&h_querydir_ex,true},
        {"NtReadFileScatter",reinterpret_cast<void*>(scatter),&h_scatter},
        {"NtWriteFileGather",reinterpret_cast<void*>(gather),&h_gather},
        {"NtCopyFileChunk",reinterpret_cast<void*>(copy_chunk),&h_copy,true},
        {"NtQueryInformationByName",reinterpret_cast<void*>(query_byname),&h_byname,true},
        {"NtQueryInformationFile",reinterpret_cast<void*>(query_info),&h_queryinfo},
        {"NtSetEaFile",reinterpret_cast<void*>(set_ea),&h_setea},
        {"NtQueryEaFile",reinterpret_cast<void*>(query_ea),&h_queryea},
        {"NtSetSecurityObject",reinterpret_cast<void*>(set_security),&h_setsecurity},
        {"NtQuerySecurityObject",reinterpret_cast<void*>(query_security),&h_querysecurity},
        {"NtFlushBuffersFile",reinterpret_cast<void*>(flush),&h_flush},
        {"NtFlushBuffersFileEx",reinterpret_cast<void*>(flush_ex),&h_flush_ex,true},
        {"NtLockFile",reinterpret_cast<void*>(lock_file),&h_lock},
        {"NtUnlockFile",reinterpret_cast<void*>(unlock_file),&h_unlock},
        {"NtFsControlFile",reinterpret_cast<void*>(fsctl),&h_fsctl},
        {"MoveFileExW",reinterpret_cast<void*>(move_w),&h_move_w},{"MoveFileExA",reinterpret_cast<void*>(move_a),&h_move_a}};
    for(const auto& spec:specs) {
        auto target=GetProcAddress(spec.name[0]=='N'?nt:kb,spec.name);
        if(!target && spec.name[0]!='N')target=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),spec.name);
        if(!target && spec.optional)continue;
        if(!target){failure=std::string("missing API ")+spec.name;return false;}
        *spec.hook=safetyhook::create_inline(reinterpret_cast<void*>(target),spec.callback,SafetyHookInline::StartDisabled);
        if(!*spec.hook){failure=std::string("cannot prepare ")+spec.name;return false;}
    }
    active.store(true,std::memory_order_release);
    for(const auto& spec:specs)if(*spec.hook && !spec.hook->enable()){failure=std::string("cannot enable ")+spec.name;return false;}
    return true;
}
struct Entry {std::string hash;uint64_t bytes;std::wstring relative;};
struct OrdinalLess {
    bool operator()(const std::wstring& a,const std::wstring& b) const {
        return CompareStringOrdinal(a.data(),static_cast<int>(a.size()),b.data(),static_cast<int>(b.size()),TRUE)==CSTR_LESS_THAN;
    }
};
bool verify_tree(const std::wstring& root,const std::vector<Entry>& entries,bool source=false,bool collect_ids=false) {
    Audit audit;if(!plain_tree(root))return false;
    size_t count=0;
    for(const auto& item:std::filesystem::recursive_directory_iterator(root))if(item.is_regular_file())++count;
    if(count!=entries.size())return false;
    for(const auto& entry:entries) {
        auto path=std::filesystem::path(root)/entry.relative;
        if(std::filesystem::file_size(path)!=entry.bytes || sha256(path)!=entry.hash)return false;
        HANDLE h=CreateFileW(path.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
        if(h==INVALID_HANDLE_VALUE)return false;
        auto id=file_id(h);CloseHandle(h);
        if(source){if(collect_ids)real_ids.insert(id);}else if(real_ids.contains(id))return false;
    }
    return true;
}
bool collect_source_ids(const std::wstring& root){
    Audit audit;if(!plain_tree(root))return false;
    for(const auto& item:std::filesystem::recursive_directory_iterator(root))if(item.is_regular_file()){
        HANDLE h=CreateFileW(item.path().c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
            nullptr,OPEN_EXISTING,0,nullptr);
        if(h==INVALID_HANDLE_VALUE)return false;
        try{real_ids.insert(file_id(h));}catch(...){CloseHandle(h);throw;}
        CloseHandle(h);
    }
    return true;
}
std::wstring env(const wchar_t* name) {
    std::vector<wchar_t> data(32768);DWORD n=GetEnvironmentVariableW(name,data.data(),static_cast<DWORD>(data.size()));
    return n && n<data.size()?std::wstring(data.data(),n):std::wstring();
}
}
bool install(const Setup& setup) {
    if(active.load() || setup.manifest_sha256.size()!=64)return false;
    std::vector<Entry> entries;
    try {
        Audit audit;
        failure="source/backup/sandbox plain tree validation";
        if(!plain_tree(setup.real) || !plain_tree(setup.backup) || !plain_tree(setup.sandbox))return false;
        auto pin=[](const std::wstring& p){return CreateFileW(p.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE,
            nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);};
        failure="pinning source/sandbox";pinned_real=pin(setup.real);pinned_sandbox=pin(setup.sandbox);
        if(pinned_real==INVALID_HANDLE_VALUE || pinned_sandbox==INVALID_HANDLE_VALUE)return false;
        real_root=final_name(pinned_real);sandbox_root=final_name(pinned_sandbox);
        if(real_root.empty() || sandbox_root.empty() || under(real_root,sandbox_root) || under(sandbox_root,real_root))return false;
        HANDLE backup_handle=pin(setup.backup);if(backup_handle==INVALID_HANDLE_VALUE)return false;
        const auto backup_root=final_name(backup_handle);CloseHandle(backup_handle);
        if(backup_root.empty() || under(backup_root,real_root) || under(real_root,backup_root) ||
            under(backup_root,sandbox_root) || under(sandbox_root,backup_root))return false;
        if(setup.profile_redirect){
            std::vector<wchar_t> local(MAX_PATH);
            if(SHGetFolderPathW(nullptr,CSIDL_LOCAL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,local.data())!=S_OK)return false;
            const auto resolved=std::wstring(local.data())+L"\\Pearl Abyss\\CD\\save";
            HANDLE native=pin(resolved);if(native==INVALID_HANDLE_VALUE)return false;
            bool equal=file_id(native)==file_id(pinned_sandbox) && final_name(native)==sandbox_root;
            CloseHandle(native);if(!equal){failure="profile resolver does not name the verified sandbox";return false;}
            profile_method=true;
        }
        failure="manifest digest";if(sha256(setup.manifest)!=setup.manifest_sha256)return false;
        std::ifstream manifest(std::filesystem::path(setup.manifest),std::ios::binary);std::string line;
        failure="manifest parse";if(!std::getline(manifest,line))return false;
        if(!line.empty() && line.back()=='\r')line.pop_back();
        if(line!="CDMP-R4-SAVE-1")return false;
        std::set<std::wstring,OrdinalLess> manifest_names;
        while(std::getline(manifest,line)) {
            if(!line.empty() && line.back()=='\r')line.pop_back();
            auto one=line.find('\t'),two=line.find('\t',one==std::string::npos?0:one+1);
            if(one!=64 || two==std::string::npos)return false;
            if(entries.size()>=16384 || two==one+1 ||
               line.substr(0,one).find_first_not_of("0123456789ABCDEF")!=std::string::npos ||
               line.substr(one+1,two-one-1).find_first_not_of("0123456789")!=std::string::npos)return false;
            Entry e{line.substr(0,one),std::stoull(line.substr(one+1,two-one-1)),widen(line.substr(two+1))};
            if(e.relative.empty() || e.relative.find(L':')!=std::wstring::npos || e.relative[0]==L'\\' ||
                e.relative.find(L"..")!=std::wstring::npos || e.relative.find(L'/')!=std::wstring::npos ||
                !manifest_names.insert(e.relative).second)return false;
            entries.push_back(std::move(e));
        }
        failure="manifest tree hashes/identities";
        if(entries.empty() || !(setup.source_may_change?collect_source_ids(real_root):verify_tree(real_root,entries,true,true)) ||
            !verify_tree(setup.backup,entries) || !verify_tree(sandbox_root,entries))return false;
        const auto log_path=log::current_path();
        if(!log_path.empty()) {
            HANDLE file=CreateFileW(log_path.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                nullptr,OPEN_EXISTING,0,nullptr);
            if(file==INVALID_HANDLE_VALUE)return false;
            const auto id=file_id(file);CloseHandle(file);
            if(real_ids.contains(id))return false;
            owned_log_ids.insert(id); // the logger intentionally has a dated-file/alias hard link
        }
        wchar_t drives[512]{};DWORD n=GetLogicalDriveStringsW(512,drives);if(!n || n>=512)return false;
        for(const wchar_t* drive=drives;*drive;drive+=wcslen(drive)+1){std::wstring dos(drive,2);wchar_t nt[32768]{};
            if(QueryDosDeviceW(dos.c_str(),nt,32768))volumes.emplace_back(nt,dos);}
        failure="pre-existing protected mappings";
        const auto query_region=reinterpret_cast<decltype(&QueryVirtualMemoryInformation)>(
            GetProcAddress(GetModuleHandleW(L"kernelbase.dll"),"QueryVirtualMemoryInformation"));
        if(!query_region)return false;
        SYSTEM_INFO system{};GetSystemInfo(&system);
        auto cursor=reinterpret_cast<uintptr_t>(system.lpMinimumApplicationAddress);
        const auto maximum=reinterpret_cast<uintptr_t>(system.lpMaximumApplicationAddress);
        while(cursor<maximum) {
            MEMORY_BASIC_INFORMATION region{};if(!VirtualQuery(reinterpret_cast<void*>(cursor),&region,sizeof(region)))return false;
            auto end=reinterpret_cast<uintptr_t>(region.BaseAddress)+region.RegionSize;if(end<=cursor)return false;
            if(region.State==MEM_COMMIT && (region.Type==MEM_MAPPED || region.Type==MEM_IMAGE)) {
                WIN32_MEMORY_REGION_INFORMATION detail{};
                if(!query_region(GetCurrentProcess(),region.BaseAddress,MemoryRegionInfo,&detail,sizeof(detail),nullptr))return false;
                if(detail.MappedPageFile){cursor=end;continue;}
                std::array<wchar_t,32768> name{};
                DWORD length=K32GetMappedFileNameW(GetCurrentProcess(),region.BaseAddress,name.data(),static_cast<DWORD>(name.size()));
                if(!length || length>=name.size())return false;
                auto path=canonical(std::wstring(name.data(),length),false);
                if(path.empty() || under(path,real_root))return false;
            }
            cursor=end;
        }
        failure="hook setup";if(!setup.profile_redirect && !hooks())return false;
        // Leave audit mode before exercising the actual redirection. Internal
        // verification may read the real tree but never writes through bypass.
    }catch(...){return false;}
    try {
        failure="redirected startup self-check";
        const auto leaf=L"\\.cdmp-r5-selfcheck-"+std::to_wstring(GetCurrentProcessId());
        const auto source_path=real_root+leaf;
        const auto path=(setup.profile_redirect?sandbox_root:real_root)+leaf;
        auto source_absent=[&]{Audit audit;return GetFileAttributesW(source_path.c_str())==INVALID_FILE_ATTRIBUTES && GetLastError()==ERROR_FILE_NOT_FOUND;};
        if(!source_absent())return false;
        HANDLE file=CreateFileW(path.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE)return false;
        const std::array<char,8> expected{'C','D','M','P','R','4','O','K'};std::array<char,8> actual{};DWORD n=0;
        bool ok=WriteFile(file,expected.data(),static_cast<DWORD>(expected.size()),&n,nullptr) && n==expected.size();
        ok=ok && SetFilePointer(file,0,nullptr,FILE_BEGIN)!=INVALID_SET_FILE_POINTER;
        ok=ok && ReadFile(file,actual.data(),static_cast<DWORD>(actual.size()),&n,nullptr) && actual==expected;
        auto where=final_name(file);CloseHandle(file);
        ok=ok && under(where,sandbox_root) && DeleteFileW(path.c_str());
        if(!ok || !source_absent() || (!setup.source_may_change && !verify_tree(real_root,entries,true)))return false;
        failure="verified";ready.store(true,std::memory_order_release);
        log::info("3663 guest isolation verified: path '{}' method {} file hooks {}",std::filesystem::path(sandbox_root).string(),
            setup.profile_redirect?"USERPROFILE native save-path override":"NT default-deny file layer",active.load());
        if(setup.source_may_change)log::info("3663 guest isolation: frozen backup/sandbox verified; concurrent host real-save changes allowed");
        return true;
    }catch(...){return false;}
}
bool install_from_environment() {
    const auto profile_before=env(L"USERPROFILE"),local_before=env(L"LOCALAPPDATA");
    const auto sandbox=env(L"CDMP_SAVE_SANDBOX"),backup=env(L"CDMP_SAVE_BACKUP"),
        manifest=env(L"CDMP_SAVE_MANIFEST"),hash=env(L"CDMP_SAVE_MANIFEST_SHA256");
    // Match the actual target resolver (SHGetFolderPathW, CSIDL_LOCAL_APPDATA),
    // not an overridable environment spelling that the game does not use.
    std::vector<wchar_t> native_local(MAX_PATH);
    if(SHGetFolderPathW(nullptr,CSIDL_LOCAL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,native_local.data())!=S_OK)return false;
    const auto method=env(L"CDMP_SAVE_ISOLATION");
    if(method!=L"nt"){failure="R7 requires NT save-directory isolation; profile override forbidden";return false;}
    const std::wstring local(native_local.data());
    if(local.empty() || sandbox.empty() || backup.empty() || manifest.empty() || hash.size()!=64)return false;
    std::string ascii;for(wchar_t c:hash){if(!((c>=L'0'&&c<=L'9')||(c>=L'A'&&c<=L'F')))return false;ascii+=static_cast<char>(c);}
    const auto source=local+L"\\Pearl Abyss\\CD\\save",expected=env(L"CDMP_REAL_SAVE_ROOT");
    if(expected.empty() || CompareStringOrdinal(source.c_str(),-1,expected.c_str(),-1,TRUE)!=CSTR_EQUAL){
        failure="normal native save root differs from the launcher's verified source";return false;}
    if(!install({source,backup,sandbox,manifest,ascii,env(L"CDMP_GUEST_WITH_HOST")==L"1",false}))return false;
    if(env(L"USERPROFILE")!=profile_before || env(L"LOCALAPPDATA")!=local_before){failure="profile environment changed";return false;}
    log::info("3663 R7 guest: USERPROFILE/LOCALAPPDATA unchanged; only save-directory paths redirected");
    return true;
}
bool verified(){return ready.load(std::memory_order_acquire);}
bool file_hooks_active(){return active.load(std::memory_order_acquire);}
std::string describe(){
    std::string result="verified="+std::to_string(verified())+" method="+(profile_method?"profile":"nt")+" file_hooks="+std::to_string(file_hooks_active())+
        " redirected="+std::to_string(redirected.load())+" refused="+std::to_string(refused.load());
    for(size_t i=0;i<refusal_sites.size();++i)if(const auto count=refusal_sites[i].load())result+=" line"+std::to_string(i)+"="+std::to_string(count);
    return result;
}
std::string failure_reason(){return failure;}
void report_refusals(){
    if(!file_hooks_active())return;
    log::info("3663 sandbox counters: {} named_refusals={} names_sampled={}/{}",
        describe(),named_refusals.load(),names_sampled.load(),max_name_samples);
}
#ifdef CDMP_SANDBOX_FIXTURE
void fixture_abort(){active.store(false);ready.store(false);}
#endif
}
