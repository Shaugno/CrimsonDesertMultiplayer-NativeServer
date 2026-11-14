// File-only disassembly of 3663 acknowledgement handlers.
// Maps the executable as a read-only SEC_IMAGE view. It does not open a
// process, install a hook, or write the file.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include <windows.h>

#include <Zydis/Zydis.h>

namespace {

constexpr uint32_t kTimestamp = 0x6AF5D198;
constexpr uint64_t kImageBase = 0x140000000;
constexpr uint32_t kSizeOfImage = 0x144E9000;

struct Section {
  uint32_t va = 0;
  uint32_t size = 0;
  uint32_t flags = 0;
};

struct Image {
  const uint8_t *base = nullptr;
  uint32_t size = 0;
  uint32_t timestamp = 0;
  uint64_t preferred = 0;
  std::vector<Section> sections;
  const uint8_t *pdata = nullptr;
  uint32_t pdata_bytes = 0;
};

bool read_u16(const Image &image, uint32_t rva, uint16_t &out) {
  if (rva > image.size || image.size - rva < 2)
    return false;
  std::memcpy(&out, image.base + rva, 2);
  return true;
}
bool read_u32(const Image &image, uint32_t rva, uint32_t &out) {
  if (rva > image.size || image.size - rva < 4)
    return false;
  std::memcpy(&out, image.base + rva, 4);
  return true;
}

bool parse_image(const uint8_t *view, Image &image) {
  image.base = view;
  if (std::memcmp(view, "MZ", 2) != 0)
    return false;
  uint32_t e_lfanew = 0;
  std::memcpy(&e_lfanew, view + 0x3C, 4);
  if (e_lfanew > 0x1000)
    return false;
  if (std::memcmp(view + e_lfanew, "PE\0\0", 4) != 0)
    return false;
  const uint8_t *pe = view + e_lfanew;
  uint16_t magic = 0;
  std::memcpy(&magic, pe + 24, 2);
  if (magic != 0x20B)
    return false;
  std::memcpy(&image.timestamp, pe + 8, 4);
  std::memcpy(&image.preferred, pe + 24 + 24, 8);
  std::memcpy(&image.size, pe + 24 + 56, 4);
  uint16_t sections = 0;
  uint16_t optional = 0;
  std::memcpy(&sections, pe + 6, 2);
  std::memcpy(&optional, pe + 20, 2);
  const uint8_t *section = pe + 24 + optional;
  for (uint16_t index = 0; index < sections; ++index) {
    const uint8_t *entry = section + index * 40;
    Section item;
    std::memcpy(&item.size, entry + 8, 4);
    std::memcpy(&item.va, entry + 12, 4);
    std::memcpy(&item.flags, entry + 36, 4);
    image.sections.push_back(item);
  }
  uint32_t exception_rva = 0;
  uint32_t exception_size = 0;
  std::memcpy(&exception_rva, pe + 24 + 112 + 3 * 8, 4);
  std::memcpy(&exception_size, pe + 24 + 112 + 3 * 8 + 4, 4);
  if (exception_rva != 0 && exception_size != 0 && exception_rva < image.size &&
      exception_size <= image.size - exception_rva) {
    image.pdata = view + exception_rva;
    image.pdata_bytes = exception_size;
  }
  return true;
}

bool function_bounds(const Image &image, uint32_t rva, uint32_t &begin, uint32_t &end) {
  if (image.pdata == nullptr || image.pdata_bytes < 12)
    return false;
  const uint32_t count = image.pdata_bytes / 12;
  for (uint32_t index = 0; index < count; ++index) {
    uint32_t start = 0;
    uint32_t stop = 0;
    std::memcpy(&start, image.pdata + index * 12, 4);
    std::memcpy(&stop, image.pdata + index * 12 + 4, 4);
    if (rva >= start && rva < stop) {
      begin = start;
      end = stop;
      return true;
    }
  }
  return false;
}

bool executable(const Image &image, uint32_t rva) {
  for (const Section &section : image.sections) {
    if (rva >= section.va && rva < section.va + section.size)
      return (section.flags & 0x20000000) != 0;
  }
  return false;
}

void disassemble(const Image &image, const char *name, uint32_t rva, uint32_t window = 0) {
  uint32_t begin = 0;
  uint32_t end = 0;
  std::printf("handler name=%s rva=0x%X ", name, rva);
  if (window != 0 && rva + window <= image.size) {
    begin = rva;
    end = rva + window;
  } else if (!function_bounds(image, rva, begin, end) && rva + 5 <= image.size && image.base[rva] == 0xE9) {
    int32_t relative = 0;
    std::memcpy(&relative, image.base + rva + 1, 4);
    const uint32_t target = static_cast<uint32_t>(rva + 5 + relative);
    std::printf("thunk=0x%X ", target);
    rva = target;
    function_bounds(image, rva, begin, end);
  }
  if (begin == 0 || end == 0) {
    std::printf("pdata=missing raw-window\n");
    if (rva + 64 <= image.size)
      end = rva + 64;
    else
      return;
    begin = rva;
  }
  std::printf("begin=0x%X end=0x%X size=%u entry=%d\n", begin, end, end - begin,
              rva == begin ? 1 : 0);
  if (!executable(image, begin) || end <= begin || end - begin > 0x20000 || end > image.size) {
    std::printf("  rejected range\n");
    return;
  }
  ZydisDecoder decoder;
  ZydisFormatter formatter;
  if (!ZYAN_SUCCESS(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64)) ||
      !ZYAN_SUCCESS(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL))) {
    std::printf("  decoder-failed\n");
    return;
  }
  uint32_t cursor = begin;
  int instructions = 0;
  while (cursor < end && instructions < 4000) {
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
    const ZyanUSize available = end - cursor;
    if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, image.base + cursor, available, &instruction,
                                             operands))) {
      std::printf("  0x%X undecodable\n", cursor);
      break;
    }
    char text[192] = {};
    ZydisFormatterFormatInstruction(&formatter, &instruction, operands, instruction.operand_count_visible,
                                    text, sizeof text, kImageBase + cursor, ZYAN_NULL);
    std::printf("  0x%X %s\n", cursor, text);
    if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL) {
      uint32_t target = 0;
      if (instruction.operand_count_visible > 0 &&
          operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
        const int64_t absolute =
            static_cast<int64_t>(kImageBase + cursor + instruction.length) + operands[0].imm.value.s;
        if (absolute >= static_cast<int64_t>(kImageBase) &&
            absolute < static_cast<int64_t>(kImageBase + image.size)) {
          target = static_cast<uint32_t>(absolute - static_cast<int64_t>(kImageBase));
        }
      }
      uint32_t target_begin = 0;
      uint32_t target_end = 0;
      const bool bounded = target != 0 && function_bounds(image, target, target_begin, target_end);
      std::printf("    call target=0x%X pdata=%d begin=0x%X end=0x%X\n", target, bounded ? 1 : 0,
                  target_begin, target_end);
    }
    cursor += instruction.length;
    ++instructions;
  }
  std::printf("  instructions=%d next=0x%X\n", instructions, cursor);
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    std::fprintf(stderr, "usage: pkt3663_ack_trace <CrimsonDesert.exe>\n");
    return 2;
  }
  wchar_t wide[1024] = {};
  if (MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, wide, 1024) == 0)
    return 2;
  HANDLE file = CreateFileW(wide, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    std::fprintf(stderr, "unreadable file\n");
    return 2;
  }
  HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY | SEC_IMAGE, 0, 0, nullptr);
  void *view = mapping == nullptr ? nullptr : MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
  if (view == nullptr) {
    std::fprintf(stderr, "map failed\n");
    if (mapping != nullptr)
      CloseHandle(mapping);
    CloseHandle(file);
    return 2;
  }
  Image image;
  if (!parse_image(static_cast<const uint8_t *>(view), image) || image.timestamp != kTimestamp ||
      image.preferred != kImageBase || image.size != kSizeOfImage) {
    std::printf("identity-rejected timestamp=0x%08X base=0x%llX size=0x%X\n", image.timestamp,
                static_cast<unsigned long long>(image.preferred), image.size);
    UnmapViewOfFile(view);
    CloseHandle(mapping);
    CloseHandle(file);
    return 3;
  }
  std::printf("identity=build-3663 timestamp=0x%08X adopted=0\n", image.timestamp);
  // File RTTI inventory only; names do not establish a component wire schema.
  for (const auto &section : image.sections) {
    if (!(section.flags & 0x40000000) || section.va > image.size ||
        section.size > image.size - section.va || section.size < 192)
      continue;
    for (uint32_t i = 0; i <= section.size - 192; ++i) {
      const auto *name = reinterpret_cast<const char *>(image.base + section.va + i);
      if (std::memcmp(name, ".?AV", 4) != 0 || std::memchr(name, 0, 192) == nullptr)
        continue;
      if (std::strstr(name, "ActorComponent") || std::strstr(name, "ActorFactory"))
        std::printf("component-rtti name-rva=0x%X name=%s adopted=0\n", section.va + i, name);
    }
  }
  // Resolve two exact RTTI names through primary MSVC complete-object locators.
  // Require signature, offset, type descriptor and self RVA, then a file pointer.
  for (uint32_t descriptor : {0x6F858A0u, 0x6F8B880u}) {
    for (const auto &section : image.sections) {
      if (!(section.flags & 0x40000000) || section.va > image.size ||
          section.size > image.size - section.va || section.size < 24)
        continue;
      for (uint32_t i = 0; i <= section.size - 24; i += 4) {
        const uint32_t at = section.va + i;
        uint32_t fields[6];
        std::memcpy(fields, image.base + at, sizeof fields);
        if (fields[0] != 1 || fields[1] != 0 || fields[3] != descriptor || fields[5] != at)
          continue;
        for (const auto &pointers : image.sections) {
          if (!(pointers.flags & 0x40000000) || pointers.va > image.size ||
              pointers.size > image.size - pointers.va || pointers.size < 0x78)
            continue;
          for (uint32_t j = 0; j <= pointers.size - 0x78; j += 8) {
            uint64_t value = 0;
            const uint32_t table = pointers.va + j + 8;
            std::memcpy(&value, image.base + table - 8, 8);
            if (value != kImageBase + at) continue;
            std::printf("component-table descriptor=0x%X locator=0x%X table=0x%X adopted=0\n",
                        descriptor, at, table);
            for (uint32_t offset : {0x18u, 0x38u, 0x58u, 0x68u}) {
              uint64_t target = 0;
              std::memcpy(&target, image.base + table + offset, 8);
              if (target < kImageBase || target - kImageBase >= image.size) continue;
              char label[80];
              std::snprintf(label, sizeof label, "Component%XSlot%X", table, offset);
              disassemble(image, label, static_cast<uint32_t>(target - kImageBase));
            }
          }
        }
      }
    }
  }
  uint64_t slot = 0;
  if (image.size >= 0x5B3BE40 + 8)
    std::memcpy(&slot, image.base + 0x5B3BE40, 8);
  std::printf("move-reader-vtable=0x5B3BE18 slot28=0x%llX\n",
              static_cast<unsigned long long>(slot));
  disassemble(image, "ActorLoadDataAck-slot2", 0xE22330);
  disassemble(image, "CharacterScalar", 0x2593A00);
  disassemble(image, "CharacterString", 0x1643F30);
  disassemble(image, "CharacterEntry", 0x19D2DB0);
  disassemble(image, "TransformNested", 0x14D0870);
  disassemble(image, "ActorLoadDataAck-slot1", 0xE22130);
  disassemble(image, "MoveActorAck", 0xCA8AB0);
  disassemble(image, "RotateActorAck", 0xCA8FB0);
  disassemble(image, "RemoveActorAck", 0xCA9660);
  disassemble(image, "MoveActorReq", 0x2C52CD0);
  disassemble(image, "MoveHelper", 0x26A8CE0);
  disassemble(image, "ReaderDirect", 0x1643CD0);
  disassemble(image, "HelperA", 0x14CB240);
  disassemble(image, "HelperB", 0x1644720);
  disassemble(image, "HelperC", 0x1644640);
  disassemble(image, "HelperD", 0x14CB150);
  disassemble(image, "ReaderScope", 0x16436F0, 0x180);
  disassemble(image, "ActorLoadDispatch", 0x9D68E0);
  disassemble(image, "MoveConsumer", 0x55EF60);
  disassemble(image, "MoveConsumerB0", 0x993D30);
  disassemble(image, "MoveConsumerC0", 0x993BA0);
  disassemble(image, "MoveConsumerState", 0x9A5310);
  disassemble(image, "ActorDispatchLookup", 0x9C7C70);
  disassemble(image, "RemoveTransition", 0x2294C50);
  disassemble(image, "RemovePredicate", 0x22976F0);
  disassemble(image, "MoveApplyB0", 0xB3A240);
  disassemble(image, "MoveApplyC0", 0x5E90A0);
  disassemble(image, "MoveSpatialDestination", 0xB39E40);
  disassemble(image, "ActorDispatchFactory", 0x5344D0);
  disassemble(image, "SpatialApply", 0x7B0960);
  disassemble(image, "FactoryInitialize", 0xCC7AC20);
  disassemble(image, "ComponentInitialize", 0xCC8E5A0);
  disassemble(image, "ComponentInitializeContinuation", 0xCC8E630);
  disassemble(image, "ComponentInitializeFailure", 0xCC8E6D6);
  disassemble(image, "BodyUnlinkRelation", 0x198F780);
  disassemble(image, "ActorRemoveTransition", 0xCC82570);
  disassemble(image, "DetachCallback", 0x9DDEB0);
  disassemble(image, "DetachCallbackBody", 0x9742160);
  disassemble(image, "ComponentCleanupContinuation", 0xCC88D77);
  disassemble(image, "ComponentCleanupEnd", 0xCC88DEB);
  disassemble(image, "FactoryReadContinuation", 0xCC93A38);
  disassemble(image, "FactoryDetach", 0x9CA890);
  disassemble(image, "FactoryRegistryRemove", 0x534CC0);
  disassemble(image, "BodyFactory", 0x9C7660);
  disassemble(image, "BodyDeserialize", 0x198E820);
  disassemble(image, "BodyAllocateA", 0x5346C0);
  disassemble(image, "BodyAllocateB", 0x5349D0);
  disassemble(image, "BodyFactoryRegister", 0x534820);
  disassemble(image, "HeadingInputCheck", 0x3E5100, 0x100);
  disassemble(image, "HeadingApplyContinuation", 0xB3A278, 0x12C);
  disassemble(image, "HeadingApplyBranch", 0xB3A38E);
  disassemble(image, "SpatialCanonicalize", 0x5F0EF0);
  disassemble(image, "SpatialNotify", 0x69B0F0);
  uint32_t cursor_constant = 0, bound_constant = 0;
  uint32_t entry_destination = 0;
  if (read_u32(image, 0x86C6F90, entry_destination))
    std::printf("character-entry-file-tag-destination=0x%X adopted=0\n",
                0x57F9E1BBu ^ entry_destination);
  uint32_t stride_constant = 0, tail_stack_constant = 0;
  if (read_u32(image, 0xC41A3B8, stride_constant) &&
      read_u32(image, 0xAFB6A78, tail_stack_constant)) {
    std::printf("body-reader-file-component-stride=%u tail-count-stack=0x%X adopted=0\n",
                0x49272B5Du - stride_constant, 0xA20F2B54u - tail_stack_constant);
  }
  if (read_u32(image, 0x12345B60, cursor_constant) &&
      read_u32(image, 0x10478F9C, bound_constant)) {
    std::printf("actor-load-file-cursor=%u computed-bound=%u adopted=0\n",
                0x41314825u - cursor_constant,
                static_cast<unsigned>((0x78B87FC1u + bound_constant) & 0xFFFFu));
  }
  // The factory installs this vtable after allocating 0x190 bytes. Resolve
  // file-backed slots only; never invoke any of them.
  for (uint32_t table : {0x59C1D90u, 0x5991108u, 0x5990BF0u, 0x5990B68u}) {
  uint64_t locator = 0;
  if (table < 8 || table > image.size || image.size - table < 8)
    continue;
  std::memcpy(&locator, image.base + table - 8, sizeof locator);
  if (locator >= kImageBase && locator - kImageBase < image.size) {
    uint32_t descriptor = 0;
    const auto locator_rva = static_cast<uint32_t>(locator - kImageBase);
    if (locator_rva <= image.size - 16 &&
        read_u32(image, locator_rva + 12, descriptor) &&
        descriptor <= image.size - 16) {
      char type[192] = {};
      const uint32_t available = image.size - descriptor - 16;
      const size_t copied = available < sizeof(type) - 1 ? available : sizeof(type) - 1;
      std::memcpy(type, image.base + descriptor + 16, copied);
      std::printf("factory-vtable=0x%X type=%s adopted=0\n", table, type);
    }
  }
  }
  for (uint32_t table : {0x59C1D90u, 0x5991108u, 0x5990BF0u}) {
  for (uint32_t offset : {0x28u, 0x40u, 0x80u, 0x110u, 0x118u, 0x130u, 0x150u}) {
    uint64_t target = 0;
    const uint32_t at = table + offset;
    if (at <= image.size && image.size - at >= sizeof target)
      std::memcpy(&target, image.base + at, sizeof target);
    std::printf("factory-vtable=0x%X slot=0x%X target=0x%llX adopted=0\n",
                table, offset, static_cast<unsigned long long>(target));
    if (target >= kImageBase && target - kImageBase < image.size) {
      char label[64];
      std::snprintf(label, sizeof label, "Table%XSlot%X", table, offset);
      disassemble(image, label, static_cast<uint32_t>(target - kImageBase));
    }
  }
  }
  float spatial_scale = 0;
  float rotation_unit = 0;
  if (image.size >= 0x61BEB48 + sizeof rotation_unit) {
    std::memcpy(&rotation_unit, image.base + 0x61BEB48, sizeof rotation_unit);
    std::printf("rotation-normalization-numerator rva=0x61BEB48 f32=%.9g adopted=0\n",
                static_cast<double>(rotation_unit));
  }
  if (image.size >= 0x61BF1C8 + sizeof spatial_scale) {
    std::memcpy(&spatial_scale, image.base + 0x61BF1C8, sizeof spatial_scale);
    std::printf("spatial-conversion-constant rva=0x61BF1C8 f32=%.9g adopted=0\n",
                static_cast<double>(spatial_scale));
  }
  UnmapViewOfFile(view);
  CloseHandle(mapping);
  CloseHandle(file);
  return 0;
}
