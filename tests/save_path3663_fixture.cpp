// Read-only Windows resolver calls matching the target's file-only evidence.
#include <windows.h>
#include <shlobj.h>
#include <cstdio>
#include <string>
int wmain(int argc,wchar_t** argv){
    wchar_t path[MAX_PATH]{};PWSTR known=nullptr;
    if(SHGetFolderPathW(nullptr,CSIDL_LOCAL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,path)!=S_OK)return 1;
    if(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&known)!=S_OK || !known)return 2;
    bool same=CompareStringOrdinal(path,-1,known,-1,TRUE)==CSTR_EQUAL;
    CoTaskMemFree(known);if(!same)return 3;
    // Parent supplies the baseline obtained in a different, fresh process.
    const int count=WideCharToMultiByte(CP_UTF8,0,path,-1,nullptr,0,nullptr,nullptr);
    if(count<1)return 5;std::string utf8(static_cast<size_t>(count),'\0');
    if(!WideCharToMultiByte(CP_UTF8,0,path,-1,utf8.data(),count,nullptr,nullptr))return 6;
    std::printf("RESOLVED=%s\n",utf8.c_str());
    if(argc==2 && CompareStringOrdinal(path,-1,argv[1],-1,TRUE)!=CSTR_EQUAL)return 4;
    return 0;
}
