// Read-only scanner for Crimson Desert 1.0.0.3576.
// Finds instruction shapes that touch a vec at +0x90 and a RIP-relative add or
// subtract. It does not attach to the game, adopt an offset, or install a hook.
// Build and run with tools/probe-3576/run_scan.ps1. Output is evidence, not a
// player-state table.

#include <algorithm>
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
constexpr uint32_t kOldChunkOrigin = 0x6D56500;
constexpr uint32_t kOldStaticPos = 0x6D77858;
constexpr uint32_t kOldStaticPosZ = 0x6D77860;
constexpr int kMaxSitesPrinted = 40;
constexpr int kWindow = 48;

struct Section {
    std::string name;
    uint32_t va = 0;
    uint32_t virtual_size = 0;
    uint32_t raw = 0;
    uint32_t raw_size = 0;
    uint32_t chars = 0;
    bool executable() const { return (chars & 0x20000000u) != 0; }
    bool writable() const { return (chars & 0x80000000u) != 0; }
};

struct Image {
    std::vector<uint8_t> bytes;
    uint32_t timestamp = 0;
    uint32_t size_of_image = 0;
    uint64_t image_base = 0;
    std::vector<Section> sections;
};

struct Site {
    uint32_t rva = 0;
    uint32_t target = 0;
    uint32_t gap = 0;
    char direction = '?';  // 'L' local-to-world (load +0x90 then add), 'W' world-to-local (sub then store +0x90)
    bool phys_2c0 = false;
    bool inner_140 = false;
    uint8_t store_modrm = 0;
    uint8_t arith_modrm = 0;
    uint8_t vex_pp = 0;
    std::string section;
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
    if (n < 0x100 || img.bytes[0] != 'M' || img.bytes[1] != 'Z') return false;
    uint32_t e_lfanew = 0;
    std::memcpy(&e_lfanew, img.bytes.data() + 0x3C, 4);
    if (e_lfanew + 0x108 > n) return false;
    if (std::memcmp(img.bytes.data() + e_lfanew, "PE\0\0", 4) != 0) return false;
    std::memcpy(&img.timestamp, img.bytes.data() + e_lfanew + 8, 4);
    uint16_t magic = 0;
    std::memcpy(&magic, img.bytes.data() + e_lfanew + 24, 2);
    if (magic != 0x20B) return false;
    std::memcpy(&img.image_base, img.bytes.data() + e_lfanew + 24 + 24, 8);
    std::memcpy(&img.size_of_image, img.bytes.data() + e_lfanew + 24 + 56, 4);
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
        const uint32_t span = s.virtual_size > s.raw_size ? s.virtual_size : s.raw_size;
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

void print_bytes(const uint8_t* p, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) std::printf("%02X ", p[i]);
}

void print_floats(const uint8_t* p, uint32_t n) {
    for (uint32_t i = 0; i + 4 <= n; i += 4) {
        float f = 0;
        std::memcpy(&f, p + i, 4);
        std::printf(" %.9g", static_cast<double>(f));
    }
}

const char* reg_name(int r) {
    static const char* names[] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
                                  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"};
    if (r < 0 || r > 15) return "?";
    return names[r];
}

bool rip_modrm(uint8_t modrm) { return (modrm & 0xC7) == 0x05; }

bool disp32_modrm(uint8_t modrm) {
    const int mod = modrm >> 6;
    const int rm = modrm & 7;
    return mod == 2 && rm != 4;
}

int base_reg(uint8_t modrm) { return modrm & 7; }

struct Tok {
    bool wild = false;
    uint8_t value = 0;
};

std::vector<Tok> parse_tokens(const char* pat) {
    std::vector<Tok> tokens;
    const char* s = pat;
    while (*s) {
        while (*s == ' ') ++s;
        if (!*s) break;
        Tok tok;
        if (s[0] == '?' && s[1] == '?') {
            tok.wild = true;
            s += 2;
        } else {
            unsigned v = 0;
            if (std::sscanf(s, "%02x", &v) != 1) break;
            tok.value = static_cast<uint8_t>(v);
            s += 2;
        }
        tokens.push_back(tok);
    }
    return tokens;
}

