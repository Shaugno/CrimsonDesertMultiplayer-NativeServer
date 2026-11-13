#define PKT3576_LIVE_FIXTURE
#include <windows.h>
#include <atomic>
#include <new>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include "pkt3576/live_runner.h"
#include "pkt3576/live_api.h"
#include "../tests/pkt3576_capture_targets.h"
std::atomic<bool> fail_next{false};
std::atomic<size_t> fail_bytes{0};
void* operator new(size_t size){if(fail_next.exchange(false)){fail_bytes.store(size);throw std::bad_alloc();}if(void* p=std::malloc(size))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete(void* p,size_t)noexcept{std::free(p);}
int main(int argc,char**){
 wchar_t temp[MAX_PATH]{};GetTempPathW(MAX_PATH,temp);
 const auto root=std::wstring(temp)+L"codex-live-review-"+std::to_wstring(GetCurrentProcessId());
 auto send=reinterpret_cast<void*>(&pkt3576_dummy_send);auto recv=reinterpret_cast<void*>(&pkt3576_dummy_recv);
 if(argc>1){
  const auto file=root+L"-file";HANDLE h=CreateFileW(file.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(h==INVALID_HANDLE_VALUE)return 20;CloseHandle(h);
  const int first=pkt3576::live::fixture_start(file.c_str(),send,recv,8,8);
  const int retry=pkt3576::live::fixture_start(root.c_str(),send,recv,8,8);
  unsigned attempts=99;pkt3576::live::status(nullptr,nullptr,nullptr,nullptr,nullptr,&attempts);
  std::printf("bad_path_code=%d retry_good_path_code=%d capture_attempts=%u\n",first,retry,attempts);
  if(retry==kPkt3576LiveStarted)pkt3576::live::stop();DeleteFileW(file.c_str());return first==kPkt3576LiveBadLog&&retry==kPkt3576LiveBadLog?95:0;
 }
 unsigned char before[16]{};std::memcpy(before,recv,16);
 const int started=pkt3576::live::fixture_start(root.c_str(),send,recv,8,8);if(started!=kPkt3576LiveStarted){std::printf("start=%d\n",started);return 21;}
 fail_next.store(true);bool escaped=false;int stopped=-1;
 try{stopped=pkt3576::live::stop();}catch(const std::bad_alloc&){escaped=true;}
 fail_next.store(false);const bool patched=std::memcmp(before,recv,16)!=0;
 const int retry_stop=pkt3576::live::stop();int retained=-1;pkt3576::live::status(nullptr,nullptr,&retained,nullptr,nullptr,nullptr);
 std::printf("stop_exception_escaped=%d allocation_size=%zu receive_still_patched=%d next_stop=%d retained=%d\n",escaped,fail_bytes.load(),patched,retry_stop,retained);std::fflush(stdout);
 const bool failed=escaped&&patched&&retry_stop==kPkt3576LiveIdle&&retained==0;ExitProcess(failed?94:0);
}
