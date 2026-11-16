#include "coop3663/title3663_eligibility.h"
#include <array>
#include <cstdio>
#include <vector>
int main() {
    using namespace cdmp::coop3663::title;
    std::array<uint32_t,3> ids{9,3,0xFFFFFFFF}, required{3,9,3};
    std::array<Entry,1> entries{{{1,required}}};
    if(evaluate(entries,ids,true)!=Eligibility::Eligible)return 1;
    if(evaluate(entries,{ids.data(),1},true)!=Eligibility::Ineligible)return 2;
    if(evaluate(entries,ids,false)!=Eligibility::Unavailable)return 3;
    entries[0].required_metadata={};
    for(uint32_t slot:{0u,1u,2u,100u,108u}) {entries[0].slot=slot;if(evaluate(entries,{},true)!=Eligibility::Eligible)return 4;}
    for(uint32_t slot:{3u,99u,109u,200u,204u,0xFFFFFFFFu}) {entries[0].slot=slot;if(evaluate(entries,ids,true)!=Eligibility::Ineligible)return 5;}
    std::array<Entry,2> mixed{{{3,{}},{100,required}}};
    if(evaluate(mixed,ids,true)!=Eligibility::Eligible)return 6;
    if(evaluate({},ids,true)!=Eligibility::Ineligible)return 7;
    std::vector<uint32_t> too_many(4097,3);entries[0]={1,too_many};
    if(evaluate(entries,ids,true)!=Eligibility::Unavailable)return 8;
    entries[0]={1,{}};if(evaluate(entries,too_many,true)!=Eligibility::Unavailable)return 9;
    std::vector<uint32_t> full(4096), needs(4096);
    for(uint32_t i=0;i<4096;++i){full[i]=4095-i;needs[i]=i;}
    std::array<Entry,128> boundary;
    for(auto& entry:boundary)entry={1,needs};
    if(evaluate(boundary,full,true)!=Eligibility::Eligible)return 10;
    needs.back()=10000;
    if(evaluate(boundary,full,true)!=Eligibility::Ineligible)return 11;
    std::puts("3663 title slot domain plus complete metadata membership, bounds and coherence policy PASS; D not armed");
    return 0;
}
