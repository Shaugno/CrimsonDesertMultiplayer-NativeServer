// Read-only scanner for Crimson Desert 1.0.0.3576 client packet functions.
// Search keys come from the 2976 research used by src/mod/net/troc_sniffer.cpp.
// A prologue match is counted and then ignored. A site is interesting only when
// a decoded instruction really carries a 2976 structural immediate or
// displacement, inside an exception-directory range.
// This program does not attach to the game, adopt an offset, or install a hook.
// Build and run with tools/probe-3576/run_packet_scan.ps1.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

constexpr uint32_t kExpectedTimestamp = 0x6AED2E85;
constexpr uint64_t kExpectedSize = 326872984ull;
constexpr uint32_t kOldPost = 0x35C670;
constexpr uint32_t kOldRecv = 0x35CA00;

const uint8_t kPostHead[14] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C,
                                0x24, 0x20, 0x55, 0x56, 0x57, 0x41};
const uint8_t kRecvHead[14] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
                                0x24, 0x10, 0x48, 0x89, 0x7C, 0x24};
// 2976 NetworkThread pump, opcion-d-p8.md section 3.3. The final 0x74 is the
// first byte of a jz whose displacement is not part of the published pattern.
const uint8_t kPump[39] = {0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38,
                            0x48, 0x89, 0x7D, 0xC7, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF,
                            0xFF, 0x50, 0x08, 0x90, 0x8B, 0x5F, 0x58, 0x48, 0x8B, 0x07,
                            0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x10, 0x85, 0xDB, 0x74};

enum : uint16_t {
    F_INV = 1,
    F_MOD = 2,
    F_I8 = 4,
    F_IW = 8,
    F_I16 = 16,
    F_REL8 = 32,
    F_REL32 = 64,
    F_MOFF = 128,
    F_I64 = 256,
    F_ENT = 512,
    F_F6 = 1024,
    F_F7 = 2048,
};

uint16_t g_pri[256];
uint16_t g_0f[256];

void set_range(uint16_t* t, int a, int b, uint16_t f) {
    for (int i = a; i <= b; ++i) t[i] = f;
}

void init_tables() {
    for (int i = 0; i < 256; ++i) {
        g_pri[i] = F_INV;
        g_0f[i] = F_INV;
    }
    for (int base = 0; base <= 0x38; base += 8) {
        set_range(g_pri, base, base + 3, F_MOD);
        g_pri[base + 4] = F_I8;
        g_pri[base + 5] = F_IW;
    }
    set_range(g_pri, 0x50, 0x5F, 0);
    g_pri[0x63] = F_MOD;
    g_pri[0x68] = F_IW;
    g_pri[0x69] = F_MOD | F_IW;
    g_pri[0x6A] = F_I8;
    g_pri[0x6B] = F_MOD | F_I8;
    set_range(g_pri, 0x6C, 0x6F, 0);
    set_range(g_pri, 0x70, 0x7F, F_REL8);
    g_pri[0x80] = F_MOD | F_I8;
    g_pri[0x81] = F_MOD | F_IW;
    g_pri[0x83] = F_MOD | F_I8;
    set_range(g_pri, 0x84, 0x8F, F_MOD);
    set_range(g_pri, 0x90, 0x97, 0);
    g_pri[0x98] = 0;
    g_pri[0x99] = 0;
    g_pri[0x9B] = 0;
    g_pri[0x9C] = 0;
    g_pri[0x9D] = 0;
    g_pri[0x9E] = 0;
    g_pri[0x9F] = 0;
    set_range(g_pri, 0xA0, 0xA3, F_MOFF);
    set_range(g_pri, 0xA4, 0xA7, 0);
    g_pri[0xA8] = F_I8;
    g_pri[0xA9] = F_IW;
    set_range(g_pri, 0xAA, 0xAF, 0);
    set_range(g_pri, 0xB0, 0xB7, F_I8);
    set_range(g_pri, 0xB8, 0xBF, F_I64);
    g_pri[0xC0] = F_MOD | F_I8;
    g_pri[0xC1] = F_MOD | F_I8;
    g_pri[0xC2] = F_I16;
    g_pri[0xC3] = 0;
    g_pri[0xC6] = F_MOD | F_I8;
    g_pri[0xC7] = F_MOD | F_IW;
    g_pri[0xC8] = F_ENT;
    g_pri[0xC9] = 0;
    g_pri[0xCA] = F_I16;
    g_pri[0xCB] = 0;
    g_pri[0xCC] = 0;
    g_pri[0xCD] = F_I8;
    g_pri[0xCF] = 0;
    set_range(g_pri, 0xD0, 0xD3, F_MOD);
    g_pri[0xD7] = 0;
    set_range(g_pri, 0xD8, 0xDF, F_MOD);
    set_range(g_pri, 0xE0, 0xE3, F_REL8);
    set_range(g_pri, 0xE4, 0xE7, F_I8);
    g_pri[0xE8] = F_REL32;
    g_pri[0xE9] = F_REL32;
    g_pri[0xEB] = F_REL8;
    set_range(g_pri, 0xEC, 0xEF, 0);
    g_pri[0xF1] = 0;
    g_pri[0xF4] = 0;
    g_pri[0xF5] = 0;
    g_pri[0xF6] = F_MOD | F_F6;
    g_pri[0xF7] = F_MOD | F_F7;
    set_range(g_pri, 0xF8, 0xFD, 0);
    g_pri[0xFE] = F_MOD;
    g_pri[0xFF] = F_MOD;

    set_range(g_0f, 0x00, 0x03, F_MOD);
    const uint8_t bare[] = {0x05, 0x06, 0x07, 0x08, 0x09, 0x0B, 0x0E, 0x30, 0x31, 0x32,
                             0x33, 0x34, 0x35, 0x37, 0x77, 0xA0, 0xA1, 0xA2, 0xA8, 0xA9, 0xAA};
    for (uint8_t b : bare) g_0f[b] = 0;
    set_range(g_0f, 0x10, 0x1F, F_MOD);
    g_0f[0x0D] = F_MOD;
    set_range(g_0f, 0x20, 0x23, F_MOD);
    set_range(g_0f, 0x28, 0x2F, F_MOD);
    set_range(g_0f, 0x40, 0x76, F_MOD);
    set_range(g_0f, 0x78, 0x7F, F_MOD);
    set_range(g_0f, 0x80, 0x8F, F_REL32);
    set_range(g_0f, 0x90, 0x9F, F_MOD);
    g_0f[0xA3] = F_MOD;
    g_0f[0xA4] = F_MOD | F_I8;
    g_0f[0xA5] = F_MOD;
    g_0f[0xAB] = F_MOD;
    g_0f[0xAC] = F_MOD | F_I8;
    g_0f[0xAD] = F_MOD;
    g_0f[0xAE] = F_MOD;
    g_0f[0xAF] = F_MOD;
    set_range(g_0f, 0xB0, 0xB7, F_MOD);
    g_0f[0xB8] = F_MOD;
    g_0f[0xB9] = F_MOD;
    g_0f[0xBA] = F_MOD | F_I8;
    set_range(g_0f, 0xBB, 0xBF, F_MOD);
    g_0f[0xC0] = F_MOD;
    g_0f[0xC1] = F_MOD;
    g_0f[0xC2] = F_MOD | F_I8;
    g_0f[0xC3] = F_MOD;
    g_0f[0xC4] = F_MOD | F_I8;
    g_0f[0xC5] = F_MOD | F_I8;
    g_0f[0xC6] = F_MOD | F_I8;
    g_0f[0xC7] = F_MOD;
    set_range(g_0f, 0xC8, 0xCF, 0);
    set_range(g_0f, 0xD0, 0xFE, F_MOD);
    set_range(g_0f, 0x70, 0x73, F_MOD | F_I8);
    g_0f[0x0F] = F_MOD | F_I8;
}

