#pragma once
// Pure, game-free verification of the experimental 3663 packet-entry evidence.
// Passing this check permits work on a separate passive collector. It does not
// adopt these RVAs for the ASI and does not prove any packet payload layout.

#include <cstdint>
#include <span>

namespace pkt3663 {

enum class EntryStatus {
    WrongIdentity,
    MalformedImage,
    MissingFunctionTable,
    NotExecutable,
    SendRangeMismatch,
    ReceiveRangeMismatch,
    PumpRangeMismatch,
    SendAnchorMismatch,
    ReceiveAnchorMismatch,
    PumpAnchorMismatch,
    AmbiguousAnchor,
    CallTargetMismatch,
    VerifiedFileCandidates,
};

struct EntryVerification {
    EntryStatus status = EntryStatus::WrongIdentity;
    bool identity_ok = false;
    bool send_range_ok = false;
    bool receive_range_ok = false;
    bool pump_range_ok = false;
    bool send_anchors_ok = false;
    bool receive_anchors_ok = false;
    bool pump_anchors_ok = false;
    bool role_calls_ok = false;
    bool passive_capture_ready = false;
    bool adopted = false;
    uint32_t failed_anchor_rva = 0;
};

EntryVerification verify_entries(std::span<const uint8_t> bytes);
const char* entry_status_name(EntryStatus status);

}  // namespace pkt3663
