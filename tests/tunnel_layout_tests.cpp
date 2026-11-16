// Game-free boundary and denial regressions for the shared tunnel profile.
#include "mod/session2/tunnel_layout.h"
#include <cstdio>
#include <vector>
int main() {
    using namespace cdmp;
    using namespace session2;
    int failed=0;
    auto check=[&](bool ok,const char* message) { if(!ok) { ++failed; std::printf("FAIL %s\n",message); } };
    const auto& old=kTunnelLayout2976;
    const auto& target=kTunnelLayout3663;
    check(old.packet_max==troc::kMaxPacketSize,"legacy packet acceptance unchanged");
    check(target.packet_max==0x7FDA && target.ordinary_payload_max==0x7FC1,"native target bounds");
    check(target.descriptor_skip==0 && old.descriptor_skip==0x80,"target scans all aligned slots");
    check(target.kNetLoopPump==0x3A8D94 && target.kNetLoopPumpPatch==7,"verified target midpoint");
    auto packet=[&](size_t payload) { return troc::make(2401,-1,std::vector<uint8_t>(payload)); };
    auto p=packet(target.ordinary_payload_max);
    check(parse_tunnel_packet(p,troc::Dir::ToHostServer,target).has_value(),"exact ordinary payload maximum");
    p=packet(target.ordinary_payload_max+1);
    check(!parse_tunnel_packet(p,troc::Dir::ToHostServer,target),"payload overflow below outer packet cap");
    check(parse_tunnel_packet(p,troc::Dir::ToHostServer,old).has_value(),"legacy limit preserved");
    p=packet(old.ordinary_payload_max);
    check(parse_tunnel_packet(p,troc::Dir::ToHostServer,old).has_value(),"legacy exact maximum");
    check(!parse_tunnel_packet(p,troc::Dir::ToHostServer,target),"target outer maximum");
    p=packet(8);p.pop_back();
    check(!parse_tunnel_packet(p,troc::Dir::ToHostServer,target),"truncated declared length");
    const auto logout=troc::make_logout_session_req();
    check(parse_tunnel_packet(logout,troc::Dir::ToHostServer,target).has_value(),"exact internal request");
    check(!parse_tunnel_packet(logout,troc::Dir::ToGuestClient,target),"internal direction enforced");
    p=packet(16);
    check(core_tunnel_request_allowed(p,target),"B admits named cryptogram only");
    p[2]=0;check(!core_tunnel_request_allowed(p,target),"non-default slot denied");
    p=troc::make(2490,-1,std::vector<uint8_t>(16));
    check(!core_tunnel_request_allowed(p,target),"login not reachable with B only");
    check(!core_tunnel_request_allowed(logout,target),"native logout waits for D");
    game::build_guard::ImageIdentity id;
    check(!tunnel_layout(id),"unknown denied");
    id.kind=game::build_guard::IdentityKind::Build3663;
    id.timestamp_known=id.size_known=true;
    id.timestamp=game::build_3663::kPeTimestamp;
    id.mapped_base=game::build_3663::kImageBase;
    id.size_of_image=game::build_3663::kSizeOfImage;
    check((tunnel_layout(id)!=nullptr)==game::build_3663::kTierBComplete,"compiled permission required");
    id.size_of_image-=0x1000;check(!tunnel_layout(id),"wrong size denied");
    id.size_of_image+=0x1000;id.mapped_base+=0x1000;check(!tunnel_layout(id),"wrong base denied");
    if(!failed) std::puts("Tunnel native bounds, legacy preservation and identity denial PASS");
    return failed?1:0;
}