struct Dec {
    int len = 0;
    uint8_t map = 0;
    uint8_t opcode = 0;
    uint8_t rex = 0;
    uint8_t modrm = 0;
    bool has_modrm = false;
    bool has_imm32 = false;
    bool has_disp32 = false;
    bool has_imm8 = false;
    bool rel32 = false;
    bool vex = false;
    uint32_t imm32 = 0;
    uint32_t disp32 = 0;
    uint8_t imm8 = 0;
    int imm_off = -1;
    int disp_off = -1;
    int modrm_off = -1;
};

int modrm_extra_len(const uint8_t* p, const uint8_t* end) {
    if (p >= end) return -1;
    const uint8_t modrm = p[0];
    const int mod = modrm >> 6;
    const int rm = modrm & 7;
    if (mod != 3 && rm == 4) {
        if (p + 1 >= end) return -1;
        const int base = p[1] & 7;
        int add = 0;
        if (mod == 0 && base == 5) add = 4;
        else if (mod == 1) add = 1;
        else if (mod == 2) add = 4;
        if (p + 2 + add > end) return -1;
        return 2 + add;
    }
    int add = 0;
    if (mod == 0 && rm == 5) add = 4;
    else if (mod == 1) add = 1;
    else if (mod == 2) add = 4;
    if (p + 1 + add > end) return -1;
    return 1 + add;
}

uint16_t flags_for(uint8_t map, uint8_t op) {
    if (map == 0) return g_pri[op];
    if (map == 1) return g_0f[op];
    if (map == 2) return F_MOD;
    if (map == 3) return F_MOD | F_I8;
    return F_INV;
}

int decode(const uint8_t* s, const uint8_t* end, Dec& d) {
    d = {};
    const uint8_t* p = s;
    if (p >= end) return 0;
    uint8_t rex = 0;
    bool p66 = false;
    bool p67 = false;
    bool legacy = false;
    int guard = 0;
    while (p < end && guard < 14) {
        const uint8_t b = *p;
        const bool is_legacy = b == 0xF0 || b == 0xF2 || b == 0xF3 || b == 0x2E || b == 0x36 || b == 0x3E ||
                               b == 0x26 || b == 0x64 || b == 0x65 || b == 0x66 || b == 0x67;
        if (is_legacy) {
            if (b == 0x66) p66 = true;
            if (b == 0x67) p67 = true;
            rex = 0;
            legacy = true;
            ++p;
            ++guard;
            continue;
        }
        if (b >= 0x40 && b <= 0x4F) {
            rex = b;
            ++p;
            ++guard;
            continue;
        }
        break;
    }
    if (p >= end) return 0;
    d.rex = rex;

    if (!legacy && rex == 0 && (*p == 0xC5 || *p == 0xC4 || *p == 0x62)) {
        d.vex = true;
        if (*p == 0xC5) {
            if (p + 3 > end) return 0;
            d.map = 1;
            d.opcode = p[2];
            p += 3;
        } else if (*p == 0xC4) {
            if (p + 4 > end) return 0;
            const uint8_t mmmmm = p[1] & 0x1F;
            if (mmmmm < 1 || mmmmm > 3) return 0;
            d.map = mmmmm;
            d.opcode = p[3];
            p += 4;
        } else {
            if (p + 5 > end) return 0;
            const uint8_t p0 = p[1];
            const uint8_t p1 = p[2];
            if ((p0 & 0x0C) != 0 || (p1 & 0x04) == 0) return 0;
            const uint8_t mm = p0 & 3;
            if (mm == 0) return 0;
            d.map = mm;
            d.opcode = p[4];
            p += 5;
        }
    } else {
        uint8_t op = *p++;
        if (op == 0x0F) {
            if (p >= end) return 0;
            uint8_t op2 = *p++;
            if (op2 == 0x38 || op2 == 0x3A) {
                if (p >= end) return 0;
                d.map = op2 == 0x38 ? 2 : 3;
                d.opcode = *p++;
            } else {
                d.map = 1;
                d.opcode = op2;
            }
        } else {
            d.map = 0;
            d.opcode = op;
        }
    }

    uint16_t f = flags_for(d.map, d.opcode);
    if (f & F_INV) return 0;
    const bool rex_w = (rex & 0x8) != 0;
    if (f & F_MOD) {
        if (p >= end) return 0;
        d.has_modrm = true;
        d.modrm_off = static_cast<int>(p - s);
        d.modrm = *p;
        const int extra = modrm_extra_len(p, end);
        if (extra < 0) return 0;
        const int mod = d.modrm >> 6;
        const int rm = d.modrm & 7;
        bool disp32 = false;
        if (mod != 3 && rm == 4) {
            const int base = p[1] & 7;
            if ((mod == 0 && base == 5) || mod == 2) disp32 = true;
        } else if ((mod == 0 && rm == 5) || mod == 2) {
            disp32 = true;
        }
        if (disp32) {
            d.has_disp32 = true;
            d.disp_off = d.modrm_off + extra - 4;
            std::memcpy(&d.disp32, s + d.disp_off, 4);
        }
        p += extra;
        const int digit = (d.modrm >> 3) & 7;
        if ((f & F_F6) && (digit == 0 || digit == 1)) f = static_cast<uint16_t>(f | F_I8);
        if ((f & F_F7) && (digit == 0 || digit == 1)) f = static_cast<uint16_t>(f | F_IW);
    }

    int isize = 0;
    if (f & F_I8) isize = 1;
    else if (f & F_REL8) isize = 1;
    else if (f & F_I16) isize = 2;
    else if (f & F_ENT) isize = 3;
    else if (f & F_REL32) isize = 4;
    else if (f & F_MOFF) isize = p67 ? 4 : 8;
    else if (f & F_I64) isize = rex_w ? 8 : (p66 ? 2 : 4);
    else if (f & F_IW) isize = (p66 && !rex_w) ? 2 : 4;
    if (isize) {
        if (p + isize > end) return 0;
        d.imm_off = static_cast<int>(p - s);
        if (isize == 1) {
            d.has_imm8 = true;
            d.imm8 = *p;
        }
        if (isize == 4) {
            d.has_imm32 = true;
            std::memcpy(&d.imm32, p, 4);
            if (f & F_REL32) d.rel32 = true;
        }
        p += isize;
    }
    const int len = static_cast<int>(p - s);
    if (len <= 0 || len > 15) return 0;
    d.len = len;
    return len;
}

struct Expect {
    const char* name;
    const uint8_t* bytes;
    int n;
    int want;
};