bool match_toks(const uint8_t* p, const uint8_t* end, const std::vector<Tok>& tokens) {
    if (static_cast<size_t>(end - p) < tokens.size()) return false;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (!tokens[i].wild && p[i] != tokens[i].value) return false;
    }
    return true;
}

int32_t read_i32(const uint8_t* p) {
    int32_t v = 0;
    std::memcpy(&v, p, 4);
    return v;
}

bool in_image(const Image& img, uint32_t rva) { return rva < img.size_of_image; }

uint32_t rip_target(uint32_t insn_rva, uint32_t insn_len, int32_t disp) {
    return static_cast<uint32_t>(static_cast<int64_t>(insn_rva) + insn_len + disp);
}

bool near_disp(const Image& img, uint32_t rva, uint32_t look_back, uint32_t disp_le) {
    const uint8_t* p = nullptr;
    const Section* s = section_at(img, rva);
    if (!s || !s->executable()) return false;
    uint32_t start = rva > look_back ? rva - look_back : s->va;
    if (start < s->va) start = s->va;
    const uint32_t len = rva - start;
    if (!file_bytes(img, start, &p, len)) return false;
    const uint8_t b0 = static_cast<uint8_t>(disp_le & 0xFF);
    const uint8_t b1 = static_cast<uint8_t>((disp_le >> 8) & 0xFF);
    const uint8_t b2 = static_cast<uint8_t>((disp_le >> 16) & 0xFF);
    const uint8_t b3 = static_cast<uint8_t>((disp_le >> 24) & 0xFF);
    for (uint32_t i = 0; i + 5 < len; ++i) {
        if (p[i] == b0 && p[i + 1] == b1 && p[i + 2] == b2 && p[i + 3] == b3) {
            const uint8_t modrm = p[i - 1];
            if (disp32_modrm(modrm)) return true;
        }
    }
    return false;
}

const char* pp_name(uint8_t pp) {
    switch (pp & 3) {
        case 0: return "ps";
        case 1: return "pd";
        case 2: return "ss";
        case 3: return "sd";
    }
    return "?";
}

void dump_target(const Image& img, uint32_t target) {
    const uint8_t* p = nullptr;
    std::printf("    target exe+0x%X section %s", target, section_name(img, target).c_str());
    const Section* s = section_at(img, target);
    if (s) {
        std::printf(" %s%s", s->executable() ? "exec " : "", s->writable() ? "writable" : "not-writable");
    }
    if (file_bytes(img, target, &p, 16)) {
        std::printf("\n    file ");
        print_bytes(p, 16);
        std::printf("\n    file-f32");
        print_floats(p, 16);
        std::printf("\n");
    } else {
        std::printf("\n    file bytes unavailable (not file-backed)\n");
    }
    if (target == kOldChunkOrigin || target == kOldStaticPos || target == kOldStaticPosZ) {
        std::printf("    THIS IS A 2976 GLOBAL RVA. It is not adopted for 3576.\n");
    }
}

struct Agg {
    int sites = 0;
    int with_2c0 = 0;
    int local_to_world = 0;
    int world_to_local = 0;
};

void print_site(const Image& img, const Site& site) {
    std::printf("  exe+0x%X %s %s vex.pp=%u (%s) gap %u", site.rva, site.section.c_str(),
                site.direction == 'L' ? "load+0x90 then add [rip]" : "sub [rip] then store +0x90",
                site.vex_pp, pp_name(site.vex_pp), site.gap);
    if (site.phys_2c0) std::printf(" near+0x2C0");
    if (site.inner_140) std::printf(" near+0x140");
    std::printf("\n    arith modrm 0x%02X  mem modrm 0x%02X base %s\n", site.arith_modrm,
                site.store_modrm, reg_name(base_reg(site.store_modrm)));
    dump_target(img, site.target);
}

