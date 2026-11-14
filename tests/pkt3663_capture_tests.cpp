#include "pkt3663/capture.h"

#include "pkt3663_capture_targets.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include <windows.h>

namespace {

int g_failures = 0;

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);         \
      ++g_failures;                                                            \
    }                                                                          \
  } while (false)

struct Owned {
  pkt3663::Capture *raw = nullptr;
  ~Owned() {
    if (raw != nullptr && !pkt3663::Capture::release_owner(raw))
      raw = nullptr;
  }
  pkt3663::Capture *operator->() const { return raw; }
};

std::wstring temp_path(const wchar_t *name) {
  wchar_t directory[MAX_PATH] = {};
  if (GetTempPathW(MAX_PATH, directory) == 0)
    return {};
  return std::wstring(directory) + name;
}

std::string read_file(const std::wstring &path) {
  FILE *file = nullptr;
  if (_wfopen_s(&file, path.c_str(), L"rb") != 0 || file == nullptr)
    return {};
  std::string text;
  char buffer[1024] = {};
  size_t count = 0;
  while ((count = std::fread(buffer, 1, sizeof buffer, file)) > 0)
    text.append(buffer, count);
  std::fclose(file);
  return text;
}

void entry_bytes(const void *function, uint8_t out[16]) {
  std::memcpy(out, function, 16);
}

bool same_bytes(const uint8_t *left, const uint8_t *right) {
  return std::memcmp(left, right, 16) == 0;
}

template <typename Predicate>
bool wait_until(Predicate predicate, uint32_t timeout_ms = 3000) {
  const ULONGLONG deadline = GetTickCount64() + timeout_ms;
  while (!predicate()) {
    if (GetTickCount64() >= deadline)
      return false;
    SwitchToThread();
  }
  return true;
}

struct CheckPlan {
  std::atomic<int> calls{0};
  std::atomic<int> order{0};
  bool receive = true;
  bool send = true;
  bool complete = true;
  void (*during_send)(void *) = nullptr;
  void *during_send_user = nullptr;
  void (*during_complete)(void *) = nullptr;
  void *during_complete_user = nullptr;
};

void check_install_evidence(const pkt3663::InstallEvidence *evidence) {
  CHECK(evidence != nullptr);
  if (evidence == nullptr)
    return;
  CHECK(evidence->target_patch != nullptr);
  CHECK(evidence->original_bytes != nullptr);
  CHECK(evidence->target_patch_size == evidence->original_size);
  CHECK(evidence->target_patch_size >= 5 && evidence->target_patch_size <= 64);
  if (evidence->target_patch == nullptr || evidence->target_patch_size < 5)
    return;
  if (evidence->target_patch[0] == 0xE9) {
    for (uint8_t index = 5; index < evidence->target_patch_size; ++index) {
      CHECK(evidence->target_patch[index] == 0x90);
    }
  } else {
    CHECK(evidence->target_patch[0] == 0xFF);
    CHECK(evidence->target_patch[1] == 0x25);
    CHECK(evidence->target_patch_size >= 14);
    for (uint8_t index = 14; index < evidence->target_patch_size; ++index) {
      CHECK(evidence->target_patch[index] == 0x90);
    }
  }
}

bool install_check(pkt3663::InstallSite site,
                   const pkt3663::InstallEvidence *evidence,
                   void *user) noexcept {
  auto *plan = static_cast<CheckPlan *>(user);
  if (plan == nullptr)
    return false;
  const int call = plan->calls.fetch_add(1, std::memory_order_relaxed) + 1;
  if (site == pkt3663::InstallSite::Receive) {
    CHECK(evidence == nullptr);
    if (call == 1)
      plan->order.fetch_or(1, std::memory_order_relaxed);
    return plan->receive;
  }
  check_install_evidence(evidence);
  if (site == pkt3663::InstallSite::Send) {
    if (call == 2)
      plan->order.fetch_or(2, std::memory_order_relaxed);
    if (plan->during_send != nullptr)
      plan->during_send(plan->during_send_user);
    return plan->send;
  }
  CHECK(site == pkt3663::InstallSite::Complete);
  if (call == 3)
    plan->order.fetch_or(4, std::memory_order_relaxed);
  if (plan->during_complete != nullptr)
    plan->during_complete(plan->during_complete_user);
  return plan->complete;
}