bool self_test() {
    struct Item {
        const char* name;
        uint8_t b[16];
        int n;
        int want;
    };
    const Item items[] = {
        {"mov [rsp+10], rbx", {0x48, 0x89, 0x5C, 0x24, 0x10}, 5, 5},
        {"mov [rsp+20], r9", {0x4C, 0x89, 0x4C, 0x24, 0x20}, 5, 5},
        {"push rbp", {0x55}, 1, 1},
        {"call rel", {0xE8, 0, 0, 0, 0}, 5, 5},
        {"call [rax+8]", {0xFF, 0x50, 0x08}, 3, 3},
        {"mov rax, [r15+E0]", {0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00}, 7, 7},
        {"mov rdi, [rax]", {0x48, 0x8B, 0x38}, 3, 3},
        {"mov [rbp-39], rdi", {0x48, 0x89, 0x7D, 0xC7}, 4, 4},
        {"nop", {0x90}, 1, 1},
        {"mov ebx, [rdi+58]", {0x8B, 0x5F, 0x58}, 3, 3},
        {"test ebx, ebx", {0x85, 0xDB}, 2, 2},
        {"jz", {0x74, 0x00}, 2, 2},
        {"cmp ecx, 7FD7", {0x81, 0xF9, 0xD7, 0x7F, 0x00, 0x00}, 6, 6},
        {"mov ecx, 8030", {0xB9, 0x30, 0x80, 0x00, 0x00}, 5, 5},
        {"lea rcx, [rax+8028]", {0x48, 0x8D, 0x88, 0x28, 0x80, 0x00, 0x00}, 7, 7},
        {"mov rax, [rax+800A]", {0x48, 0x8B, 0x80, 0x0A, 0x80, 0x00, 0x00}, 7, 7},
        {"sub eax, 1F", {0x83, 0xE8, 0x1F}, 3, 3},
        {"vxorps", {0xC5, 0xF8, 0x57, 0xC0}, 4, 4},
        {"vmovss rip", {0xC5, 0xFA, 0x10, 0x05, 0, 0, 0, 0}, 8, 8},
        {"vbroadcastss", {0xC4, 0xE2, 0x79, 0x18, 0x05, 0, 0, 0, 0}, 9, 9},
        {"vmovups ymm rip", {0xC5, 0xFC, 0x10, 0x05, 0, 0, 0, 0}, 8, 8},
        {"evex vmovups rip", {0x62, 0xF1, 0x7C, 0x08, 0x10, 0x05, 0, 0, 0, 0}, 10, 10},
        {"endbr64", {0xF3, 0x0F, 0x1E, 0xFA}, 4, 4},
        {"nop dword", {0x0F, 0x1F, 0x44, 0x00, 0x00}, 5, 5},
        {"nop long", {0x0F, 0x1F, 0x84, 0x00, 0, 0, 0, 0}, 8, 8},
        {"lea rcx, [rip]", {0x48, 0x8D, 0x0D, 0, 0, 0, 0}, 7, 7},
        {"enter", {0xC8, 0, 0, 0}, 4, 4},
        {"mov rax, imm64", {0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0}, 10, 10},
        {"xchg ax, ax", {0x66, 0x90}, 2, 2},
        {"sub rsp, 28", {0x48, 0x83, 0xEC, 0x28}, 4, 4},
        {"movd xmm0, eax", {0xC4, 0xE1, 0x79, 0x6E, 0xC0}, 5, 5},
        {"vpermilps", {0xC4, 0xE3, 0x79, 0x04, 0xC0, 0x00}, 6, 6},
    };
    bool ok = true;
    for (const Item& it : items) {
        Dec d;
        const int n = decode(it.b, it.b + it.n, d);
        if (n != it.want) {
            std::printf("SELF-TEST FAIL %s got %d want %d\n", it.name, n, it.want);
            ok = false;
        }
    }
    Dec d;
    const int cmp = decode(items[12].b, items[12].b + items[12].n, d);
    if (cmp != 6 || !d.has_imm32 || d.imm32 != 0x7FD7) {
        std::printf("SELF-TEST FAIL cmp immediate\n");
        ok = false;
    }
    const int mov = decode(items[13].b, items[13].b + items[13].n, d);
    if (mov != 5 || !d.has_imm32 || d.imm32 != 0x8030) {
        std::printf("SELF-TEST FAIL mov 8030\n");
        ok = false;
    }
    const int lea = decode(items[14].b, items[14].b + items[14].n, d);
    if (lea != 7 || !d.has_disp32 || d.disp32 != 0x8028) {
        std::printf("SELF-TEST FAIL disp 8028\n");
        ok = false;
    }
    int sum = 0;
    for (int i = 0; i < 38;) {
        const int n = decode(kPump + i, kPump + 38, d);
        if (n <= 0) {
            std::printf("SELF-TEST FAIL pump at %d\n", i);
            ok = false;
            break;
        }
        i += n;
        sum = i;
    }
    if (sum != 38) {
        std::printf("SELF-TEST FAIL pump sum %d\n", sum);
        ok = false;
    }
    if (ok) std::printf("self-test: pass (%d lengths, pump prefix 38)\n", 31);
    return ok;
}

struct Section {
    std::string name;
    uint32_t va = 0;
    uint32_t virtual_size = 0;
    uint32_t raw = 0;
    uint32_t raw_size = 0;
    uint32_t chars = 0;
    bool executable() const { return (chars & 0x20000000u) != 0; }
};

struct Image {
    std::vector<uint8_t> bytes;
    uint32_t timestamp = 0;
    uint32_t size_of_image = 0;
    uint64_t image_base = 0;
    uint32_t except_rva = 0;
    uint32_t except_size = 0;
    std::vector<Section> sections;
};

struct Rf {
    uint32_t begin = 0;
    uint32_t end = 0;
    uint8_t st = 0;  // 0 exact, 1 short, 2 over, 3 nobytes
};

enum Kind : int {
    K_8030 = 0,
    K_8028,
    K_800A,
    K_8014,
    K_8000,
    K_7FD7,
    K_7FDA,
    K_7FC6,
    K_E0,
    K_COUNT
};

struct Agg {
    uint32_t end = 0;
    uint8_t exact = 0;
    int n[K_COUNT] = {};
};

bool load_image(const char* path, Image& img) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(0, std::ios::end);
    const auto n = static_cast<size_t>(in.tellg());
    in.seekg(0);
    img.bytes.resize(n);
    in.read(reinterpret_cast<char*>(img.bytes.data()), static_cast<std::streamsize>(n));
    if (!in) return false;
    if (n < 0x200 || img.bytes[0] != 'M' || img.bytes[1] != 'Z') return false;
    uint32_t e_lfanew = 0;
    std::memcpy(&e_lfanew, img.bytes.data() + 0x3C, 4);
    if (static_cast<size_t>(e_lfanew) + 0x118 > n) return false;
    if (std::memcmp(img.bytes.data() + e_lfanew, "PE\0\0", 4) != 0) return false;
    std::memcpy(&img.timestamp, img.bytes.data() + e_lfanew + 8, 4);
    uint16_t magic = 0;
    std::memcpy(&magic, img.bytes.data() + e_lfanew + 24, 2);
    if (magic != 0x20B) return false;
    const uint32_t opt = e_lfanew + 24;
    std::memcpy(&img.image_base, img.bytes.data() + opt + 24, 8);
    std::memcpy(&img.size_of_image, img.bytes.data() + opt + 56, 4);
    uint32_t num_rva = 0;
    std::memcpy(&num_rva, img.bytes.data() + opt + 108, 4);
    if (num_rva > 3) {
        std::memcpy(&img.except_rva, img.bytes.data() + opt + 112 + 24, 4);
        std::memcpy(&img.except_size, img.bytes.data() + opt + 112 + 28, 4);
    }
    uint16_t number_of_sections = 0;
    uint16_t size_of_optional = 0;
    std::memcpy(&number_of_sections, img.bytes.data() + e_lfanew + 6, 2);
    std::memcpy(&size_of_optional, img.bytes.data() + e_lfanew + 20, 2);
    const uint32_t sec_off = e_lfanew + 24 + size_of_optional;
    for (uint16_t i = 0; i < number_of_sections; ++i) {
        const uint32_t o = sec_off + static_cast<uint32_t>(i) * 40;
        if (o + 40 > n) return false;
        Section s;
        char name[9] = {};
        std::memcpy(name, img.bytes.data() + o, 8);
        s.name = name;
        std::memcpy(&s.virtual_size, img.bytes.data() + o + 8, 4);
        std::memcpy(&s.va, img.bytes.data() + o + 12, 4);
        std::memcpy(&s.raw_size, img.bytes.data() + o + 16, 4);
        std::memcpy(&s.raw, img.bytes.data() + o + 20, 4);
        std::memcpy(&s.chars, img.bytes.data() + o + 36, 4);
        img.sections.push_back(s);
    }
    return true;
}

const Section* section_at(const Image& img, uint32_t rva) {
    for (const auto& s : img.sections) {
        const uint32_t span = std::max(s.virtual_size, s.raw_size);
        if (rva >= s.va && rva < s.va + span) return &s;
    }
    return nullptr;
}

std::string section_name(const Image& img, uint32_t rva) {
    const Section* s = section_at(img, rva);
    return s ? s->name : "(none)";
}

bool file_bytes(const Image& img, uint32_t rva, const uint8_t** out, uint32_t n) {
    const Section* s = section_at(img, rva);
    if (!s || rva < s->va) return false;
    const uint32_t delta = rva - s->va;
    if (delta + n > s->raw_size) return false;
    if (static_cast<size_t>(s->raw) + delta + n > img.bytes.size()) return false;
    *out = img.bytes.data() + s->raw + delta;
    return true;
}

