#pragma once
// Read-only view used by the 3576 diagnostic. The live tool adapts
// probe::Process. Tests supply a fake.

#include <cstddef>
#include <cstdint>
#include <string>

namespace diag3576 {

class Memory {
public:
    virtual ~Memory() = default;

    virtual bool alive() const = 0;
    virtual uint64_t module_base() const = 0;
    virtual uint64_t module_size() const = 0;
    virtual std::wstring module_path() const = 0;
    virtual bool read(uint64_t addr, void* out, size_t size) const = 0;
    virtual std::string rtti_name(uint64_t object) const = 0;
};

}  // namespace diag3576
