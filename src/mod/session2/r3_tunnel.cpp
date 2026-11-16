#include "mod/session2/r3_tunnel.h"
#include "mod/session2/server.h"
#include "mod/session2/tunnel_layout.h"
#include "mod/log.h"
#include <format>

namespace cdmp::session2 {
R3TunnelBridge::R3TunnelBridge(ClientSession& session)
    : link_(session, troc::Dir::ToGuestClient, troc::Dir::ToHostServer,
        [this](std::vector<uint8_t>&& p, uint32_t now) { receive(std::move(p), now); }) {}

void R3TunnelBridge::receive(std::vector<uint8_t>&& packet, uint32_t now) {
    const auto view = parse_tunnel_packet(packet, troc::Dir::ToHostServer, kTunnelLayout3663);
    if (failed_ || !view || !core_tunnel_request_allowed(packet, kTunnelLayout3663)) { ++refused_; return; }
    if (pending_.size() >= 16) { failed_ = true; return; }
    if (pending_.empty()) first_pending_ = now;
    pending_.push_back(std::move(packet));
}

void R3TunnelBridge::tick(uint32_t now) {
    link_.tick(now);
    if (!link_.connected() || failed_) return;
    if (!epoch_) epoch_ = link_.session_epoch();
    if (epoch_ != link_.session_epoch() || link_.broken() || server_inject_halted() || server_outgoing_lost()) {
        failed_ = true;
        log::error("3663 B: tunnel halted; native reconnect is gated by D");
        return;
    }
    while (!pending_.empty()) {
        const auto result = server_inject_ex(pending_.front());
        const auto disposition = inj::disposition(result);
        if (disposition == inj::Disposition::Hold) {
            if (now - first_pending_ > 15000) failed_ = true;
            break;
        }
        if (disposition == inj::Disposition::Stop) { failed_ = true; break; }
        if (disposition == inj::Disposition::Done) ++queued_; else ++refused_;
        log::info("3663 B: cryptogram request {} ({} bytes)", inj::outcome_name(result), pending_.front().size());
        pending_.pop_front();
    }
    if (!failed_) {
        for (auto& packet : server_take_outgoing()) {
            const auto view = parse_tunnel_packet(packet, troc::Dir::ToGuestClient, kTunnelLayout3663);
            if (!view || !link_.push(packet)) { failed_ = true; break; }
            log::info("3663 B: native/captured packet opcode {} bytes {} queued for loopback peer", view->opcode, packet.size());
        }
    }
    link_.tick(now);
}

std::string R3TunnelBridge::describe() const {
    return std::format("3663 B: queued {} refused {} pending {} halted {} | {} | {} | {}",
        queued_, refused_, pending_.size(), failed_, server_epoch_describe(), server_inject_describe(), link_.describe());
}
}