uint32_t file_off(const Image& img, uint32_t rva) {
    const Section* s = section_at(img, rva);
    if (!s || rva < s->va) return 0;
    return s->raw + (rva - s->va);
}

uint32_t raw_avail(const Image& img, uint32_t rva) {
    const Section* s = section_at(img, rva);
    if (!s || rva < s->va) return 0;
    const uint32_t delta = rva - s->va;
    if (delta >= s->raw_size) return 0;
    return s->raw_size - delta;
}

void print_bytes(const uint8_t* p, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) std::printf("%02X", p[i]);
}

const char* reg_name(int r) {
    static const char* names[] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
                                  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"};
    if (r < 0 || r > 15) return "?";
    return names[r];
}

int kind_of_imm(uint32_t v) {
    switch (v) {
        case 0x8030: return K_8030;
        case 0x7FD7: return K_7FD7;
        case 0x7FDA: return K_7FDA;
        case 0x7FC6: return K_7FC6;
        case 0xE0: return K_E0;
        default: return -1;
    }
}

int kind_of_disp(uint32_t v) {
    switch (v) {
        case 0x8028: return K_8028;
        case 0x800A: return K_800A;
        case 0x8014: return K_8014;
        case 0x8000: return K_8000;
        case 0xE0: return K_E0;
        default: return -1;
    }
}

const char* kind_name(int k) {
    static const char* names[] = {"imm8030", "disp8028", "disp800A", "disp8014", "disp8000",
                                  "imm7FD7", "imm7FDA", "imm7FC6", "dispE0"};
    if (k < 0 || k >= K_COUNT) return "?";
    return names[k];
}

struct Cov {
    uint64_t n = 0;
    uint64_t exact = 0;
    uint64_t shrt = 0;
    uint64_t over = 0;
    uint64_t nobytes = 0;
};

struct FailEx {
    uint32_t rva = 0;
    uint8_t map = 0;
    uint8_t op = 0;
    uint8_t b[8] = {};
};

const Rf* find_entry(const std::vector<Rf>& funcs, uint32_t rva) {
    auto it = std::lower_bound(funcs.begin(), funcs.end(), rva,
                               [](const Rf& f, uint32_t v) { return f.begin < v; });
    if (it == funcs.end() || it->begin != rva) return nullptr;
    return &*it;
}

const Rf* find_tight(const std::vector<Rf>& funcs, uint32_t rva) {
    auto it = std::upper_bound(funcs.begin(), funcs.end(), rva,
                               [](uint32_t v, const Rf& f) { return v < f.begin; });
    const Rf* best = nullptr;
    int steps = 0;
    while (it != funcs.begin() && steps < 16384) {
        --it;
        ++steps;
        if (it->begin <= rva && rva < it->end) {
            if (!best || (it->end - it->begin) < (best->end - best->begin)) best = &*it;
        }
        if (best && rva - it->begin > 0x200000) break;
        if (!best && rva > it->begin && rva - it->begin > 0x200000) break;
    }
    return best;
}

const Rf* find_outer(const std::vector<Rf>& funcs, uint32_t rva) {
    auto it = std::upper_bound(funcs.begin(), funcs.end(), rva,
                               [](uint32_t v, const Rf& f) { return v < f.begin; });
    const Rf* best = nullptr;
    int steps = 0;
    while (it != funcs.begin() && steps < 16384) {
        --it;
        ++steps;
        if (it->begin <= rva && rva < it->end) {
            if (!best || it->begin < best->begin) best = &*it;
        }
        if (best && rva - it->begin > 0x200000) break;
        if (!best && rva > it->begin && rva - it->begin > 0x200000) break;
    }
    return best;
}

enum Walk { W_EXACT, W_SHORT, W_OVER, W_NOBYTES };

Walk walk_range(const Image& img, uint32_t begin, uint32_t end, bool record, Agg* agg,
                std::vector<std::pair<uint32_t, int>>* sites, int max_insns) {
    if (end <= begin) return W_NOBYTES;
    const uint32_t need = end - begin;
    const uint32_t avail = raw_avail(img, begin);
    const uint8_t* p = nullptr;
    const uint32_t limit = std::min(avail, need);
    if (limit == 0 || !file_bytes(img, begin, &p, limit)) return W_NOBYTES;
    uint32_t ip = 0;
    int insns = 0;
    while (ip < need) {
        if (max_insns && insns >= max_insns) return W_SHORT;
        if (ip >= limit) return W_SHORT;
        Dec d;
        const int n = decode(p + ip, p + limit, d);
        if (n <= 0) return W_SHORT;
        if (ip + static_cast<uint32_t>(n) > need) return W_OVER;
        if (record && agg) {
            if (d.has_imm32 && !d.rel32) {
                const int k = kind_of_imm(d.imm32);
                if (k >= 0) {
                    agg->n[k]++;
                    if (sites && k != K_E0) sites->push_back({begin + ip, k});
                }
            }
            if (d.has_disp32) {
                const int k = kind_of_disp(d.disp32);
                if (k >= 0) {
                    agg->n[k]++;
                    if (sites && k != K_E0 && k != K_8000) sites->push_back({begin + ip, k});
                }
            }
        }
        ip += static_cast<uint32_t>(n);
        ++insns;
    }
    return ip == need ? W_EXACT : W_OVER;
}

void note_fail(std::vector<FailEx>& ex, uint32_t rva, const Image& img, const Dec& d) {
    if (ex.size() >= 60) return;
    FailEx e;
    e.rva = rva;
    e.map = d.map;
    e.op = d.opcode;
    const uint8_t* p = nullptr;
    if (file_bytes(img, rva, &p, 8)) std::memcpy(e.b, p, 8);
    ex.push_back(e);
}

std::string mem_text(const Dec& d) {
    if (!d.has_modrm) return "-";
    const int mod = d.modrm >> 6;
    const int rm = d.modrm & 7;
    const int reg = ((d.rex & 4) ? 8 : 0) | ((d.modrm >> 3) & 7);
    const int rm_r = ((d.rex & 1) ? 8 : 0) | rm;
    if (mod == 3) {
        char buf[64];
        std::snprintf(buf, sizeof buf, "reg %s <- %s", reg_name(rm_r), reg_name(reg));
        return buf;
    }
    char disp[32] = {};
    if (d.has_disp32) std::snprintf(disp, sizeof disp, "+0x%X", d.disp32);
    else if (mod == 1) {
        // disp8 is the byte after modrm, or after sib. Recover from the encoded
        // length: not stored. Leave a marker; the bytes on the line are authoritative.
        std::snprintf(disp, sizeof disp, "+disp8");
    }
    if (mod == 0 && rm == 5 && (d.rex & 1) == 0) {
        char buf[64];
        std::snprintf(buf, sizeof buf, "[rip%s] reg=%s", disp, reg_name(reg));
        return buf;
    }
    if (rm == 4) {
        char buf[80];
        std::snprintf(buf, sizeof buf, "[sib%s] reg=%s", disp, reg_name(reg));
        return buf;
    }
    char buf[80];
    std::snprintf(buf, sizeof buf, "[%s%s] reg=%s", reg_name(rm_r), disp, reg_name(reg));
    return buf;
}

bool mentions_r9(const Dec& d, const uint8_t* insn) {
    if ((d.rex & 1) && d.has_modrm) {
        const int rm = d.modrm & 7;
        if (rm != 4 && rm == 1) return true;
    }
    if ((d.rex & 4) && d.has_modrm) {
        const int reg = (d.modrm >> 3) & 7;
        if (reg == 1) return true;
    }
    if (d.map == 0 && d.opcode >= 0xB8 && d.opcode <= 0xBF) {
        const int rd = ((d.rex & 1) ? 8 : 0) | (d.opcode & 7);
        if (rd == 9) return true;
    }
    if (d.map == 0 && d.opcode >= 0x50 && d.opcode <= 0x5F) {
        const int rd = ((d.rex & 1) ? 8 : 0) | (d.opcode & 7);
        if (rd == 9) return true;
    }
    (void)insn;
    return false;
}

