#include "pkt3663/observer.h"

#include <cstring>
#include <new>

#include <windows.h>

namespace pkt3663 {
namespace {

uint16_t load_u16(const uint8_t* bytes) {
    return static_cast<uint16_t>(bytes[0] | (static_cast<uint16_t>(bytes[1]) << 8));
}

uint64_t qpc_now() {
    LARGE_INTEGER now{};
    if (!QueryPerformanceCounter(&now)) return 0;
    return static_cast<uint64_t>(now.QuadPart);
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
        for (uint32_t index = 0; index < capacity; ++index) {
            cells[index].sequence.store(index, std::memory_order_relaxed);
        }
    }

    ~Ring() { delete[] cells; }
    Ring(const Ring&) = delete;
    Ring& operator=(const Ring&) = delete;

    bool try_push(const Record& record) {
        if (cells == nullptr || capacity == 0) return false;
        uint64_t position = head.load(std::memory_order_relaxed);
        for (;;) {
            Cell& cell = cells[position % capacity];
            const uint64_t sequence = cell.sequence.load(std::memory_order_acquire);
            const int64_t gap = static_cast<int64_t>(sequence) - static_cast<int64_t>(position);
            if (gap == 0) {
                if (head.compare_exchange_weak(position, position + 1, std::memory_order_relaxed)) {
                    cell.record = record;
                    cell.sequence.store(position + 1, std::memory_order_release);
                    return true;
                }
            } else if (gap < 0) {
                return false;
            } else {
                position = head.load(std::memory_order_relaxed);
            }
        }
    }

    bool try_pop(Record& out) {
        if (cells == nullptr || capacity == 0) return false;
        const uint64_t position = tail.load(std::memory_order_relaxed);
        Cell& cell = cells[position % capacity];
        const uint64_t sequence = cell.sequence.load(std::memory_order_acquire);
        if (static_cast<int64_t>(sequence) - static_cast<int64_t>(position + 1) != 0) return false;
        out = cell.record;
        cell.sequence.store(position + static_cast<uint64_t>(capacity), std::memory_order_release);
        tail.store(position + 1, std::memory_order_relaxed);
        return true;
    }

