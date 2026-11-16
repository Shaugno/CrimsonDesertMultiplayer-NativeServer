# Native identity contracts, R2

The paired lobby loaders are 2976 `2B7F4D0` and candidate 3663 `2E879A0`;
the status loaders are `293FE70` and candidate `2C160B0` (Denuvo body
`E9F4150`). Target RTTI independently places these in slot `+50` of
ServerLobbyActorComponent and ServerStatusActorComponent. Function/vtable
addresses still require standard matcher support before adoption.

The status character-number pointer **changes from +70 to +90**. The target
status loader writes INT64_MAX through `[component+90]`, and the named native
LoginCharacter implementation independently compares the requested character
number through actor+68, status+20, pointer+90. Legacy hooks cannot be reused
with their +70 field.

The lobby number remains the first qword of a D0-byte entry. Paired native
entry writers `2143E70` / `237EA60` (thunks into Denuvo) search that number,
read component+18's vector count+8, and append the passed number at entry+0.
Target writer calls `B15EB0` to construct the new entry; clear `B15C50`
independently walks D0-byte entries. Lobby load obtains its owner at +8,
locks the owner's lock pointer at +8, and passes INT64_MAX to the writer.

The first identity checkpoint added 18 member/value proofs, then bringing the
catalog to 149 beyond the 17 retained R1 values. This is not a complete identity port:
native load order, fix hook sites, login-manager lookup ownership, return-site
classification, and all function-entry signatures remain separate checks.
No native hook, call, game-memory write, or live test was performed.

The actual target FindByCharacterNo is now independently identified as
2D7A640, paired with 2A83F80. It takes the manager+68 read lock, scans the
pointer vector at +B8 with u32 count +C0 and eight-byte elements, compares the
requested u64 through actor+68/status+20/number-pointer+90, acquires the native
result reference and releases the lock. This confirms +B8/+C0 as member facts.
Validation windows map scan 2A84066→2D7A726 and component read
2A84090→2D7A750. Both target windows were checked against decoded instructions
and containing .pdata functions; their actual bytes are in r2-signatures.json.
These windows are not hook entries. Their RVAs still need standard support
for the containing function; extracting bytes does not adopt their addresses.

The earlier exploratory 2D740D0 is an actor construction/admission routine
and is rejected as the character-number lookup. A raw displacement search
found the new lead; complete native role review supplied the evidence. That
raw search is not either standard whole-program correlator. This identity
checkpoint brought the catalog to 188 plus retained R1 17, with 46 independent
RVA checks awaiting support; R2-PROGRESS records subsequent catalog additions.

Reproduction: `r2_query.ps1`, old RVAs `2C3E030,2B7F4D0,293FE70,2A83F80`,
new RVAs `2E879A0,2C160B0,B15C50,2D7A640`; entry writers/common loaders old
`2143E70,1791980`, new `237EA60,192EA40`. Small outputs and hashes are in
`port-evidence/r2/identity-*.txt` and `text-hashes.json`. Run
`r2_reviewed.py` then `r2_layout_header.py` to regenerate the catalog.