bool stack_slot(const Dec& d, const uint8_t* insn, int* disp_out, const char** base_out) {
    if (!d.has_modrm) return false;
    const int mod = d.modrm >> 6;
    const int rm = d.modrm & 7;
    if (mod == 3) return false;
    int disp = 0;
    bool have = false;
    const char* base = "?";
    if (rm == 4) {
        const uint8_t sib = insn[d.modrm_off + 1];
        const int b = ((d.rex & 1) ? 8 : 0) | (sib & 7);
        base = reg_name(b);
        if (d.has_disp32) {
            disp = static_cast<int>(d.disp32);
            have = true;
        } else if (mod == 1) {
            disp = static_cast<int8_t>(insn[d.modrm_off + 2]);
            have = true;
        }
    } else {
        const int b = ((d.rex & 1) ? 8 : 0) | rm;
        base = reg_name(b);
        if (mod == 0 && rm == 5) return false;
        if (d.has_disp32) {
            disp = static_cast<int>(d.disp32);
            have = true;
        } else if (mod == 1) {
            disp = static_cast<int8_t>(insn[d.modrm_off + 1]);
            have = true;
        } else if (mod == 0) {
            disp = 0;
            have = true;
        }
    }
    if (!have) return false;
    if (disp != 0x20 && disp != 0x28 && disp != 0x30 && disp != 0x38 && disp != 0x08 && disp != 0x10 &&
        disp != 0x18) {
        return false;
    }
    *disp_out = disp;
    *base_out = base;
    return true;
}

struct CallSite {
    uint32_t from = 0;
    uint32_t to = 0;
    bool indirect = false;
};

void collect_calls(const Image& img, uint32_t begin, uint32_t end, std::vector<CallSite>& out, int max_insns) {
    const uint32_t need = end - begin;
    const uint32_t avail = raw_avail(img, begin);
    const uint32_t limit = std::min(avail, need);
    const uint8_t* p = nullptr;
    if (limit == 0 || !file_bytes(img, begin, &p, limit)) return;
    uint32_t ip = 0;
    int insns = 0;
    while (ip < limit && ip < need && insns < max_insns) {
        Dec d;
        const int n = decode(p + ip, p + limit, d);
        if (n <= 0) return;
        if (d.map == 0 && d.opcode == 0xE8 && d.rel32) {
            const uint32_t from = begin + ip;
            const uint32_t to = static_cast<uint32_t>(static_cast<int64_t>(from) + n + static_cast<int32_t>(d.imm32));
            out.push_back({from, to, false});
        } else if (d.map == 0 && d.opcode == 0xFF && d.has_modrm && ((d.modrm >> 3) & 7) == 2) {
            out.push_back({begin + ip, 0, true});
        }
        ip += static_cast<uint32_t>(n);
        ++insns;
    }
}

bool reaches(const Image& img, uint32_t begin, uint32_t target, uint32_t end) {
    if (target < begin || target >= end) return false;
    const uint32_t avail = raw_avail(img, begin);
    const uint32_t need = target - begin;
    if (need > avail) return false;
    const uint8_t* p = nullptr;
    if (need == 0) return true;
    if (!file_bytes(img, begin, &p, need)) return false;
    uint32_t ip = 0;
    while (ip < need) {
        Dec d;
        const int n = decode(p + ip, p + (need), d);
        // Allow reading the instruction that might extend past `need` only when ip==need.
        if (n <= 0 || ip + static_cast<uint32_t>(n) > need) {
            // Retry with more bytes so a length mismatch is real, not a truncated buffer.
            const uint32_t more = std::min(avail, (end - begin));
            const uint8_t* q = nullptr;
            if (!file_bytes(img, begin, &q, more)) return false;
            ip = 0;
            while (ip < need) {
                Dec e;
                const int m = decode(q + ip, q + more, e);
                if (m <= 0) return false;
                if (ip + static_cast<uint32_t>(m) > need) return false;
                ip += static_cast<uint32_t>(m);
            }
            return ip == need;
        }
        ip += static_cast<uint32_t>(n);
    }
    return ip == need;
}

void print_disasm(const Image& img, uint32_t begin, uint32_t end, int max_insns) {
    const uint32_t need = end - begin;
    const uint32_t avail = raw_avail(img, begin);
    const uint32_t limit = std::min(avail, need);
    const uint8_t* p = nullptr;
    if (limit == 0 || !file_bytes(img, begin, &p, limit)) {
        std::printf("  (no bytes)\n");
        return;
    }
    uint32_t ip = 0;
    int insns = 0;
    while (ip < limit && insns < max_insns) {
        Dec d;
        const int n = decode(p + ip, p + limit, d);
        if (n <= 0) {
            std::printf("  +%04X  decode-stop\n", ip);
            return;
        }
        std::printf("  +%04X  ", ip);
        for (int i = 0; i < n; ++i) std::printf("%02X ", p[ip + i]);
        for (int i = n; i < 8; ++i) std::printf("   ");
        const char* base = "";
        int slot = 0;
        const bool slot_hit = stack_slot(d, p + ip, &slot, &base);
        const bool r9 = mentions_r9(d, p + ip);
        std::printf("op=%02X map=%u %s", d.opcode, d.map, mem_text(d).c_str());
        if (d.has_imm32) std::printf(" imm=%08X%s", d.imm32, d.rel32 ? " rel" : "");
        if (d.has_imm8) std::printf(" imm8=%02X", d.imm8);
        if (r9) std::printf(" uses-r9");
        if (slot_hit) std::printf(" stack %s+0x%X", base, slot);
        if (d.map == 0 && d.opcode == 0xE8 && d.rel32) {
            const uint32_t from = begin + ip;
            const uint32_t to =
                static_cast<uint32_t>(static_cast<int64_t>(from) + n + static_cast<int32_t>(d.imm32));
            std::printf(" call->exe+%X", to);
        }
        std::printf("\n");
        ip += static_cast<uint32_t>(n);
        ++insns;
        if (ip > need) {
            std::printf("  overshoot\n");
            return;
        }
    }
    if (ip < need) std::printf("  ... %u bytes not printed\n", need - ip);
}

struct RawHit {
    uint32_t site = 0;
    uint32_t value = 0;
    int kind = 0;
    bool imm = false;
    uint32_t start = 0;
    bool unique = false;
    bool from_entry = false;
    bool suffix = false;
    uint32_t func_begin = 0;
    uint32_t func_end = 0;
};

bool local_align(const Image& img, const std::vector<Rf>& funcs, uint32_t site, uint32_t value, bool as_imm,
                 RawHit& h) {
    h = {};
    h.site = site;
    h.value = value;
    std::vector<uint32_t> starts;
    std::vector<bool> imm_field;
    if (site < 14) return false;
    for (int back = 1; back <= 14; ++back) {
        const uint32_t start = site - static_cast<uint32_t>(back);
        const uint8_t* p = nullptr;
        if (!file_bytes(img, start, &p, 15)) continue;
        Dec d;
        if (!decode(p, p + 15, d)) continue;
        bool hit = false;
        bool imm = false;
        if (d.has_imm32 && !d.rel32 && d.imm_off == back && d.imm32 == value) {
            hit = true;
            imm = true;
        }
        if (d.has_disp32 && d.disp_off == back && d.disp32 == value) {
            hit = true;
            imm = false;
        }
        if (!hit) continue;
        starts.push_back(start);
        imm_field.push_back(imm);
    }
    if (starts.size() != 1) {
        h.unique = false;
        return false;
    }
    h.unique = true;
    h.start = starts[0];
    h.imm = imm_field[0];
    if (as_imm && !h.imm) return false;
    if (!as_imm && h.imm) return false;
    const Rf* outer = find_outer(funcs, h.start);
    const Rf* tight = find_tight(funcs, h.start);
    const Rf* fn = outer ? outer : tight;
    if (!fn) return true;
    h.func_begin = fn->begin;
    h.func_end = fn->end;
    h.from_entry = reaches(img, fn->begin, h.start, fn->end);
    const Walk w = walk_range(img, h.start, fn->end, false, nullptr, nullptr, 0);
    h.suffix = w == W_EXACT;
    return true;
}

