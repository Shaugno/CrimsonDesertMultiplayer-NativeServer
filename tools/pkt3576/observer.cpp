#include "pkt3576/observer.h"

#include <cstring>
#include <new>

#include <windows.h>

namespace pkt3576 {
namespace {

bool read_timestamp(const Observer::Deps& deps, uint32_t& timestamp) {
    uint16_t mz = 0;
    if (!deps.read(deps.image_base, &mz, sizeof mz, deps.read_user) || mz != 0x5A4D) return false;
    uint32_t lfanew = 0;
    if (!deps.read(deps.image_base + 0x3C, &lfanew, sizeof lfanew, deps.read_user)) return false;
    if (lfanew < 0x40 || lfanew > 0x1000) return false;
    uint32_t signature = 0;
    if (!deps.read(deps.image_base + lfanew, &signature, sizeof signature, deps.read_user)) return false;
    if (signature != 0x4550) return false;
    return deps.read(deps.image_base + lfanew + 8, &timestamp, sizeof timestamp, deps.read_user);
}

bool same_entry(const uint8_t* found, const uint8_t* expect) {
    return std::memcmp(found, expect, 16) == 0;
}

}  // namespace

bool guarded_read(uint64_t address, void* destination, size_t size) {
    if (destination == nullptr || size == 0 || address == 0) return false;
    const uint64_t end = address + static_cast<uint64_t>(size);
    if (end < address) return false;
    __try {
        std::memcpy(destination, reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), size);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

struct Observer::Ring {
    struct Cell {
        std::atomic<uint64_t> sequence{0};
        Record record{};
    };

    explicit Ring(uint32_t capacity_in) : capacity(capacity_in) {
        cells = new (std::nothrow) Cell[capacity];
        if (cells == nullptr) {
            capacity = 0;
            return;
        }
        for (uint32_t i = 0; i < capacity; ++i) {
            cells[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    ~Ring() { delete[] cells; }

    Ring(const Ring&) = delete;
    Ring& operator=(const Ring&) = delete;

    // Lock-free. A full ring returns false immediately.
    bool try_push(const Record& record) {
        if (cells == nullptr || capacity == 0) return false;
        uint64_t pos = head.load(std::memory_order_relaxed);
        for (;;) {
            Cell& cell = cells[pos % capacity];
            const uint64_t sequence = cell.sequence.load(std::memory_order_acquire);
            const int64_t gap = static_cast<int64_t>(sequence) - static_cast<int64_t>(pos);
            if (gap == 0) {
                if (head.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    cell.record = record;
                    cell.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (gap < 0) {
                return false;
            } else {
                pos = head.load(std::memory_order_relaxed);
            }
        }
    }

    bool try_pop(Record& out) {
        if (cells == nullptr || capacity == 0) return false;
        const uint64_t pos = tail.load(std::memory_order_relaxed);
        Cell& cell = cells[pos % capacity];
        const uint64_t sequence = cell.sequence.load(std::memory_order_acquire);
        const int64_t gap = static_cast<int64_t>(sequence) - static_cast<int64_t>(pos + 1);
        if (gap != 0) return false;
        out = cell.record;
        cell.sequence.store(pos + static_cast<uint64_t>(capacity), std::memory_order_release);
        tail.store(pos + 1, std::memory_order_relaxed);
        return true;
    }

    uint32_t capacity = 0;
    Cell* cells = nullptr;
    std::atomic<uint64_t> head{0};
    std::atomic<uint64_t> tail{0};
};

Observer::Observer(uint32_t capacity) : capacity_(capacity) {}

Observer::~Observer() {
    if (recv_installed_ || send_installed_) shutdown();
    if (recv_installed_ || send_installed_) {
        // A live callback still needs the queue and this callback code.
        ring_ = nullptr;
        return;
    }
    free_queue();
}

bool Observer::enter() {
    if (!open_.load(std::memory_order_acquire)) return false;
    inflight_.fetch_add(1, std::memory_order_acq_rel);
    if (!open_.load(std::memory_order_acquire) || ring_ == nullptr) {
        inflight_.fetch_sub(1, std::memory_order_acq_rel);
        return false;
    }
    return true;
}

void Observer::leave() { inflight_.fetch_sub(1, std::memory_order_acq_rel); }

void Observer::drain_callbacks() {
    while (inflight_.load(std::memory_order_acquire) != 0) YieldProcessor();
}

void Observer::free_queue() {
    delete ring_;
    ring_ = nullptr;
}

bool Observer::read_at(uint64_t address, void* destination, size_t size) const {
    if (read_ == nullptr || destination == nullptr || size == 0) return false;
    return read_(address, destination, size, read_user_);
}

void Observer::observe(Direction direction, const EntryArgs& args, bool send_slots) {
    Record record{};
    record.direction = direction;
    record.rcx = args.rcx;
    record.rdx = args.rdx;
    record.r8 = args.r8;
    record.r9 = args.r9;

    uint16_t claimed = 0;
    if (!read_at(args.rsp + kStackSlot28, &claimed, sizeof claimed)) {
        unread_.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    record.has_stack_u16_28 = true;
    record.stack_u16_28 = claimed;

    if (send_slots) {
        uint8_t stack30 = 0;
        uint16_t stack38 = 0;
        if (!read_at(args.rsp + kStackSlot30, &stack30, sizeof stack30) ||
            !read_at(args.rsp + kStackSlot38, &stack38, sizeof stack38)) {
            unread_.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        record.has_stack_u8_30 = true;
        record.stack_u8_30 = stack30;
        record.has_stack_u16_38 = true;
        record.stack_u16_38 = stack38;
    }

    uint32_t prefix_len = claimed;
    if (prefix_len > kPrefixCap) prefix_len = kPrefixCap;
    if (prefix_len > 0) {
        if (!read_at(args.r9, record.prefix, prefix_len)) {
            unread_.fetch_add(1, std::memory_order_relaxed);
            return;
        }
    }
    record.prefix_len = static_cast<uint8_t>(prefix_len);

    if (ring_ == nullptr || !ring_->try_push(record)) {
        dropped_.fetch_add(1, std::memory_order_relaxed);
    }
}

void Observer::on_send(const EntryArgs& args) {
    if (!enter()) {
        dropped_.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    observe(Direction::Send, args, true);
    leave();
}

void Observer::on_recv(const EntryArgs& args) {
    if (!enter()) {
        dropped_.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    observe(Direction::Receive, args, false);
    leave();
}

bool Observer::try_pop(Record& out) {
    if (ring_ == nullptr) return false;
    return ring_->try_pop(out);
}

uint64_t Observer::dropped() const { return dropped_.load(std::memory_order_relaxed); }

uint64_t Observer::unread() const { return unread_.load(std::memory_order_relaxed); }

bool Observer::queue_allocated() const { return ring_ != nullptr; }

const void* Observer::queue_address() const { return ring_; }

bool Observer::accepting() const { return open_.load(std::memory_order_acquire); }

bool Observer::send_installed() const { return send_installed_; }

bool Observer::recv_installed() const { return recv_installed_; }

bool Observer::callback_code_released() const { return callback_released_; }

Observer::InstallResult Observer::install(const Deps& deps) {
    if (ring_ != nullptr || send_installed_ || recv_installed_) return InstallResult::Refused;
    if (deps.read == nullptr || deps.hooks == nullptr || capacity_ == 0) return InstallResult::Refused;
    if (deps.image_base != kImageBase) return InstallResult::Refused;

    uint32_t timestamp = 0;
    if (!read_timestamp(deps, timestamp) || timestamp != kTimestamp3576) return InstallResult::Refused;

    const uint64_t send_address = deps.image_base + kSendRva;
    const uint64_t recv_address = deps.image_base + kRecvRva;
    uint8_t send_bytes[16] = {};
    uint8_t recv_bytes[16] = {};
    if (!deps.read(send_address, send_bytes, sizeof send_bytes, deps.read_user)) return InstallResult::Refused;
    if (!deps.read(recv_address, recv_bytes, sizeof recv_bytes, deps.read_user)) return InstallResult::Refused;
    if (!same_entry(send_bytes, kSendEntry) || !same_entry(recv_bytes, kRecvEntry)) return InstallResult::Refused;

    Ring* ring = new (std::nothrow) Ring(capacity_);
    if (ring == nullptr || ring->cells == nullptr) {
        delete ring;
        return InstallResult::Refused;
    }

    ring_ = ring;
    read_ = deps.read;
    read_user_ = deps.read_user;
    hooks_ = deps.hooks;
    send_address_ = send_address;
    recv_address_ = recv_address;
    callback_released_ = false;
    open_.store(true, std::memory_order_release);

    if (!hooks_->install(recv_address_, this)) {
        open_.store(false, std::memory_order_release);
        drain_callbacks();
        free_queue();
        hooks_ = nullptr;
        read_ = nullptr;
        return InstallResult::Refused;
    }
    recv_installed_ = true;

    if (!hooks_->install(send_address_, this)) {
        if (hooks_->remove(recv_address_)) {
            recv_installed_ = false;
            open_.store(false, std::memory_order_release);
            drain_callbacks();
            free_queue();
            hooks_->release_callback_code();
            callback_released_ = true;
            hooks_ = nullptr;
            read_ = nullptr;
            return InstallResult::RolledBack;
        }
        return InstallResult::Retained;
    }
    send_installed_ = true;
    return InstallResult::Installed;
}

Observer::ShutdownResult Observer::shutdown() {
    if (!recv_installed_ && !send_installed_) return ShutdownResult::Idle;

    bool failed = false;
    if (recv_installed_) {
        if (hooks_ != nullptr && hooks_->remove(recv_address_)) recv_installed_ = false;
        else failed = true;
    }
    if (send_installed_) {
        if (hooks_ != nullptr && hooks_->remove(send_address_)) send_installed_ = false;
        else failed = true;
    }
    if (failed || recv_installed_ || send_installed_) return ShutdownResult::Retained;

    open_.store(false, std::memory_order_release);
    drain_callbacks();
    free_queue();
    if (hooks_ != nullptr && !callback_released_) {
        hooks_->release_callback_code();
        callback_released_ = true;
    }
    hooks_ = nullptr;
    read_ = nullptr;
    return ShutdownResult::Removed;
}

}  // namespace pkt3576