pkt3663::Capture *make_capture(uint32_t queue, uint32_t output,
                               const std::wstring &path) {
  pkt3663::CaptureLimits limits;
  limits.queue_capacity = queue;
  limits.output_records = output;
  return pkt3663::Capture::create(limits,
                                  path.empty() ? nullptr : path.c_str());
}

bool admit(pkt3663::Capture *capture) {
  LARGE_INTEGER now{};
  if (capture == nullptr || !QueryPerformanceCounter(&now) || now.QuadPart <= 0)
    return false;
  return capture->publish_ready(static_cast<uint64_t>(now.QuadPart));
}

void test_refusal_and_install_checks() {
  const std::wstring path = temp_path(L"pkt3663-capture-check.log");
  DeleteFileW(path.c_str());
  uint8_t send_before[16] = {};
  uint8_t recv_before[16] = {};
  entry_bytes(reinterpret_cast<void *>(&pkt3663_dummy_send), send_before);
  entry_bytes(reinterpret_cast<void *>(&pkt3663_dummy_receive), recv_before);

  Owned small{make_capture(1, 2, path)};
  CHECK(small.raw != nullptr);
  CHECK(small->start(GetModuleHandleW(nullptr),
                     reinterpret_cast<void *>(&pkt3663_dummy_send),
                     reinterpret_cast<void *>(&pkt3663_dummy_receive)) ==
        pkt3663::Capture::StartResult::Refused);

  CheckPlan receive_reject;
  receive_reject.receive = false;
  Owned first{make_capture(8, 4, path)};
  CHECK(first.raw != nullptr);
  CHECK(first->start(GetModuleHandleW(nullptr),
                     reinterpret_cast<void *>(&pkt3663_dummy_send),
                     reinterpret_cast<void *>(&pkt3663_dummy_receive),
                     &install_check, &receive_reject) ==
        pkt3663::Capture::StartResult::Refused);
  CHECK(receive_reject.calls.load() == 1);
  CHECK(receive_reject.order.load() == 1);

  CheckPlan send_reject;
  send_reject.send = false;
  Owned second{make_capture(8, 4, path)};
  CHECK(second.raw != nullptr);
  CHECK(second->start(GetModuleHandleW(nullptr),
                      reinterpret_cast<void *>(&pkt3663_dummy_send),
                      reinterpret_cast<void *>(&pkt3663_dummy_receive),
                      &install_check, &send_reject) ==
        pkt3663::Capture::StartResult::RolledBack);
  CHECK(send_reject.calls.load() == 2);
  CHECK(send_reject.order.load() == 3);
  CHECK(!second->queue_allocated());
  CHECK(!second->accepting());

  CheckPlan complete_reject;
  complete_reject.complete = false;
  Owned third{make_capture(8, 4, path)};
  CHECK(third.raw != nullptr);
  CHECK(third->start(GetModuleHandleW(nullptr),
                     reinterpret_cast<void *>(&pkt3663_dummy_send),
                     reinterpret_cast<void *>(&pkt3663_dummy_receive),
                     &install_check, &complete_reject) ==
        pkt3663::Capture::StartResult::RolledBack);
  CHECK(complete_reject.calls.load() == 3);
  CHECK(complete_reject.order.load() == 7);
  CHECK(!third->queue_allocated());
  CHECK(!third->accepting());

  uint8_t send_after[16] = {};
  uint8_t recv_after[16] = {};
  entry_bytes(reinterpret_cast<void *>(&pkt3663_dummy_send), send_after);
  entry_bytes(reinterpret_cast<void *>(&pkt3663_dummy_receive), recv_after);
  CHECK(same_bytes(send_before, send_after));
  CHECK(same_bytes(recv_before, recv_after));
}

