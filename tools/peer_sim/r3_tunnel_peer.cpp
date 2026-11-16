#include "r3_tunnel_peer.h"
#include "common/client_session.h"
#include "common/clock.h"
#include "common/troc_link.h"
#include "common/udp.h"
#include "mod/session2/tunnel_layout.h"
#include <cstdio>
#include <optional>

int run_r3_tunnel_peer(uint16_t port, int seconds, volatile bool& running) {
    using namespace cdmp;
    net::UdpSocket socket;
    if (!socket.open(port, true)) return 1;
    std::optional<net::Endpoint> peer;
    ClientSession session("cdmp_peer (R3 loopback)", 0,
        [&](const std::vector<uint8_t>& bytes) { if (peer) socket.send_to(*peer, bytes); },
        [](const std::string& text) { std::printf("Session: %s\n", text.c_str()); },
        ClientSession::Role::Responder);
    std::optional<std::vector<uint8_t>> echo;
    bool copied = false;
    uint64_t responses = 0, states_echoed = 0;
    troc::TrocLink link(session, troc::Dir::ToHostServer, troc::Dir::ToGuestClient,
        [&](std::vector<uint8_t>&& packet, uint32_t) {
            const auto view = session2::parse_tunnel_packet(packet, troc::Dir::ToGuestClient,
                                                           session2::kTunnelLayout3663);
            if (!view) return;
            if (view->opcode == session2::kTunnelLayout3663.connect_opcode) {
                if (!copied) {
                    copied = true; echo = std::move(packet);
                    std::printf("R3: received game-generated cryptogram request; echo once, bytes %zu\n", echo->size());
                }
            } else if (view->opcode == 2128) { // named target ConnectCryptogramToServerAck
                ++responses;
                std::printf("R3: native response opcode %u bytes %zu (payload private)\n", view->opcode, packet.size());
            }
        });
    std::printf("R3 tunnel peer on 127.0.0.1:%u; one public-key request echo; real pose echo; no login/save/world replay\n", port);
    const uint32_t start = now_ms();
    uint32_t last_report = start;
    std::vector<uint8_t> bytes(proto::kMaxPacketSize);
    while (running && (seconds <= 0 || now_ms() - start < static_cast<uint32_t>(seconds) * 1000)) {
        net::Endpoint from;
        if (const auto n = socket.recv_from(from, bytes, 5); n && from.is_loopback()) {
            const auto packet = proto::decode({bytes.data(), *n});
            if (!peer && packet && std::holds_alternative<proto::Hello>(packet->body)) peer = from;
            if (peer && from == *peer) session.on_datagram({bytes.data(), *n}, now_ms());
        }
        const auto now = now_ms();
        session.tick(now); link.tick(now);
        if (session.stats().remote_states > states_echoed && session.remote_state()) {
            // Echo only an actual received state; never fabricate a pose.
            session.send_local_state(*session.remote_state(), now);
            states_echoed = session.stats().remote_states;
        }
        if (echo && link.connected()) {
            if (!link.push(*echo)) return 3;
            echo.reset();
        }
        link.tick(now);
        if (now - last_report >= 2000) {
            std::printf("R3: real states echoed %llu native responses %llu | %s\n",
                static_cast<unsigned long long>(states_echoed), static_cast<unsigned long long>(responses),
                link.describe().c_str());
            std::fflush(stdout); last_report = now;
        }
    }
    session.disconnect(now_ms());
    std::printf("R3 peer stopped: real states %llu native responses %llu\n",
        static_cast<unsigned long long>(states_echoed), static_cast<unsigned long long>(responses));
    return 0;
}
