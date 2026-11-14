#pragma once
// Strict mapped-image gate for the experimental 3663 passive collector.
// A match permits only the separate diagnostic runner to consider installing
// its passive hooks. It does not open the ASI compatibility guard, adopt an
// RVA, or prove a packet payload layout.

#include <cstddef>
#include <cstdint>

namespace pkt3663 {

enum class IdentityKind {
  Match,
  Unreadable,
  WrongName,
  WrongTimestamp,
  WrongBase,
  WrongSize,
  MalformedImage,
  MissingFunctionTable,
  NotExecutable,
  FunctionRangeMismatch,
  AnchorMismatch,
  AmbiguousAnchor,
  CallTargetMismatch,
};

struct Identity {
  IdentityKind kind = IdentityKind::Unreadable;
  uintptr_t mapped_base = 0;
  uint32_t timestamp = 0;
  uint64_t preferred_base = 0;
  uint32_t size_of_image = 0;
  bool timestamp_known = false;
  bool preferred_base_known = false;
  bool size_known = false;
  bool rejected_2976 = false;
  bool rejected_3576 = false;
  bool sections_ok = false;
  bool function_table_ok = false;
  bool pump_range_ok = false;
  bool send_range_ok = false;
  bool receive_range_ok = false;
  bool pump_anchors_ok = false;
  bool send_anchors_ok = false;
  bool receive_anchors_ok = false;
  bool anchors_unique = false;
  bool role_calls_ok = false;
  uint32_t failed_anchor_rva = 0;
};

// Copies from a mapped image. The callback must return false instead of
// throwing when a range cannot be read.
using ImageRead = bool (*)(uintptr_t address, void *destination, size_t size,
                           void *user);

// `exe_name` is a file name, not a path. Identity fields are checked before
// any 3663 candidate RVA is read. The mapped image is then independently
// checked for section layout, pdata ranges, long-anchor uniqueness, and the
// two decoded pump calls.
Identity identify_image(const wchar_t *exe_name, uintptr_t mapped_base,
                        ImageRead read, void *user);

// Rechecks the strict mapped identity after the Receive hook is installed.
// Every live byte in the exact SafetyHook overwrite span must still match
// `expected_patch`; only then is that span normalized back to the verified
// 3663 Receive entry for the immutable-image comparison.
Identity identify_image_after_receive_patch(const wchar_t *exe_name,
                                            uintptr_t mapped_base,
                                            ImageRead read, void *user,
                                            const uint8_t *expected_patch,
                                            size_t expected_patch_size);

// Checks the current process executable. Reads are guarded by VirtualQuery
// and SEH. This function never opens or attaches to another process.
Identity identify_process();

// Guarded copy used by identify_process. It never writes mapped memory.
bool read_mapped_bytes(uintptr_t address, void *destination, size_t size);

bool identity_allows_hooks(const Identity &identity);
const char *identity_kind_name(IdentityKind kind);

} // namespace pkt3663