void search_bytes(const Image& img, const uint8_t* pat, size_t n, std::vector<uint32_t>& hits, size_t cap) {
    for (const Section& s : img.sections) {
        if (!s.executable() || s.raw_size < n) continue;
        const uint8_t* b = img.bytes.data() + s.raw;
        const uint8_t* end = b + s.raw_size;
        for (const uint8_t* p = b; p + n <= end; ++p) {
            if (std::memcmp(p, pat, n) != 0) continue;
            const uint32_t rva = s.va + static_cast<uint32_t>(p - b);
            if (hits.size() < cap) hits.push_back(rva);
            else {
                hits.push_back(0xFFFFFFFF);
                return;
            }
        }
    }
}

const char* st_name(uint8_t st) {
    switch (st) {
        case 0: return "exact";
        case 1: return "short";
        case 2: return "over";
        case 3: return "nobytes";
        default: return "?";
    }
}

void print_old(const Image& img, const std::vector<Rf>& funcs, uint32_t rva, const char* name,
               const uint8_t* head) {
    std::printf("\n-- 2976 RVA %s exe+%X --\n", name, rva);
    std::printf("section=%s file=0x%X\n", section_name(img, rva).c_str(), file_off(img, rva));
    const uint8_t* p = nullptr;
    if (!file_bytes(img, rva, &p, 32)) {
        std::printf("unreadable\n");
        return;
    }
    std::printf("bytes32=");
    print_bytes(p, 32);
    std::printf("\nhead14_match=%s\n", std::memcmp(p, head, 14) == 0 ? "yes" : "NO");
    const Rf* entry = find_entry(funcs, rva);
    const Rf* tight = find_tight(funcs, rva);
    if (entry) std::printf("is_unwind_entry=yes end=%X st=%s\n", entry->end, st_name(entry->st));
    else std::printf("is_unwind_entry=no\n");
    if (tight) {
        std::printf("tight_unwind=%X-%X size=%X st=%s\n", tight->begin, tight->end, tight->end - tight->begin,
                    st_name(tight->st));
    }
    std::printf("first insns:\n");
    print_disasm(img, rva, rva + 32, 8);
}

bool head_eq(const Image& img, uint32_t rva, const uint8_t* head) {
    const uint8_t* p = nullptr;
    if (!file_bytes(img, rva, &p, 14)) return false;
    return std::memcmp(p, head, 14) == 0;
}

struct FuncRow {
    uint32_t begin = 0;
    uint32_t end = 0;
    uint8_t exact = 0;
    int n[K_COUNT] = {};
    int score = 0;
};

void print_function_block(const Image& img, const std::vector<Rf>& funcs, const FuncRow& row, const char* role) {
    std::printf("\n==== %s exe+%X-%X ====\n", role, row.begin, row.end);
    std::printf("section=%s file=0x%X size=0x%X unwind=%s\n", section_name(img, row.begin).c_str(),
                file_off(img, row.begin), row.end - row.begin, row.exact ? "exact" : "not-exact");
    std::printf("tls_hook_target=no\n");
    const uint8_t* p = nullptr;
    if (file_bytes(img, row.begin, &p, 16)) {
        std::printf("entry16=");
        print_bytes(p, 16);
        std::printf("\n");
    }
    std::printf("matches_2976_post_head=%s matches_2976_recv_head=%s\n",
                head_eq(img, row.begin, kPostHead) ? "yes" : "no",
                head_eq(img, row.begin, kRecvHead) ? "yes" : "no");
    std::printf("counts:");
    for (int k = 0; k < K_COUNT; ++k)
        if (row.n[k]) std::printf(" %s=%d", kind_name(k), row.n[k]);
    std::printf("\n");
    std::vector<std::pair<uint32_t, int>> sites;
    Agg tmp = {};
    tmp.end = row.end;
    const Walk w = walk_range(img, row.begin, row.end, true, &tmp, &sites, 0);
    std::printf("rescan=%s\n", w == W_EXACT ? "exact" : w == W_SHORT ? "short" : w == W_OVER ? "over" : "nobytes");
    std::printf("sites:");
    int shown = 0;
    for (const auto& s : sites) {
        if (shown++ > 24) {
            std::printf(" ...");
            break;
        }
        std::printf(" %s@%X", kind_name(s.second), s.first);
    }
    std::printf("\n");

    std::vector<uint32_t> raw_callers;
    for (const Section& sec : img.sections) {
        if (!sec.executable() || sec.raw_size < 5) continue;
        const uint8_t* b = img.bytes.data() + sec.raw;
        for (uint32_t i = 0; i + 5 <= sec.raw_size; ++i) {
            if (b[i] != 0xE8) continue;
            int32_t rel = 0;
            std::memcpy(&rel, b + i + 1, 4);
            const uint32_t from = sec.va + i;
            const uint32_t to = static_cast<uint32_t>(static_cast<int64_t>(from) + 5 + rel);
            if (to == row.begin) raw_callers.push_back(from);
        }
    }
    int verified = 0;
    int tls = 0;
    int text1 = 0;
    int other = 0;
    std::printf("direct_e8_raw=%zu\n", raw_callers.size());
    std::printf("callers:\n");
    int printed = 0;
    for (uint32_t from : raw_callers) {
        const std::string sn = section_name(img, from);
        if (sn == ".tls$") ++tls;
        else if (sn == ".text1") ++text1;
        else ++other;
        const Rf* outer = find_outer(funcs, from);
        bool ok = false;
        if (outer && outer->st == 0) ok = reaches(img, outer->begin, from, outer->end);
        if (ok) ++verified;
        if (printed < 40) {
            std::printf("  exe+%X section=%s unwind=%X-%X boundary=%s\n", from, sn.c_str(),
                        outer ? outer->begin : 0, outer ? outer->end : 0, ok ? "verified" : "unverified");
            ++printed;
        }
    }
    if (raw_callers.size() > 40) std::printf("  ... %zu more callers\n", raw_callers.size() - 40);
    std::printf("caller_sections text1=%d tls=%d other=%d boundary_verified=%d\n", text1, tls, other, verified);

    std::vector<CallSite> callees;
    collect_calls(img, row.begin, row.end, callees, 4000);
    std::printf("callees_in_decoded_prefix:\n");
    int ce = 0;
    for (const CallSite& c : callees) {
        if (ce++ > 80) {
            std::printf("  ... %zu more\n", callees.size() - 80);
            break;
        }
        if (c.indirect) std::printf("  exe+%X indirect-call\n", c.from);
        else std::printf("  exe+%X call exe+%X section=%s\n", c.from, c.to, section_name(img, c.to).c_str());
    }
    std::printf("disassembly:\n");
    const uint32_t size = row.end - row.begin;
    const uint32_t show_end = size > 0x900 ? row.begin + 0x900 : row.end;
    print_disasm(img, row.begin, show_end, 450);
    std::printf("adopted=no\n");
}

}  // namespace