bool self_test() {
    // vsubps xmm0, xmm0, [rip+0x10]; vmovups [rcx+0x90], xmm0
    const uint8_t buf[] = {0xC5, 0xF8, 0x5C, 0x05, 0x10, 0x00, 0x00, 0x00,
                           0xC5, 0xF8, 0x11, 0x81, 0x90, 0x00, 0x00, 0x00};
    if (!rip_modrm(buf[3]) || !disp32_modrm(buf[11])) return false;
    if (base_reg(buf[11]) != 1) return false;
    const uint32_t target = rip_target(0x1000, 8, read_i32(buf + 4));
    if (target != 0x1018) return false;
    const auto tokens = parse_tokens("C5 F8 5C 05 ?? ?? ?? ?? C5 F8 11 81 90 00 00 00");
    if (tokens.size() != 16 || !match_toks(buf, buf + sizeof buf, tokens)) return false;
    return true;
}

void scan_exact(const Image& img) {
    struct Named {
        const char* name;
        std::vector<Tok> tokens;
        int rip_disp_at;  // offset of the disp32 inside the match, or -1
        int rip_insn_len;
        int rip_insn_start;
    };
    // Lengths are the encoded instruction that contains the RIP displacement.
    const Named pats[] = {
        {"2976 static XY then Z", parse_tokens("C5 FB 11 3D ?? ?? ?? ?? 44 89 3D ?? ?? ?? ??"), 4, 8, 0},
        {"2976 world-minus-offset store to [rcx+0x90]",
         parse_tokens("48 8B 8B ?? ?? 00 00 C5 FB 10 44 24 ?? C4 E3 79 21 44 24 ?? 20 C5 F8 5C 05 ?? ?? ?? ?? C5 F8 11 81 90 00 00 00"),
         25, 8, 21},
        {"2976 short worldOffset neighbour",
         parse_tokens("C5 ?? ?? ?? ?? ?? ?? ?? C5 ?? 11 ?? 90 00 00 00 E8 ?? ?? ?? ?? C5"), -1, 0, 0},
        {"2976 SSE local delta through r13",
         parse_tokens("0F 28 C6 F3 45 0F 5C C8 41 0F 58 45 00 41 0F 11 45 00"), -1, 0, 0},
        {"lea r13, [rbx+0x90]", parse_tokens("4C 8D AB 90 00 00 00"), -1, 0, 0},
        {"vmovups [rcx+0x90], xmm0", parse_tokens("C5 F8 11 81 90 00 00 00"), -1, 0, 0},
        {"vmovups xmm0, [rcx+0x90]", parse_tokens("C5 F8 10 81 90 00 00 00"), -1, 0, 0},
    };

    std::printf("\n== Exact 2976 shapes (wildcard). A miss means that byte string is not in this file.\n");
    for (const auto& pat : pats) {
        std::vector<uint32_t> hits;
        for (const auto& sec : img.sections) {
            if (!sec.executable() || sec.raw_size < 16) continue;
            const uint8_t* base = img.bytes.data() + sec.raw;
            const uint8_t* end = base + sec.raw_size;
            for (uint32_t i = 0; i + pat.tokens.size() <= sec.raw_size; ++i) {
                if (!match_toks(base + i, end, pat.tokens)) continue;
                hits.push_back(sec.va + i);
                if (hits.size() >= 30) break;
            }
            if (hits.size() >= 30) break;
        }
        std::printf("\n%s\n  matches shown: %zu%s\n", pat.name, hits.size(),
                    hits.size() >= 30 ? " (capped at 30)" : "");
        for (uint32_t rva : hits) {
            std::printf("  exe+0x%X %s\n", rva, section_name(img, rva).c_str());
            if (pat.rip_disp_at < 0) continue;
            const uint8_t* p = nullptr;
            if (!file_bytes(img, rva, &p, 48)) continue;
            const int32_t disp = read_i32(p + pat.rip_disp_at);
            const uint32_t target = rip_target(rva + static_cast<uint32_t>(pat.rip_insn_start),
                                                static_cast<uint32_t>(pat.rip_insn_len), disp);
            if (std::strcmp(pat.name, "2976 static XY then Z") == 0) {
                const int32_t zdisp = read_i32(p + 11);
                const uint32_t ztarget = rip_target(rva + 8, 7, zdisp);
                std::printf("    XY ");
                dump_target(img, target);
                std::printf("    Z  ");
                dump_target(img, ztarget);
            } else if (in_image(img, target)) {
                dump_target(img, target);
            } else {
                std::printf("    RIP target 0x%X is outside the image\n", target);
            }
        }
    }
}