void test_raw_records_and_log_metadata() {
  uint8_t bytes[12] = {0xEF, 0xBE, 0x7A, 0x99, 0x88, 1, 2, 3, 4, 5, 6, 7};
  uint8_t original[sizeof bytes] = {};
  std::memcpy(original, bytes, sizeof bytes);
  const std::wstring path = temp_path(L"pkt3663-capture-raw.log");
  DeleteFileW(path.c_str());

  Owned capture{make_capture(16, 8, path)};
  CHECK(capture.raw != nullptr);
  capture->set_phase(static_cast<uint8_t>(pkt3663::Phase::Stand));
  CheckPlan checks;
  CHECK(capture->start(GetModuleHandleW(nullptr),
                       reinterpret_cast<void *>(&pkt3663_dummy_send),
                       reinterpret_cast<void *>(&pkt3663_dummy_receive),
                       &install_check,
                       &checks) == pkt3663::Capture::StartResult::Started);
  CHECK(admit(capture.raw));
  CHECK(checks.calls.load() == 3);
  CHECK(checks.order.load() == 7);
  CHECK(pkt3663_dummy_send(0x1111111111111111ull, 0x2222222222222222ull,
                           0x3333333333333333ull, bytes,
                           static_cast<uint16_t>(sizeof bytes), 0x5A,
                           0xBEEF) == pkt3663fix::kSendReturn);
  CHECK(pkt3663_dummy_receive(0xAAAAAAAAAAAAAAAAull, 2, 3, bytes,
                              static_cast<uint16_t>(sizeof bytes), 4,
                              5) == pkt3663fix::kReceiveReturn);
  CHECK(wait_until([&] { return capture->stored() >= 2; }));
  CHECK(capture->stop() == pkt3663::Capture::StopResult::Removed);
  CHECK(capture->stored() == 2);
  CHECK(capture->dropped() == 0);
  CHECK(capture->unread() == 0);
  CHECK(capture->truncated() == 0);
  CHECK(capture->omitted() == 0);
  CHECK(std::memcmp(bytes, original, sizeof bytes) == 0);

  pkt3663::Record send;
  pkt3663::Record receive;
  CHECK(capture->copy_record(0, send));
  CHECK(capture->copy_record(1, receive));
  CHECK(send.direction == pkt3663::Direction::Send);
  CHECK(send.rcx == 0x1111111111111111ull);
  CHECK(send.claimed_length == sizeof bytes);
  CHECK(send.copied_length == sizeof bytes);
  CHECK(send.has_raw_word0);
  CHECK(send.raw_word0 == 0xBEEF);
  CHECK(send.status == pkt3663::RawStatus::Complete);
  CHECK(send.stack_u8_30 == 0x5A);
  CHECK(send.stack_u16_38 == 0xBEEF);
  CHECK(send.phase == static_cast<uint8_t>(pkt3663::Phase::Stand));
  CHECK(!send.receive_pre_transform_possible);
  CHECK(receive.direction == pkt3663::Direction::Receive);
  CHECK(receive.has_raw_word0);
  CHECK(receive.raw_word0 == 0xBEEF);
  CHECK(receive.receive_pre_transform_possible);
  CHECK(!receive.has_stack_u8_30);
  CHECK(!receive.has_stack_u16_38);

  const std::string log = read_file(path);
  CHECK(log.find("capture_mode=raw-all") != std::string::npos);
  CHECK(log.find("classification=unclassified") != std::string::npos);
  CHECK(log.find("layout=unresolved") != std::string::npos);
  CHECK(log.find("receive_pre_transform_possible=1") != std::string::npos);
  CHECK(log.find("adopted=0") != std::string::npos);
  CHECK(log.find("allowlist_count=0") != std::string::npos);
  CHECK(log.find("copy_cap=4096") != std::string::npos);
  CHECK(log.find("raw_word0=BEEF") != std::string::npos);
  CHECK(log.find("opcode=") == std::string::npos);
}