int main(int argc, char** argv) {
    init_tables();
    if (!self_test()) return 1;
    const char* path = argc > 1 ? argv[1] : R"(D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe)";
    Image img;
    if (!load_image(path, img)) {
        std::printf("load failed\n");
        return 1;
    }
    std::printf("BEGIN_SCAN\n");
    std::printf("path=%s\n", path);
    std::printf("size=%zu expected=%llu size_match=%s\n", img.bytes.size(),
                static_cast<unsigned long long>(kExpectedSize),
                img.bytes.size() == kExpectedSize ? "yes" : "NO");
    std::printf("timestamp=%08X expected=%08X timestamp_match=%s\n", img.timestamp, kExpectedTimestamp,
                img.timestamp == kExpectedTimestamp ? "yes" : "NO");
    std::printf("image_base=%llX size_of_image=%X\n", static_cast<unsigned long long>(img.image_base),
                img.size_of_image);
    if (img.timestamp != kExpectedTimestamp || img.bytes.size() != kExpectedSize) {
        std::printf("identity refused\n");
        return 2;
    }
    std::printf("sections:");
    for (const auto& s : img.sections) {
        std::printf(" %s@%X", s.name.c_str(), s.va);
    }
    std::printf("\nexception_dir rva=%X size=%X\n", img.except_rva, img.except_size);

    const uint8_t* ed = nullptr;
    if (img.except_size < 12 || (img.except_size % 12) != 0 || !file_bytes(img, img.except_rva, &ed, img.except_size)) {
        std::printf("exception directory unreadable\n");
        return 1;
    }
    const uint32_t nfunc = img.except_size / 12;
    std::vector<Rf> funcs(nfunc);
    uint32_t inversions = 0;
    for (uint32_t i = 0; i < nfunc; ++i) {
        std::memcpy(&funcs[i].begin, ed + i * 12, 4);
        std::memcpy(&funcs[i].end, ed + i * 12 + 4, 4);
        if (i && funcs[i].begin < funcs[i - 1].begin) ++inversions;
    }
    std::printf("unwind_entries=%u inversions=%u\n", nfunc, inversions);
    if (inversions) {
        std::sort(funcs.begin(), funcs.end(), [](const Rf& a, const Rf& b) { return a.begin < b.begin; });
    }

    std::unordered_map<uint32_t, Agg> aggs;
    aggs.reserve(4096);
    std::unordered_map<std::string, Cov> cov;
    uint32_t hist_pri[256] = {};
    uint32_t hist_0f[256] = {};
    uint32_t hist_map2 = 0;
    uint32_t hist_map3 = 0;
    uint32_t hist_other = 0;
    std::vector<FailEx> fails;
    const auto t0 = std::chrono::steady_clock::now();

    for (uint32_t i = 0; i < nfunc; ++i) {
        Rf& f = funcs[i];
        if ((i % 50000) == 0) {
            std::printf("walk %u/%u\n", i, nfunc);
            std::fflush(stdout);
        }
        Agg local;
        const Walk w = walk_range(img, f.begin, f.end, true, &local, nullptr, 0);
        if (w == W_EXACT) f.st = 0;
        else if (w == W_SHORT) f.st = 1;
        else if (w == W_OVER) f.st = 2;
        else f.st = 3;
        const std::string sn = section_name(img, f.begin);
        Cov& c = cov[sn];
        c.n++;
        if (f.st == 0) c.exact++;
        else if (f.st == 1) c.shrt++;
        else if (f.st == 2) c.over++;
        else c.nobytes++;
        bool any = false;
        for (int k = 0; k < K_COUNT; ++k)
            if (local.n[k]) any = true;
        if (any && f.st == 0) {
            local.end = f.end;
            local.exact = 1;
            aggs[f.begin] = local;
        }
        if (f.st == 1) {
            const uint32_t avail = raw_avail(img, f.begin);
            const uint32_t need = f.end > f.begin ? f.end - f.begin : 0;
            const uint32_t limit = std::min(avail, need);
            const uint8_t* p = nullptr;
            if (limit && file_bytes(img, f.begin, &p, limit)) {
                uint32_t ip = 0;
                while (ip < limit) {
                    Dec d;
                    const int n = decode(p + ip, p + limit, d);
                    if (n <= 0) {
                        if (d.map == 0) hist_pri[d.opcode]++;
                        else if (d.map == 1) hist_0f[d.opcode]++;
                        else if (d.map == 2) hist_map2++;
                        else if (d.map == 3) hist_map3++;
                        else hist_other++;
                        if (fails.size() < 40) note_fail(fails, f.begin + ip, img, d);
                        break;
                    }
                    ip += static_cast<uint32_t>(n);
                }
            }
        }
    }
    const auto t1 = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::printf("\ncoverage_ms=%lld\n", static_cast<long long>(ms));
    for (const auto& kv : cov) {
        const Cov& c = kv.second;
        const double pct = c.n ? (100.0 * static_cast<double>(c.exact) / static_cast<double>(c.n)) : 0;
        std::printf("coverage section=%s n=%llu exact=%llu (%.2f%%) short=%llu over=%llu nobytes=%llu\n",
                    kv.first.c_str(), static_cast<unsigned long long>(c.n), static_cast<unsigned long long>(c.exact),
                    pct, static_cast<unsigned long long>(c.shrt), static_cast<unsigned long long>(c.over),
                    static_cast<unsigned long long>(c.nobytes));
    }
    std::printf("fail_histogram primary:");
    for (int i = 0; i < 256; ++i)
        if (hist_pri[i]) std::printf(" %02X=%u", i, hist_pri[i]);
    std::printf("\nfail_histogram 0F:");
    for (int i = 0; i < 256; ++i)
        if (hist_0f[i]) std::printf(" %02X=%u", i, hist_0f[i]);
    std::printf("\nfail_map2=%u fail_map3=%u fail_other=%u\n", hist_map2, hist_map3, hist_other);
    std::printf("fail_examples:\n");
    for (const FailEx& e : fails) {
        std::printf("  exe+%X map=%u op=%02X ", e.rva, e.map, e.op);
        print_bytes(e.b, 8);
        std::printf("\n");
    }

    std::printf("\nexact_hit_totals:");
    int totals[K_COUNT] = {};
    for (const auto& kv : aggs) {
        for (int k = 0; k < K_COUNT; ++k) totals[k] += kv.second.n[k];
    }
    for (int k = 0; k < K_COUNT; ++k) std::printf(" %s=%d", kind_name(k), totals[k]);
    std::printf("\nexact_functions_with_any_hit=%zu\n", aggs.size());

    print_old(img, funcs, kOldPost, "ClientPostPacket", kPostHead);
    print_old(img, funcs, kOldRecv, "ClientOnRecvPacket", kRecvHead);

    std::vector<uint32_t> post_hits;
    std::vector<uint32_t> recv_hits;
    std::vector<uint32_t> pump_hits;
    std::vector<uint32_t> pump7_hits;
    search_bytes(img, kPostHead, 14, post_hits, 16);
    search_bytes(img, kRecvHead, 14, recv_hits, 16);
    search_bytes(img, kPump, 39, pump_hits, 16);
    search_bytes(img, kPump, 7, pump7_hits, 16);
    auto report_pats = [](const char* name, const std::vector<uint32_t>& h, const Image& image) {
        const bool trunc = !h.empty() && h.back() == 0xFFFFFFFF;
        const size_t n = trunc ? h.size() - 1 : h.size();
        std::printf("\nraw_pattern %s stored=%zu truncated=%s\n", name, n, trunc ? "yes" : "no");
        for (size_t i = 0; i < n && i < 8; ++i) {
            std::printf("  exe+%X section=%s\n", h[i], section_name(image, h[i]).c_str());
        }
        std::printf("prologue_or_pump_is_not_a_candidate=yes\n");
    };
    report_pats("post14", post_hits, img);
    report_pats("recv14", recv_hits, img);
    report_pats("pump39", pump_hits, img);
    report_pats("pump7", pump7_hits, img);

    std::printf("\n-- pump boundary --\n");
    for (uint32_t rva : pump_hits) {
        if (rva == 0xFFFFFFFF) break;
        const Rf* outer = find_outer(funcs, rva);
        const Rf* tight = find_tight(funcs, rva);
        std::printf("pump exe+%X section=%s\n", rva, section_name(img, rva).c_str());
        if (outer) {
            const bool reached = outer->st == 0 || reaches(img, outer->begin, rva, outer->end);
            std::printf("  outer=%X-%X st=%s reached_from_entry=%s\n", outer->begin, outer->end, st_name(outer->st),
                        reached ? "yes" : "no");
        }
        if (tight && (!outer || tight->begin != outer->begin)) {
            std::printf("  tight=%X-%X st=%s\n", tight->begin, tight->end, st_name(tight->st));
        }
        const Rf* endf = outer ? outer : tight;
        if (endf) {
            const Walk suf = walk_range(img, rva, endf->end, false, nullptr, nullptr, 0);
            std::printf("  suffix_to_unwind_end=%s\n", suf == W_EXACT ? "exact" : "not-exact");
            std::printf("  pump window disasm:\n");
            const uint32_t window_end = std::min(endf->end, rva + 0x180);
            print_disasm(img, rva, window_end, 80);
            std::vector<CallSite> calls;
            collect_calls(img, rva, window_end, calls, 80);
            std::printf("  calls in pump window:\n");
            for (const CallSite& c : calls) {
                if (c.indirect) std::printf("    exe+%X indirect\n", c.from);
                else std::printf("    exe+%X -> exe+%X\n", c.from, c.to);
            }
        }
    }

    // Raw rare immediates, locally confirmed when the containing function was not exact.
    struct Pat {
        uint32_t value;
        int kind;
        bool imm;
        uint8_t b[4];
    };
    const Pat pats[] = {
        {0x8030, K_8030, true, {0x30, 0x80, 0x00, 0x00}}, {0x8028, K_8028, false, {0x28, 0x80, 0x00, 0x00}},
        {0x800A, K_800A, false, {0x0A, 0x80, 0x00, 0x00}}, {0x8014, K_8014, false, {0x14, 0x80, 0x00, 0x00}},
        {0x7FD7, K_7FD7, true, {0xD7, 0x7F, 0x00, 0x00}}, {0x7FDA, K_7FDA, true, {0xDA, 0x7F, 0x00, 0x00}},
        {0x7FC6, K_7FC6, true, {0xC6, 0x7F, 0x00, 0x00}},
    };
    std::printf("\n-- raw structural constants (local alignment) --\n");
    for (const Pat& pat : pats) {
        std::vector<uint32_t> sites;
        uint64_t raw_n = 0;
        bool trunc = false;
        for (const Section& s : img.sections) {
            if (!s.executable() || s.raw_size < 4) continue;
            const uint8_t* b = img.bytes.data() + s.raw;
            for (uint32_t i = 0; i + 4 <= s.raw_size; ++i) {
                if (std::memcmp(b + i, pat.b, 4) != 0) continue;
                ++raw_n;
                if (sites.size() < 4000) sites.push_back(s.va + i);
                else trunc = true;
            }
        }
        int unique_ok = 0;
        int suffix_ok = 0;
        int entry_ok = 0;
        std::printf("value=%s raw=%llu stored=%zu truncated=%s\n", kind_name(pat.kind),
                    static_cast<unsigned long long>(raw_n), sites.size(), trunc ? "yes" : "no");
        int listed = 0;
        for (uint32_t site : sites) {
            RawHit h;
            if (!local_align(img, funcs, site, pat.value, pat.imm, h)) continue;
            if (!h.unique) continue;
            ++unique_ok;
            if (h.suffix) ++suffix_ok;
            if (h.from_entry) ++entry_ok;
            const bool already = aggs.count(h.func_begin) && aggs[h.func_begin].exact && aggs[h.func_begin].n[pat.kind];
            if (already) continue;
            if (!(h.suffix || h.from_entry)) continue;
            if (listed < 12) {
                std::printf("  extra site=%X start=%X func=%X-%X suffix=%s from_entry=%s field=%s\n", h.site, h.start,
                            h.func_begin, h.func_end, h.suffix ? "yes" : "no", h.from_entry ? "yes" : "no",
                            h.imm ? "imm" : "disp");
                ++listed;
            }
            if (h.func_begin && (h.suffix || h.from_entry)) {
                Agg& a = aggs[h.func_begin];
                a.end = h.func_end;
                a.exact = h.from_entry && find_entry(funcs, h.func_begin) && find_entry(funcs, h.func_begin)->st == 0
                              ? 1
                              : 0;
                // Count the extra confirmation without double-counting an exact walk.
                if (!a.n[pat.kind]) a.n[pat.kind] = 1;
            }
        }
        std::printf("  locally_unique=%d suffix_exact=%d from_entry=%d\n", unique_ok, suffix_ok, entry_ok);
    }

    std::vector<FuncRow> rows;
    for (const auto& kv : aggs) {
        const bool rare = kv.second.n[K_8030] || kv.second.n[K_8028] || kv.second.n[K_800A] || kv.second.n[K_8014] ||
                          kv.second.n[K_7FD7] || kv.second.n[K_7FDA] || kv.second.n[K_7FC6];
        if (!rare) continue;
        FuncRow row;
        row.begin = kv.first;
        row.end = kv.second.end;
        row.exact = kv.second.exact;
        for (int k = 0; k < K_COUNT; ++k) row.n[k] = kv.second.n[k];
        const bool post_pair = row.n[K_8030] && (row.n[K_8028] || row.n[K_800A] || row.n[K_8014]);
        const bool recv = row.n[K_7FD7] || row.n[K_7FDA];
        row.score = (post_pair ? 100 : 0) + (row.n[K_8030] ? 40 : 0) + (recv ? 50 : 0) + (row.exact ? 10 : 0) +
                    row.n[K_8028] + row.n[K_800A];
        rows.push_back(row);
    }
    std::sort(rows.begin(), rows.end(), [](const FuncRow& a, const FuncRow& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.begin < b.begin;
    });
    std::printf("\ninteresting_functions=%zu\n", rows.size());
    for (const FuncRow& row : rows) {
        std::printf("  exe+%X-%X exact=%u score=%d", row.begin, row.end, row.exact, row.score);
        for (int k = 0; k < K_COUNT; ++k)
            if (row.n[k]) std::printf(" %s=%d", kind_name(k), row.n[k]);
        std::printf(" section=%s\n", section_name(img, row.begin).c_str());
    }

    int post_pairs = 0;
    int recv_funcs = 0;
    for (const FuncRow& row : rows) {
        if (row.n[K_8030] && (row.n[K_8028] || row.n[K_800A] || row.n[K_8014])) ++post_pairs;
        if (row.n[K_7FD7] || row.n[K_7FDA]) ++recv_funcs;
    }
    std::printf("post_strong_pairs=%d recv_limit_funcs=%d\n", post_pairs, recv_funcs);

    int printed_blocks = 0;
    for (const FuncRow& row : rows) {
        const bool post_pair = row.n[K_8030] && (row.n[K_8028] || row.n[K_800A] || row.n[K_8014]);
        const bool recv = row.n[K_7FD7] || row.n[K_7FDA];
        const bool partial = row.n[K_8030] || row.n[K_8028] || row.n[K_800A];
        if (!(post_pair || recv || (partial && printed_blocks < 8))) continue;
        if (printed_blocks >= 8) break;
        const char* role = post_pair ? "POST-LIKE" : recv ? "RECV-LIKE" : "PARTIAL";
        // Caller scan of a whole image is expensive; do it only for the ranked rows.
        print_function_block(img, funcs, row, role);
        ++printed_blocks;
    }

    std::printf("\n-- names --\n");
    int names = 0;
    const char* keys[] = {"ClientNet", "NwSession", "NwVirtual", "TrocTr", "PostPacket", "OnRecvPacket"};
    const uint8_t* b = img.bytes.data();
    const size_t n = img.bytes.size();
    for (size_t i = 0; i + 8 < n && names < 80; ++i) {
        bool key = false;
        const char* which = nullptr;
        for (const char* k : keys) {
            const size_t kn = std::strlen(k);
            if (i + kn < n && std::memcmp(b + i, k, kn) == 0) {
                key = true;
                which = k;
                break;
            }
        }
        if (!key) continue;
        size_t s = i;
        while (s > 0 && s + 16 > i && b[s - 1] >= 0x20 && b[s - 1] < 0x7F) --s;
        size_t e = i;
        while (e < n && e < i + 120 && b[e] >= 0x20 && b[e] < 0x7F) ++e;
        std::string text(reinterpret_cast<const char*>(b + s), e - s);
        std::printf("  file+0x%zX key=%s text=%s\n", s, which, text.c_str());
        ++names;
        i = e;
    }
    std::printf("name_hits_printed=%d\n", names);
    std::printf("END_SCAN adopted=no hooks=no guard=closed\n");
    return 0;
}
