// Read a Crimson Desert executable as a data file and print the 3663 assessment.
// This process does not open, attach to, or launch the game.

#include <cstdio>
#include <fstream>
#include <vector>

#include "pkt3663/assess.h"

namespace {

std::vector<uint8_t> read_file(const char* path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    in.seekg(0, std::ios::end);
    const std::streamoff end = in.tellg();
    if (end <= 0) return {};
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes(static_cast<size_t>(end));
    in.read(reinterpret_cast<char*>(bytes.data()), end);
    if (!in) return {};
    return bytes;
}

void print_message(const pkt3663::MessageHit& hit) {
    std::printf("message key=%s found=%d class_count=%zu\n", hit.key, hit.found ? 1 : 0, hit.classes.size());
    for (const pkt3663::ClassChain& chain : hit.classes) {
        std::printf("  class exact=%d mangled=%s td_rva=0x%X col_count=%zu vtable_count=%zu instance_count=%zu\n",
                    chain.exact_name ? 1 : 0, chain.mangled.c_str(), chain.type_descriptor_rva, chain.col_rvas.size(),
                    chain.vtables.size(), chain.instances.size());
        for (uint32_t col : chain.col_rvas) std::printf("  col rva=0x%X\n", col);
        for (const pkt3663::VtableRead& vt : chain.vtables) {
            std::printf("  vtable rva=0x%X slot0=%d:0x%X slot1=%d:0x%X slot2=%d:0x%X\n", vt.rva, vt.slot0 ? 1 : 0,
                        vt.slot0_rva, vt.slot1 ? 1 : 0, vt.slot1_rva, vt.slot2 ? 1 : 0, vt.slot2_rva);
        }
        for (const pkt3663::InstanceRead& instance : chain.instances) {
            std::printf(
                "  instance rva=0x%X first_qword=0x%llX kind_u32=%u opcode_u16=%u flag_u32=0x%X min_u16=%u max_u16=%u\n",
                instance.rva, static_cast<unsigned long long>(instance.first_qword), instance.kind_u32,
                instance.opcode_u16, instance.flag_u32, instance.min_u16, instance.max_u16);
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: pkt3663_scan <CrimsonDesert.exe>\n");
        return 2;
    }
    const std::vector<uint8_t> bytes = read_file(argv[1]);
    if (bytes.empty()) {
        std::fprintf(stderr, "unreadable file\n");
        return 2;
    }
    const pkt3663::ImageView image = pkt3663::parse_image(bytes);
    const pkt3663::Assessment result = pkt3663::assess(bytes);
    std::printf("file_bytes=%zu\n", bytes.size());
    std::printf("timestamp=0x%08X\n", image.timestamp);
    std::printf("image_base=0x%llX\n", static_cast<unsigned long long>(image.image_base));
    std::printf("size_of_image=0x%X\n", image.size_of_image);
    std::printf("identity=%s\n", pkt3663::kind_name(result.kind));
    std::printf("scanned=%d\n", result.scanned ? 1 : 0);
    std::printf("adopted=%d\n", result.adopted ? 1 : 0);
    std::printf("section_count=%zu\n", image.sections.size());
    for (const pkt3663::Section& section : image.sections) {
        std::printf("section name=%s va=0x%X vsize=0x%X raw=0x%X raw_size=0x%X flags=0x%X\n", section.name,
                    section.virtual_address, section.virtual_size, section.raw_ptr, section.raw_size, section.flags);
    }
    if (!result.scanned) return 1;
    for (const pkt3663::MessageHit& hit : result.messages) print_message(hit);
    std::printf("alloc_site_count=%u truncated=%d\n", result.alloc_site_count, result.alloc_sites_truncated ? 1 : 0);
    for (const pkt3663::AllocSite& site : result.alloc_sites) {
        std::printf("alloc rva=0x%X encoding=%s in_function=%d begin=0x%X end=0x%X\n", site.rva, site.encoding,
                    site.in_function ? 1 : 0, site.function_begin, site.function_end);
    }
    std::printf("former_sample_count=%u send_count=%u recv_count=%u truncated=%d adopted=0\n", result.former_sample_count,
                result.former_send_count, result.former_recv_count, result.former_samples_truncated ? 1 : 0);
    for (const pkt3663::FormerSampleHit& hit : result.former_samples) {
        std::printf("former_sample label=%s rva=0x%X adopted=0\n", hit.label, hit.rva);
    }
    return 0;
}