void test_cap_and_queue_pressure() {
  std::vector<uint8_t> bytes(pkt3663::kCopyCap + 1, 0xA5);
  bytes[0] = 0x34;
  bytes[1] = 0x12;
  const std::wstring path = temp_path(L"pkt3663-capture-bound.log");
  DeleteFileW(path.c_str());
  Owned capture{make_capture(2, 2, path)};
  CHECK(capture.raw != nullptr);
  CHECK(capture->start(GetModuleHandleW(nullptr),
                       reinterpret_cast<void *>(&pkt3663_dummy_send),
                       reinterpret_cast<void *>(&pkt3663_dummy_receive)) ==
        pkt3663::Capture::StartResult::Started);
  CHECK(admit(capture.raw));
  capture->set_writer_blocked(true);
  for (int index = 0; index < 20; ++index) {
    CHECK(pkt3663_dummy_send(static_cast<uint64_t>(index), 0, 0, bytes.data(),
                             static_cast<uint16_t>(bytes.size()), 1,
                             2) == pkt3663fix::kSendReturn);
  }
  CHECK(capture->stop() == pkt3663::Capture::StopResult::Removed);
  CHECK(capture->stored() == 2);
  CHECK(capture->dropped() == 18);
  CHECK(capture->truncated() == 20);
  pkt3663::Record sample;
  CHECK(capture->copy_record(0, sample));
  CHECK(sample.status == pkt3663::RawStatus::Truncated);
  CHECK(sample.claimed_length == pkt3663::kCopyCap + 1);
  CHECK(sample.copied_length == pkt3663::kCopyCap);
  CHECK(sample.bytes[0] == 0x34);
  CHECK(sample.bytes[1] == 0x12);

  const std::wstring output_path =
      temp_path(L"pkt3663-capture-output-bound.log");
  DeleteFileW(output_path.c_str());
  Owned output{make_capture(32, 2, output_path)};
  CHECK(output.raw != nullptr);
  CHECK(output->start(GetModuleHandleW(nullptr),
                      reinterpret_cast<void *>(&pkt3663_dummy_send),
                      reinterpret_cast<void *>(&pkt3663_dummy_receive)) ==
        pkt3663::Capture::StartResult::Started);
  CHECK(admit(output.raw));
  for (int index = 0; index < 10; ++index) {
    CHECK(pkt3663_dummy_send(static_cast<uint64_t>(index), 0, 0, bytes.data(),
                             8, 1, 2) == pkt3663fix::kSendReturn);
  }
  CHECK(output->stop() == pkt3663::Capture::StopResult::Removed);
  CHECK(output->stored() == 2);
  CHECK(output->omitted() == 8);
  CHECK(output->dropped() == 0);
}

void test_pre_ready_flood_and_cutoff() {
  uint8_t frame[4] = {0x11, 0x22, 0x33, 0x44};
  const std::wstring path = temp_path(L"pkt3663-capture-preready.log");
  DeleteFileW(path.c_str());
  Owned capture{make_capture(32, 4, path)};
  CHECK(capture.raw != nullptr);
  CHECK(capture->start(GetModuleHandleW(nullptr),
                       reinterpret_cast<void *>(&pkt3663_dummy_send),
                       reinterpret_cast<void *>(&pkt3663_dummy_receive)) ==
        pkt3663::Capture::StartResult::Started);
  for (int index = 0; index < 40; ++index) {
    CHECK(pkt3663_dummy_receive(static_cast<uint64_t>(index), 0, 0, frame, 4, 0,
                                0) == pkt3663fix::kReceiveReturn);
  }
  CHECK(capture->stored() == 0);
  CHECK(capture->omitted() == 0);
  CHECK(capture->dropped() == 0);
  CHECK(capture->pre_ready() == 40);
  CHECK(!capture->publish_ready(0));

  capture->set_writer_blocked(true);
  CHECK(capture->publish_ready(1000000));
  CHECK(!capture->publish_ready(1000001));
  pkt3663::Record early{};
  early.direction = pkt3663::Direction::Receive;
  early.qpc_ticks = 999999;
  early.claimed_length = 4;
  early.copied_length = 4;
  early.phase = static_cast<uint8_t>(pkt3663::Phase::Stand);
  pkt3663::Record exact = early;
  exact.qpc_ticks = 1000000;
  exact.direction = pkt3663::Direction::Send;
  CHECK(capture->enqueue_for_test(early));
  CHECK(capture->enqueue_for_test(exact));
  for (int index = 0; index < 6; ++index) {
    CHECK(pkt3663_dummy_send(static_cast<uint64_t>(index), 0, 0, frame, 4, 1,
                             2) == pkt3663fix::kSendReturn);
  }
  capture->set_writer_blocked(false);
  CHECK(wait_until([&] { return capture->stored() == 4; }));
  CHECK(capture->stop() == pkt3663::Capture::StopResult::Removed);
  CHECK(capture->stored() == 4);
  CHECK(capture->omitted() == 3);
  CHECK(capture->dropped() == 0);
  CHECK(capture->pre_ready() >= 41);
  int sends = 0;
  int receives = 0;
  for (uint32_t index = 0; index < capture->stored(); ++index) {
    pkt3663::Record row{};
    CHECK(capture->copy_record(index, row));
    CHECK(row.qpc_ticks >= 1000000);
    if (row.direction == pkt3663::Direction::Send)
      ++sends;
    if (row.direction == pkt3663::Direction::Receive)
      ++receives;
  }
  CHECK(sends >= 1);
  CHECK(receives == 0);

  const std::wstring race_path = temp_path(L"pkt3663-capture-stop-flood.log");
  DeleteFileW(race_path.c_str());
  Owned racing{make_capture(32, 8, race_path)};
  CHECK(racing.raw != nullptr);
  CHECK(racing->start(GetModuleHandleW(nullptr),
                      reinterpret_cast<void *>(&pkt3663_dummy_send),
                      reinterpret_cast<void *>(&pkt3663_dummy_receive)) ==
        pkt3663::Capture::StartResult::Started);
  std::atomic<bool> run{true};
  std::thread producer([&] {
    while (run.load(std::memory_order_acquire)) {
      pkt3663_dummy_receive(1, 2, 3, frame, 4, 0, 0);
    }
  });
  CHECK(wait_until([&] { return racing->pre_ready() > 0; }));
  CHECK(racing->stop() == pkt3663::Capture::StopResult::Removed);
  run.store(false, std::memory_order_release);
  producer.join();
  CHECK(racing->stored() == 0);
  CHECK(racing->omitted() == 0);
  CHECK(racing->pre_ready() > 0);
  CHECK(!racing->queue_allocated());
  CHECK(!racing->publish_ready(1000000));
}