bool parse_vex_arith_or_mov(const uint8_t* p, const uint8_t* end, uint32_t* len, uint8_t* opcode,
                            uint8_t* pp, uint8_t* modrm, int32_t* disp, bool* is_rip) {
    uint32_t op_at = 0;
    if (p + 8 <= end && p[0] == 0xC5) {
        *pp = p[1] & 3;
        op_at = 2;
    } else if (p + 9 <= end && p[0] == 0xC4 && (p[1] & 0x1F) == 1) {
        *pp = p[2] & 3;
        op_at = 3;
    } else {
        return false;
    }
    *opcode = p[op_at];
    if (*opcode != 0x10 && *opcode != 0x11 && *opcode != 0x58 && *opcode != 0x5C) return false;
    *modrm = p[op_at + 1];
    const uint32_t disp_at = op_at + 2;
    if (rip_modrm(*modrm) || disp32_modrm(*modrm)) {
        if (p + disp_at + 4 > end) return false;
        *disp = read_i32(p + disp_at);
        *len = disp_at + 4;
        *is_rip = rip_modrm(*modrm);
        return true;
    }
    return false;
}

void scan_tight(const Image& img) {
    std::printf("\n== Tight pairs: VEX vector op on +0x90 beside a RIP-relative add or sub.\n");
    std::printf("Direction L reads +0x90 and adds a RIP operand. Direction W subtracts a RIP operand and stores +0x90.\n");
    std::printf("These are candidates. None is an adopted address.\n");

    std::vector<Site> sites;
    std::unordered_map<uint32_t, Agg> by_target;
    uint64_t load90 = 0, store90 = 0;

    for (const auto& sec : img.sections) {
        if (!sec.executable() || sec.raw_size < 24) continue;
        const uint8_t* base = img.bytes.data() + sec.raw;
        const uint32_t n = sec.raw_size;
        for (uint32_t i = 0; i + 16 < n; ++i) {
            uint32_t len = 0;
            uint8_t opcode = 0, pp = 0, modrm = 0;
            int32_t disp = 0;
            bool is_rip = false;
            if (!parse_vex_arith_or_mov(base + i, base + n, &len, &opcode, &pp, &modrm, &disp, &is_rip))
                continue;
            if (is_rip || disp != 0x90) continue;
            if (opcode == 0x10) ++load90;
            if (opcode == 0x11) ++store90;

            const uint32_t rva = sec.va + i;
            // Look backward up to kWindow for a RIP sub (world-to-local) when this is a store.
            if (opcode == 0x11) {
                const uint32_t back = i > static_cast<uint32_t>(kWindow) ? i - kWindow : 0;
                for (uint32_t j = back; j + 8 <= i; ++j) {
                    uint32_t alen = 0;
                    uint8_t aop = 0, app = 0, am = 0;
                    int32_t ad = 0;
                    bool arip = false;
                    if (!parse_vex_arith_or_mov(base + j, base + n, &alen, &aop, &app, &am, &ad, &arip))
                        continue;
                    if (!arip || aop != 0x5C) continue;
                    if (j + alen > i) continue;
                    Site site;
                    site.rva = sec.va + j;
                    site.gap = i - (j + alen);
                    site.direction = 'W';
                    site.vex_pp = app;
                    site.arith_modrm = am;
                    site.store_modrm = modrm;
                    site.target = rip_target(site.rva, alen, ad);
                    site.section = sec.name;
                    site.phys_2c0 = near_disp(img, rva, 64, 0x2C0);
                    site.inner_140 = near_disp(img, rva, 96, 0x140);
                    if (!in_image(img, site.target)) continue;
                    sites.push_back(site);
                    auto& agg = by_target[site.target];
                    ++agg.sites;
                    ++agg.world_to_local;
                    if (site.phys_2c0) ++agg.with_2c0;
                }
            }
            // Look forward up to kWindow for a RIP add (local-to-world) when this is a load.
            if (opcode == 0x10) {
                const uint32_t end = std::min(n, i + static_cast<uint32_t>(kWindow) + 8);
                for (uint32_t j = i + len; j + 8 <= end; ++j) {
                    uint32_t alen = 0;
                    uint8_t aop = 0, app = 0, am = 0;
                    int32_t ad = 0;
                    bool arip = false;
                    if (!parse_vex_arith_or_mov(base + j, base + n, &alen, &aop, &app, &am, &ad, &arip))
                        continue;
                    if (!arip || aop != 0x58) continue;
                    Site site;
                    site.rva = rva;
                    site.gap = j - (i + len);
                    site.direction = 'L';
                    site.vex_pp = app;
                    site.arith_modrm = am;
                    site.store_modrm = modrm;
                    site.target = rip_target(sec.va + j, alen, ad);
                    site.section = sec.name;
                    site.phys_2c0 = near_disp(img, rva, 64, 0x2C0);
                    site.inner_140 = near_disp(img, rva, 96, 0x140);
                    if (!in_image(img, site.target)) continue;
                    sites.push_back(site);
                    auto& agg = by_target[site.target];
                    ++agg.sites;
                    ++agg.local_to_world;
                    if (site.phys_2c0) ++agg.with_2c0;
                }
            }
        }
        std::printf("section %s load[+0x90] %llu store[+0x90] running totals %llu %llu\n", sec.name.c_str(),
                    static_cast<unsigned long long>(load90), static_cast<unsigned long long>(store90),
                    static_cast<unsigned long long>(load90), static_cast<unsigned long long>(store90));
    }

    std::printf("\nTight pair sites: %zu\n", sites.size());
    std::printf("Distinct RIP targets: %zu\n", by_target.size());

    std::vector<std::pair<uint32_t, Agg>> ranked(by_target.begin(), by_target.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.second.with_2c0 != b.second.with_2c0) return a.second.with_2c0 > b.second.with_2c0;
        return a.second.sites > b.second.sites;
    });

    std::printf("\n-- RIP targets ranked by nearby +0x2C0 links, then by site count\n");
    int shown_targets = 0;
    for (const auto& [target, agg] : ranked) {
        if (shown_targets >= 25) break;
        const Section* s = section_at(img, target);
        std::printf("exe+0x%X sites %d  L %d  W %d  near+0x2C0 %d  %s %s%s\n", target, agg.sites,
                    agg.local_to_world, agg.world_to_local, agg.with_2c0, section_name(img, target).c_str(),
                    s && s->executable() ? "exec " : "", s && s->writable() ? "writable" : "not-writable");
        const uint8_t* p = nullptr;
        if (file_bytes(img, target, &p, 16)) {
            std::printf("  file ");
            print_bytes(p, 16);
            std::printf("\n  file-f32");
            print_floats(p, 16);
            std::printf("\n");
        }
        if (target == kOldChunkOrigin || target == kOldStaticPos || target == kOldStaticPosZ)
            std::printf("  matches a 2976 global RVA. Not adopted.\n");
        ++shown_targets;
    }

    std::printf("\n-- Example sites (at most %d). Prefer a +0x2C0 neighbour.\n", kMaxSitesPrinted);
    std::vector<Site> examples = sites;
    std::sort(examples.begin(), examples.end(), [](const Site& a, const Site& b) {
        if (a.phys_2c0 != b.phys_2c0) return a.phys_2c0 > b.phys_2c0;
        return a.rva < b.rva;
    });
    int printed = 0;
    uint32_t last_rva = 0xFFFFFFFF;
    for (const auto& site : examples) {
        if (printed >= kMaxSitesPrinted) break;
        if (site.rva == last_rva) continue;
        last_rva = site.rva;
        print_site(img, site);
        ++printed;
    }

    int old_hits = 0;
    for (const auto& [target, agg] : by_target) {
        if (target == kOldChunkOrigin || target == kOldStaticPos || target == kOldStaticPosZ) {
            std::printf("OLD GLOBAL USED exe+0x%X sites %d\n", target, agg.sites);
            ++old_hits;
        }
    }
    std::printf("\n2976 origin/static RVAs among tight targets: %d\n", old_hits);
}

