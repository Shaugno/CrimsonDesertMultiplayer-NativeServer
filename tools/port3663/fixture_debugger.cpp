// Owned fixture child only. No attach mode and no game execution path.
#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
#include <string>
int wmain(int argc,wchar_t** argv) {
    if(argc<3)return 2;
    std::wstring exe=argv[1];
    if(exe.find(L"cdmp_tests_file_sandbox3663.exe")==std::wstring::npos)return 3;
    std::wstring command=L"\""+exe+L"\"";
    for(int i=2;i<argc;++i)command+=L" \""+std::wstring(argv[i])+L"\"";
    STARTUPINFOW startup{sizeof(startup)};PROCESS_INFORMATION process{};
    if(!CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,TRUE,DEBUG_ONLY_THIS_PROCESS|CREATE_NO_WINDOW,
        nullptr,nullptr,&startup,&process))return 4;
    SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS);
    SymInitialize(process.hProcess,nullptr,FALSE);
    DWORD result=1;bool reported=false;DEBUG_EVENT event{};
    for(;;) {
        if(!WaitForDebugEvent(&event,30000)){TerminateProcess(process.hProcess,99);break;}
        DWORD disposition=DBG_CONTINUE;
        if(event.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT || event.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT) {
            const bool initial=event.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT;
            HANDLE file=initial?event.u.CreateProcessInfo.hFile:event.u.LoadDll.hFile;
            void* base=initial?event.u.CreateProcessInfo.lpBaseOfImage:event.u.LoadDll.lpBaseOfDll;
            wchar_t name[32768]{};GetFinalPathNameByHandleW(file,name,32768,0);
            SymLoadModuleExW(process.hProcess,file,*name?name:nullptr,nullptr,reinterpret_cast<DWORD64>(base),0,nullptr,0);
            if(file)CloseHandle(file);
        }
        if(event.dwDebugEventCode==EXCEPTION_DEBUG_EVENT) {
            auto& exception=event.u.Exception;
            if(exception.ExceptionRecord.ExceptionCode!=EXCEPTION_BREAKPOINT &&
               exception.ExceptionRecord.ExceptionCode!=EXCEPTION_SINGLE_STEP)disposition=DBG_EXCEPTION_NOT_HANDLED;
            if(exception.ExceptionRecord.ExceptionCode==EXCEPTION_ACCESS_VIOLATION && !reported) {
                reported=true;
                HANDLE thread=OpenThread(THREAD_GET_CONTEXT|THREAD_QUERY_INFORMATION,FALSE,event.dwThreadId);
                CONTEXT context{};context.ContextFlags=CONTEXT_FULL;GetThreadContext(thread,&context);
                std::printf("AV firstChance=%lu address=%p operation=%llu target=%llX RIP=%llX RSP=%llX\n",
                    exception.dwFirstChance,exception.ExceptionRecord.ExceptionAddress,
                    static_cast<unsigned long long>(exception.ExceptionRecord.ExceptionInformation[0]),
                    static_cast<unsigned long long>(exception.ExceptionRecord.ExceptionInformation[1]),context.Rip,context.Rsp);
                STACKFRAME64 frame{};frame.AddrPC={context.Rip,0,AddrModeFlat};
                frame.AddrFrame={context.Rbp,0,AddrModeFlat};frame.AddrStack={context.Rsp,0,AddrModeFlat};
                for(int i=0;i<32;++i) {
                    alignas(SYMBOL_INFO) char memory[sizeof(SYMBOL_INFO)+1024]{};
                    auto* symbol=reinterpret_cast<SYMBOL_INFO*>(memory);symbol->SizeOfStruct=sizeof(SYMBOL_INFO);symbol->MaxNameLen=1024;
                    DWORD64 displacement=0;DWORD line_displacement=0;IMAGEHLP_LINE64 line{sizeof(line)};
                    bool found=SymFromAddr(process.hProcess,frame.AddrPC.Offset,&displacement,symbol)!=FALSE;
                    bool source=SymGetLineFromAddr64(process.hProcess,frame.AddrPC.Offset,&line_displacement,&line)!=FALSE;
                    std::printf("#%d %llX %s+%llX %s:%lu\n",i,frame.AddrPC.Offset,found?symbol->Name:"?",displacement,
                        source?line.FileName:"?",source?line.LineNumber:0);
                    if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process.hProcess,thread,&frame,&context,nullptr,
                        SymFunctionTableAccess64,SymGetModuleBase64,nullptr))break;
                }
                CloseHandle(thread);std::fflush(stdout);TerminateProcess(process.hProcess,101);
            }
        }
        if(event.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT){result=event.u.ExitProcess.dwExitCode;ContinueDebugEvent(event.dwProcessId,event.dwThreadId,DBG_CONTINUE);break;}
        ContinueDebugEvent(event.dwProcessId,event.dwThreadId,disposition);
    }
    SymCleanup(process.hProcess);CloseHandle(process.hThread);CloseHandle(process.hProcess);
    return static_cast<int>(result);
}
