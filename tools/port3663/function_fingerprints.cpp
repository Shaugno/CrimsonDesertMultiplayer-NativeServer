// File-only candidate discovery. Relative branch and RIP displacements are
// masked; all other instruction bytes remain in the fingerprint. A unique
// fingerprint is not semantic proof and never arms a gameplay path.
#include <Zydis/Zydis.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include "pkt3663/assess.h"

namespace {
const uint8_t* at(const pkt3663::ImageView& image, uint32_t rva, uint32_t size) {
    const uint8_t* result = nullptr;
    for (const auto& section : image.sections) {
        if (rva >= section.virtual_address && rva - section.virtual_address <= section.raw_size &&
            size <= section.raw_size - (rva - section.virtual_address)) {
            if (result) return nullptr;
            result = image.data + section.raw_ptr + (rva - section.virtual_address);
        }
    }
    return result;
}
uint32_t word(const uint8_t* bytes) {
    uint32_t out = 0;
    std::memcpy(&out, bytes, sizeof out);
    return out;
}
bool executable(const pkt3663::ImageView& image, uint32_t rva, uint32_t size) {
    for (const auto& section : image.sections) {
        if ((section.flags & 0x20000000) && rva >= section.virtual_address &&
            rva - section.virtual_address <= section.raw_size &&
            size <= section.raw_size - (rva - section.virtual_address)) return true;
    }
    return false;
}
void mix(uint64_t& hash, uint64_t& other, uint8_t byte) {
    hash = (hash ^ byte) * 1099511628211ull;
    other = (other ^ byte) * 0x9E3779B185EBCA87ull;
}
}

int main(int argc, char** argv) {
    if (argc != 2) { std::fprintf(stderr, "usage: port3663_fingerprints <exe>\n"); return 2; }
    std::ifstream input(argv[1], std::ios::binary | std::ios::ate);
    if (!input) return 2;
    const auto size = input.tellg();
    if (size <= 0 || size > 1024ll * 1024 * 1024) return 2;
    std::vector<uint8_t> bytes(static_cast<size_t>(size));
    input.seekg(0);
    if (!input.read(reinterpret_cast<char*>(bytes.data()), size)) return 2;
    const auto image = pkt3663::parse_image(bytes);
    if (!image.headers_ok || image.image_base != 0x140000000 ||
        !((image.timestamp == 0x6AB28F00 && image.size_of_image == 0x173AB000) ||
          (image.timestamp == 0x6AF5D198 && image.size_of_image == 0x144E9000))) return 2;
    if (!image.exception_size || image.exception_size % 12) return 2;
    const auto* pdata = at(image, image.exception_rva, image.exception_size);
    if (!pdata) return 2;
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    std::printf("# timestamp=0x%X image_size=0x%X adopted=0\n", image.timestamp, image.size_of_image);
    std::printf("begin\tend\tinstructions\tfingerprint\tcalls\n");
    uint32_t accepted = 0, refused = 0;
    for (uint32_t record = 0; record < image.exception_size; record += 12) {
        const uint32_t begin = word(pdata + record), end = word(pdata + record + 4);
        if (end <= begin || end - begin > 1024 * 1024 || !executable(image, begin, end - begin)) {
            ++refused; continue;
        }
        const auto* function = at(image, begin, end - begin);
        if (!function) { ++refused; continue; }
        uint64_t hash = 14695981039346656037ull, other = 0xD6E8FEB86659FD93ull;
        uint32_t cursor = 0, instructions = 0;
        std::vector<uint32_t> calls;
        bool valid = true;
        while (cursor < end - begin) {
            ZydisDecodedInstruction instruction{};
            ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT]{};
            if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, function + cursor, end - begin - cursor,
                                                     &instruction, operands))) { valid = false; break; }
            uint8_t normalized[ZYDIS_MAX_INSTRUCTION_LENGTH]{};
            std::memcpy(normalized, function + cursor, instruction.length);
            bool rip = false;
            for (uint8_t i = 0; i < instruction.operand_count_visible; ++i) {
                rip = rip || (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY && operands[i].mem.base == ZYDIS_REGISTER_RIP);
                if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL && operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE &&
                    operands[i].imm.is_relative) {
                    const int64_t destination = static_cast<int64_t>(begin + cursor + instruction.length) + operands[i].imm.value.s;
                    if (destination >= 0 && destination < image.size_of_image) calls.push_back(static_cast<uint32_t>(destination));
                }
            }
            if (rip && instruction.raw.disp.size)
                std::memset(normalized + instruction.raw.disp.offset, 0, instruction.raw.disp.size / 8);
            for (const auto& immediate : instruction.raw.imm) {
                if (immediate.is_relative && immediate.size)
                    std::memset(normalized + immediate.offset, 0, immediate.size / 8);
            }
            mix(hash, other, instruction.length);
            for (uint8_t i = 0; i < instruction.length; ++i) mix(hash, other, normalized[i]);
            cursor += instruction.length;
            ++instructions;
        }
        if (!valid) { ++refused; continue; }
        ++accepted;
        std::printf("0x%X\t0x%X\t%u\t%016llX%016llX\t", begin, end, instructions,
                    static_cast<unsigned long long>(hash), static_cast<unsigned long long>(other));
        for (size_t i = 0; i < calls.size(); ++i) std::printf("%s0x%X", i ? "," : "", calls[i]);
        std::printf("\n");
    }
    std::fprintf(stderr, "decoded=%u refused=%u adopted=0\n", accepted, refused);
    return 0;
}
