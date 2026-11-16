#pragma once
// File-analysis policy of native 3663 consumer 758EA0. Not a game-memory
// reader or early guest save redirect. D130 stays blocked on native lifetimes.
#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>
namespace cdmp::coop3663::title {
struct Entry { uint32_t slot{}; std::span<const uint32_t> required_metadata; };
enum class Eligibility { Eligible, Ineligible, Unavailable };
inline Eligibility evaluate(std::span<const Entry> entries,std::span<const uint32_t> available,
                            bool coherent) {
    // Explicit diagnostic limits, not alleged native allocation constants.
    if (!coherent || entries.size()>128 || available.size()>4096) return Eligibility::Unavailable;
    // Check every borrowed view before using a partial result.
    for(const auto& e:entries) if(e.required_metadata.size()>4096) return Eligibility::Unavailable;
    std::vector<uint32_t> sorted(available.begin(),available.end());
    std::sort(sorted.begin(),sorted.end());
    for(const auto& e:entries) {
        if(!(e.slot<=2 || (e.slot>=100 && e.slot<=108)))continue;
        bool all=true;
        for(uint32_t id:e.required_metadata)
            if(!std::binary_search(sorted.begin(),sorted.end(),id)){all=false;break;}
        if(all)return Eligibility::Eligible;
    }
    return Eligibility::Ineligible;
}
}
