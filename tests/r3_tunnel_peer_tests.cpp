#include "common/client_session.h"
#include "common/troc_link.h"
#include "mod/session2/tunnel_layout.h"
#include <cstdio>
#include <deque>
int main() {
 using namespace cdmp;
 std::deque<std::vector<uint8_t>> to_host,to_peer;
 ClientSession host("host",0,[&](const auto& b){to_peer.push_back(b);});
 ClientSession peer("peer",0,[&](const auto& b){to_host.push_back(b);},{},ClientSession::Role::Responder);
 bool request=false,response=false;
 troc::TrocLink* peer_link=nullptr;
 troc::TrocLink h(host,troc::Dir::ToGuestClient,troc::Dir::ToHostServer,
  [&](std::vector<uint8_t>&& p,uint32_t){request=session2::core_tunnel_request_allowed(p,session2::kTunnelLayout3663);});
 troc::TrocLink g(peer,troc::Dir::ToHostServer,troc::Dir::ToGuestClient,
  [&](std::vector<uint8_t>&& p,uint32_t){
    auto v=troc::parse(p); if(!v)return;
    if(v->opcode==2401) {if(!peer_link->push(p)) std::abort();} else if(v->opcode==2128) response=true;
  });
 peer_link=&g;
 bool sent_req=false,sent_ack=false;
 for(uint32_t now=1;now<2000;now+=5) {
  host.tick(now);peer.tick(now);
  while(!to_host.empty()){host.on_datagram(to_host.front(),now);to_host.pop_front();}
  while(!to_peer.empty()){peer.on_datagram(to_peer.front(),now);to_peer.pop_front();}
  h.tick(now);g.tick(now);
  if(h.connected()&&!sent_req){sent_req=h.push(troc::make(2401,-1,std::vector<uint8_t>(20,1)));}
  if(request&&!sent_ack){sent_ack=h.push(troc::make(2128,-1,std::vector<uint8_t>(8)));}
  proto::PlayerState state;state.pos={1,2,3};host.send_local_state(state,now);peer.send_local_state(state,now);
  h.tick(now);g.tick(now);
 }
 if(!request||!response||!host.stats().remote_states||!peer.stats().remote_states) return 1;
 std::puts("R3 reliable cryptogram echo, response and bidirectional states PASS (synthetic, game-free)");
 return 0;
}