void test_concurrent_teardown_and_partial_install() {
  uint8_t bytes[8] = {0x78, 0x56, 0, 1, 2, 3, 4, 5};
  const std::wstring path = temp_path(L"pkt3663-capture-race.log");
  DeleteFileW(path.c_str());
  Owned capture{make_capture(64, 16, path)};
  CHECK(capture.raw != nullptr);
  CHECK(capture->start(GetModuleHandleW(nullptr),
                       reinterpret_cast<void *>(&pkt3663_dummy_send),
                       reinterpret_cast<void *>(&pkt3663_dummy_receive)) ==
        pkt3663::Capture::StartResult::Started);
  CHECK(admit(capture.raw));
  std::atomic<bool> run{true};
  std::atomic<int> bad{0};
  std::thread producer([&] {
    while (run.load(std::memory_order_acquire)) {
      if (pkt3663_dummy_send(1, 2, 3, bytes, 8, 4, 5) !=
          pkt3663fix::kSendReturn) {
        bad.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });
  CHECK(wait_until([&] { return capture->stored() > 0; }));
  CHECK(capture->stop() == pkt3663::Capture::StopResult::Removed);
  run.store(false, std::memory_order_release);
  producer.join();
  CHECK(bad.load() == 0);
  CHECK(!capture->queue_allocated());
  CHECK(!capture->accepting());

  uint8_t recv_before[16] = {};
  uint8_t recv_after[16] = {};
  entry_bytes(reinterpret_cast<void *>(&pkt3663_dummy_receive), recv_before);
  Owned partial{make_capture(8, 2, path)};
  CHECK(partial.raw != nullptr);
  CHECK(partial->start(GetModuleHandleW(nullptr), pkt3663_bad_entry,
                       reinterpret_cast<void *>(&pkt3663_dummy_receive)) ==
        pkt3663::Capture::StartResult::RolledBack);
  entry_bytes(reinterpret_cast<void *>(&pkt3663_dummy_receive), recv_after);
  CHECK(same_bytes(recv_before, recv_after));
  CHECK(!partial->queue_allocated());
}

struct PatchDamage {
  void *target = nullptr;
  uint8_t original[16] = {};
  std::atomic<bool> ran{false};
};

bool write_code(void *target, const uint8_t *bytes, size_t size) {
  DWORD old_protection = 0;
  if (VirtualProtect(target, size, PAGE_EXECUTE_READWRITE, &old_protection) ==
      FALSE)
    return false;
  std::memcpy(target, bytes, size);
  FlushInstructionCache(GetCurrentProcess(), target, size);
  DWORD restored = 0;
  const bool ok =
      VirtualProtect(target, size, old_protection, &restored) != FALSE;
  return ok;
}

void damage_receive_patch(void *user) {
  auto *damage = static_cast<PatchDamage *>(user);
  if (damage == nullptr || damage->target == nullptr)
    return;
  uint8_t nop = 0x90;
  if (write_code(damage->target, &nop, sizeof nop))
    damage->ran.store(true, std::memory_order_release);
}

void test_pre_send_changed_receive_patch_retains() {
  const std::wstring path = temp_path(L"pkt3663-capture-retained.log");
  DeleteFileW(path.c_str());
  pkt3663::Capture *capture = make_capture(8, 2, path);
  CHECK(capture != nullptr);
  if (capture == nullptr)
    return;

  PatchDamage damage;
  damage.target = reinterpret_cast<void *>(&pkt3663_dummy_receive);
  entry_bytes(damage.target, damage.original);
  pkt3663::Capture::set_pre_send_hook(&damage_receive_patch, &damage);
  const pkt3663::Capture::StartResult result = capture->start(
      GetModuleHandleW(nullptr), reinterpret_cast<void *>(&pkt3663_dummy_send),
      damage.target);
  pkt3663::Capture::set_pre_send_hook(nullptr, nullptr);
  CHECK(damage.ran.load(std::memory_order_acquire));
  CHECK(result == pkt3663::Capture::StartResult::Retained);
  CHECK(!pkt3663::Capture::release_owner(capture));
  CHECK(capture != nullptr);

  // The retained object and SafetyHook allocations intentionally leak until
  // process exit. Restore only the dummy target bytes so no later runtime
  // teardown can execute the deliberately damaged test entry.
  CHECK(write_code(damage.target, damage.original, sizeof damage.original));
}

void test_rejected_send_check_reproves_receive_ownership() {
  const std::wstring path =
      temp_path(L"pkt3663-capture-send-reject-retained.log");
  DeleteFileW(path.c_str());
  pkt3663::Capture *capture = make_capture(8, 2, path);
  CHECK(capture != nullptr);
  if (capture == nullptr)
    return;

  PatchDamage damage;
  damage.target = reinterpret_cast<void *>(&pkt3663_dummy_receive);
  entry_bytes(damage.target, damage.original);
  CheckPlan checks;
  checks.send = false;
  checks.during_send = &damage_receive_patch;
  checks.during_send_user = &damage;
  const pkt3663::Capture::StartResult result = capture->start(
      GetModuleHandleW(nullptr), reinterpret_cast<void *>(&pkt3663_dummy_send),
      damage.target, &install_check, &checks);
  CHECK(damage.ran.load(std::memory_order_acquire));
  CHECK(checks.calls.load() == 2);
  CHECK(checks.order.load() == 3);
  CHECK(result == pkt3663::Capture::StartResult::Retained);
  CHECK(!pkt3663::Capture::release_owner(capture));
  CHECK(capture != nullptr);
  CHECK(write_code(damage.target, damage.original, sizeof damage.original));
}

void test_stop_reproves_both_hook_slots() {
  const std::wstring path = temp_path(L"pkt3663-capture-stop-retained.log");
  DeleteFileW(path.c_str());
  pkt3663::Capture *capture = make_capture(8, 2, path);
  CHECK(capture != nullptr);
  if (capture == nullptr)
    return;

  PatchDamage damage;
  damage.target = reinterpret_cast<void *>(&pkt3663_dummy_send);
  entry_bytes(damage.target, damage.original);
  void *receive = reinterpret_cast<void *>(&pkt3663_dummy_receive);
  uint8_t receive_original[16] = {};
  entry_bytes(receive, receive_original);
  CHECK(capture->start(GetModuleHandleW(nullptr), damage.target, receive) ==
        pkt3663::Capture::StartResult::Started);
  damage_receive_patch(&damage);
  CHECK(damage.ran.load(std::memory_order_acquire));
  CHECK(capture->stop() == pkt3663::Capture::StopResult::Retained);
  uint8_t receive_after[16] = {};
  entry_bytes(receive, receive_after);
  CHECK(!same_bytes(receive_original, receive_after));
  CHECK(!pkt3663::Capture::release_owner(capture));
  CHECK(capture != nullptr);
  CHECK(write_code(damage.target, damage.original, sizeof damage.original));
  CHECK(write_code(receive, receive_original, sizeof receive_original));
}

void test_complete_check_reproves_both_hook_slots() {
  const std::wstring path = temp_path(L"pkt3663-capture-complete-retained.log");
  DeleteFileW(path.c_str());
  pkt3663::Capture *capture = make_capture(8, 2, path);
  CHECK(capture != nullptr);
  if (capture == nullptr)
    return;

  PatchDamage damage;
  damage.target = reinterpret_cast<void *>(&pkt3663_dummy_send);
  entry_bytes(damage.target, damage.original);
  void *receive = reinterpret_cast<void *>(&pkt3663_dummy_receive);
  uint8_t receive_original[16] = {};
  entry_bytes(receive, receive_original);
  CheckPlan checks;
  checks.complete = false;
  checks.during_complete = &damage_receive_patch;
  checks.during_complete_user = &damage;
  const pkt3663::Capture::StartResult result =
      capture->start(GetModuleHandleW(nullptr), damage.target, receive,
                     &install_check, &checks);
  CHECK(damage.ran.load(std::memory_order_acquire));
  CHECK(checks.calls.load() == 3);
  CHECK(checks.order.load() == 7);
  CHECK(result == pkt3663::Capture::StartResult::Retained);
  uint8_t receive_after[16] = {};
  entry_bytes(receive, receive_after);
  CHECK(!same_bytes(receive_original, receive_after));
  CHECK(!pkt3663::Capture::release_owner(capture));
  CHECK(capture != nullptr);
  CHECK(write_code(damage.target, damage.original, sizeof damage.original));
  CHECK(write_code(receive, receive_original, sizeof receive_original));
}

bool run_child(const wchar_t *mode) {
  wchar_t executable[1024] = {};
  const DWORD length = GetModuleFileNameW(nullptr, executable, 1024);
  if (length == 0 || length >= 1024)
    return false;
  std::wstring command = L"\"" + std::wstring(executable) + L"\" " + mode;
  std::vector<wchar_t> mutable_command(command.begin(), command.end());
  mutable_command.push_back(L'\0');
  STARTUPINFOW startup{};
  startup.cb = sizeof startup;
  PROCESS_INFORMATION process{};
  if (!CreateProcessW(nullptr, mutable_command.data(), nullptr, nullptr, FALSE,
                      0, nullptr, nullptr, &startup, &process)) {
    return false;
  }
  const DWORD waited = WaitForSingleObject(process.hProcess, 10000);
  DWORD exit_code = 1;
  const bool exited = waited == WAIT_OBJECT_0 &&
                      GetExitCodeProcess(process.hProcess, &exit_code) != FALSE;
  CloseHandle(process.hThread);
  CloseHandle(process.hProcess);
  return exited && exit_code == 0;
}

int finish() {
  if (g_failures != 0) {
    std::printf("pkt3663 capture tests failed: %d\n", g_failures);
    return 1;
  }
  std::puts("pkt3663 capture tests ok");
  return 0;
}

} // namespace

int main(int argc, char **argv) {
  if (argc == 2 && std::strcmp(argv[1], "--pre-send-damage") == 0) {
    test_pre_send_changed_receive_patch_retains();
    return finish();
  }
  if (argc == 2 && std::strcmp(argv[1], "--send-check-damage") == 0) {
    test_rejected_send_check_reproves_receive_ownership();
    return finish();
  }
  if (argc == 2 && std::strcmp(argv[1], "--stop-damage") == 0) {
    test_stop_reproves_both_hook_slots();
    return finish();
  }
  if (argc == 2 && std::strcmp(argv[1], "--complete-check-damage") == 0) {
    test_complete_check_reproves_both_hook_slots();
    return finish();
  }
  test_refusal_and_install_checks();
  test_raw_records_and_log_metadata();
  test_cap_and_queue_pressure();
  test_pre_ready_flood_and_cutoff();
  test_concurrent_teardown_and_partial_install();
  CHECK(run_child(L"--pre-send-damage"));
  CHECK(run_child(L"--send-check-damage"));
  CHECK(run_child(L"--stop-damage"));
  CHECK(run_child(L"--complete-check-damage"));
  return finish();
}