void scan_strings(const Image& img) {
    std::printf("\n== Strings and RTTI names. Presence is not an address.\n");
    const char* needles[] = {"worldOffset", "WorldOffset", "chunkOrigin", "ChunkOrigin",
                             "hknpWorld",    "BroadPhase",  "world shift",  "WorldShift"};
    for (const char* needle : needles) {
        const size_t n = std::strlen(needle);
        int count = 0;
        for (size_t i = 0; i + n < img.bytes.size(); ++i) {
            if (std::memcmp(img.bytes.data() + i, needle, n) == 0) ++count;
        }
        std::printf("  ascii \"%s\": %d\n", needle, count);
    }

    std::printf("\nRTTI names containing hknpWorld, Shift, Chunk, Origin, BroadPhase, or WorldOffset:\n");
    int shown = 0;
    int found = 0;
    const auto& b = img.bytes;
    for (size_t i = 0; i + 8 < b.size(); ++i) {
        if (b[i] != '.' || b[i + 1] != '?' || b[i + 2] != 'A' || b[i + 3] != 'V') continue;
        size_t j = i;
        while (j + 1 < b.size() && b[j] != '@' && j - i < 180) ++j;
        if (j + 1 >= b.size() || b[j] != '@' || b[j + 1] != '@') continue;
        std::string name(reinterpret_cast<const char*>(b.data() + i), j + 2 - i);
        const bool keep = name.find("hknpWorld") != std::string::npos ||
                          name.find("Shift") != std::string::npos ||
                          name.find("Chunk") != std::string::npos ||
                          name.find("Origin") != std::string::npos ||
                          name.find("BroadPhase") != std::string::npos ||
                          name.find("WorldOffset") != std::string::npos;
        if (!keep) continue;
        ++found;
        if (shown < 60) {
            std::printf("  file+0x%zX %s\n", i, name.c_str());
            ++shown;
        }
    }
    std::printf("matching RTTI names: %d (showed %d)\n", found, shown);
}