    uint32_t capacity = 0;
    Cell* cells = nullptr;
    std::atomic<uint64_t> head{0};
    std::atomic<uint64_t> tail{0};
};

Observer* Observer::create(uint32_t capacity) { return new (std::nothrow) Observer(capacity); }

bool Observer::release_owner(Observer*& observer) {
    if (observer == nullptr) return true;
    delete observer;
    observer = nullptr;
    return true;
}

Observer::Observer(uint32_t capacity) : capacity_(capacity) {}

Observer::~Observer() { retire_queue(); }

bool Observer::arm(ReadFn read, void* user) {
    if (ring_ != nullptr || read == nullptr || capacity_ < kMinQueueCapacity) return false;
    Ring* ring = new (std::nothrow) Ring(capacity_);
    if (ring == nullptr || ring->cells == nullptr) {
        delete ring;
        return false;
    }
    ring_ = ring;
    read_ = read;
    read_user_ = user;
    if (!gate_.open()) {
        free_queue();
        read_ = nullptr;
        read_user_ = nullptr;
        return false;
    }
    return true;
}

bool Observer::disarm() {
    retire_queue();
    read_ = nullptr;
    read_user_ = nullptr;
    return true;
}

void Observer::retire_queue() {
    gate_.close_and_wait();
    free_queue();
}

void Observer::free_queue() {
    delete ring_;
    ring_ = nullptr;
}

void Observer::set_consumer_hold(ConsumerHold hold, void* user) {
    consumer_hold_user_.store(user, std::memory_order_release);
    consumer_hold_.store(hold, std::memory_order_release);
}

void Observer::set_phase(uint8_t phase) {
    if (phase > static_cast<uint8_t>(Phase::Turn)) phase = static_cast<uint8_t>(Phase::Unlabeled);
    phase_.store(phase, std::memory_order_relaxed);
}

bool Observer::read_at(uint64_t address, void* destination, size_t size) const {
    if (read_ == nullptr || destination == nullptr || size == 0 || address == 0) return false;
    if (address + static_cast<uint64_t>(size) < address) return false;
    return read_(address, destination, size, read_user_);
}

void Observer::observe(Direction direction, const EntryArgs& args, bool send_slots) {
    if (args.rsp > UINT64_MAX - kStackSlot38) {
        unread_.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    uint16_t claimed = 0;
    if (!read_at(args.rsp + kStackSlot28, &claimed, sizeof claimed)) {
        unread_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    uint8_t stack30 = 0;
    uint16_t stack38 = 0;
    if (send_slots && (!read_at(args.rsp + kStackSlot30, &stack30, sizeof stack30) ||
                       !read_at(args.rsp + kStackSlot38, &stack38, sizeof stack38))) {
        unread_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    Record record{};
    record.direction = direction;
    record.rcx = args.rcx;
    record.rdx = args.rdx;
    record.r8 = args.r8;
    record.r9 = args.r9;
    record.has_stack_u16_28 = true;
    record.stack_u16_28 = claimed;
    record.claimed_length = claimed;
    record.phase = phase_.load(std::memory_order_relaxed);
    record.receive_pre_transform_possible = direction == Direction::Receive;
    if (send_slots) {
        record.has_stack_u8_30 = true;
        record.stack_u8_30 = stack30;
        record.has_stack_u16_38 = true;
        record.stack_u16_38 = stack38;
    }

    const uint32_t copy_length = claimed > kCopyCap ? kCopyCap : static_cast<uint32_t>(claimed);
    if (copy_length != 0 && !read_at(args.r9, record.bytes, copy_length)) {
        unread_.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    record.copied_length = copy_length;
    if (copy_length >= 2) {
        record.has_raw_word0 = true;
        record.raw_word0 = load_u16(record.bytes);
    }
    record.qpc_ticks = qpc_now();
    if (claimed > kCopyCap) {
        record.status = RawStatus::Truncated;
        truncated_.fetch_add(1, std::memory_order_relaxed);
    } else {
        record.status = RawStatus::Complete;
    }

    if (ring_ == nullptr || !ring_->try_push(record)) {
        dropped_.fetch_add(1, std::memory_order_relaxed);
    }
}

void Observer::on_send(const EntryArgs& args) noexcept {
    if (!gate_.admit()) {
        dropped_.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    try {
        observe(Direction::Send, args, true);
    } catch (...) {
        unread_.fetch_add(1, std::memory_order_relaxed);
    }
    gate_.leave();
}

void Observer::on_recv(const EntryArgs& args) noexcept {
    if (!gate_.admit()) {
        dropped_.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    try {
        observe(Direction::Receive, args, false);
    } catch (...) {
        unread_.fetch_add(1, std::memory_order_relaxed);
    }
    gate_.leave();
}

bool Observer::push_for_test(const Record& record) {
    if (ring_ == nullptr) return false;
    return ring_->try_push(record);
}

bool Observer::try_pop(Record& out) noexcept {
    if (!gate_.admit()) return false;
    bool result = false;
    try {
        const ConsumerHold hold = consumer_hold_.load(std::memory_order_acquire);
        if (hold != nullptr) hold(consumer_hold_user_.load(std::memory_order_acquire));
        result = ring_ != nullptr && ring_->try_pop(out);
    } catch (...) {
        result = false;
    }
    gate_.leave();
    return result;
}

uint64_t Observer::dropped() const { return dropped_.load(std::memory_order_relaxed); }
uint64_t Observer::unread() const { return unread_.load(std::memory_order_relaxed); }
uint64_t Observer::truncated() const { return truncated_.load(std::memory_order_relaxed); }
bool Observer::queue_allocated() const { return ring_ != nullptr; }
const void* Observer::queue_address() const { return ring_; }
bool Observer::accepting() const { return !gate_.closed(); }

}  // namespace pkt3663
