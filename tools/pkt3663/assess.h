#pragma once
// Offline file assessment for Crimson Desert 1.0.0.3663.
// This is not a loader, not a hook, and not an adopted offset table.
// It does not know the 3576 send or receive RVAs. A 2976 or 3576 timestamp
// is rejected from the PE header before any signature or RTTI scan.

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace pkt3663 {

inline constexpr uint32_t kTimestamp3663 = 0x6AF5D198;
inline constexpr uint64_t kImageBase3663 = 0x140000000;
inline constexpr uint32_t kSizeOfImage3663 = 0x144E9000;
inline constexpr uint32_t kTimestamp3576 = 0x6AED2E85;
inline constexpr uint32_t kTimestamp2976 = 0x6AB28F00;

enum class FileKind {
    Unreadable,
    Rejected2976,
    Rejected3576,
    RejectedOther,
    Build3663,
};

struct Section {
    char name[9]{};
    uint32_t virtual_size = 0;
    uint32_t virtual_address = 0;
    uint32_t raw_size = 0;
    uint32_t raw_ptr = 0;
    uint32_t flags = 0;
};

struct ImageView {
    const uint8_t* data = nullptr;
    size_t size = 0;
    bool headers_ok = false;
    uint32_t timestamp = 0;
    bool timestamp_known = false;
    uint64_t image_base = 0;
    bool image_base_known = false;
    uint32_t size_of_image = 0;
    bool size_known = false;
    uint32_t size_of_headers = 0;
    uint32_t exception_rva = 0;
    uint32_t exception_size = 0;
    std::vector<Section> sections;
};

struct InstanceRead {
    uint32_t rva = 0;
    uint64_t first_qword = 0;
    uint32_t kind_u32 = 0;
    uint16_t opcode_u16 = 0;
    uint32_t flag_u32 = 0;
    uint16_t min_u16 = 0;
    uint16_t max_u16 = 0;
};

struct VtableRead {
    uint32_t rva = 0;
    bool slot0 = false;
    bool slot1 = false;
    bool slot2 = false;
    uint32_t slot0_rva = 0;
    uint32_t slot1_rva = 0;
    uint32_t slot2_rva = 0;
};

struct ClassChain {
    bool exact_name = false;
    std::string mangled;
    uint32_t type_descriptor_rva = 0;
    std::vector<uint32_t> col_rvas;
    std::vector<VtableRead> vtables;
    std::vector<InstanceRead> instances;
};

struct MessageHit {
    const char* key = "";
    bool found = false;
    std::vector<ClassChain> classes;
};

struct AllocSite {
    uint32_t rva = 0;
    const char* encoding = "";
    bool in_function = false;
    uint32_t function_begin = 0;
    uint32_t function_end = 0;
};

// A byte-for-byte occurrence of a sample taken from an older build.
// Hit means the bytes occur. It does not name a send or receive function.
struct FormerSampleHit {
    const char* label = "";
    uint32_t rva = 0;
};

struct Assessment {
    FileKind kind = FileKind::Unreadable;
    bool scanned = false;
    bool adopted = false;
    std::vector<MessageHit> messages;
    uint32_t alloc_site_count = 0;
    bool alloc_sites_truncated = false;
    std::vector<AllocSite> alloc_sites;
    uint32_t former_sample_count = 0;
    uint32_t former_send_count = 0;
    uint32_t former_recv_count = 0;
    bool former_samples_truncated = false;
    std::vector<FormerSampleHit> former_samples;
};

ImageView parse_image(std::span<const uint8_t> bytes);
FileKind classify(const ImageView& image);
const char* kind_name(FileKind kind);

// Scans only when classify(image) is Build3663. Every other kind returns
// with scanned == false and empty message, alloc, and sample lists.
Assessment assess(std::span<const uint8_t> bytes);

}  // namespace pkt3663