void dump_old_slots(const Image& img) {
    std::printf("\n== File bytes at the 2976 globals. These RVAs are a comparison, not 3576 candidates.\n");
    for (uint32_t rva : {kOldChunkOrigin, kOldStaticPos, kOldStaticPosZ}) {
        std::printf("exe+0x%X\n", rva);
        dump_target(img, rva);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (!self_test()) {
        std::fprintf(stderr, "self-test failed\n");
        return 2;
    }
    std::printf("self-test passed\n");
    if (argc < 2) {
        std::fprintf(stderr, "usage: scan_local_to_world <CrimsonDesert.exe>\n");
        return 2;
    }
    Image img;
    if (!load_image(argv[1], img)) {
        std::fprintf(stderr, "cannot parse PE\n");
        return 1;
    }
    std::printf("file %s\n", argv[1]);
    std::printf("timestamp 0x%08X %s\n", img.timestamp,
                img.timestamp == kExpectedTimestamp ? "expected 3576" : "UNEXPECTED");
    std::printf("image base 0x%llX size 0x%X bytes %zu\n",
                static_cast<unsigned long long>(img.image_base), img.size_of_image, img.bytes.size());
    if (img.timestamp != kExpectedTimestamp) return 1;
    std::printf("sections:\n");
    for (const auto& s : img.sections) {
        std::printf("  %-8s va 0x%08X vsize 0x%X raw 0x%X rawsize 0x%X %s%s\n", s.name.c_str(), s.va,
                    s.virtual_size, s.raw, s.raw_size, s.executable() ? "exec " : "",
                    s.writable() ? "writable" : "");
    }
    dump_old_slots(img);
    scan_exact(img);
    scan_tight(img);
    scan_strings(img);
    std::printf("\nDone. Targets above are candidates from the file. They are not adopted offsets.\n");
    return 0;
}
