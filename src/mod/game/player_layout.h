#pragma once
// One reader, explicit per-build data. No fallback from a recognized new build
// to legacy offsets. The HP-only 3663 scope follows the approved R3 rules.
#include "mod/game/build_guard.h"
#include "mod/game/health_definition.h"
#include "mod/game/offsets.h"

namespace cdmp::game {
struct PlayerLayout {
#define CDMP_PLAYER_FIELD(name) decltype(build_2976::name) name = build_2976::name
    CDMP_PLAYER_FIELD(kWorldSystemSlot);
    CDMP_PLAYER_FIELD(kChunkOrigin);
    CDMP_PLAYER_FIELD(kStaticPlayerPos);
    CDMP_PLAYER_FIELD(kVtClientActorManager);
    CDMP_PLAYER_FIELD(kVtControlledActor);
    CDMP_PLAYER_FIELD(kVtCharacterControl);
    CDMP_PLAYER_FIELD(kVtStatusComponent);
    CDMP_PLAYER_FIELD(kVtPhysicsObject);
    CDMP_PLAYER_FIELD(kWorldSystem_ActorManager);
    CDMP_PLAYER_FIELD(kActorManager_ControlledActor);
    CDMP_PLAYER_FIELD(kActor_ComponentTable);
    CDMP_PLAYER_FIELD(kComponents_Status);
    CDMP_PLAYER_FIELD(kComponents_CharacterControl);
    CDMP_PLAYER_FIELD(kControl_Inner);
    CDMP_PLAYER_FIELD(kControlInner_Physics);
    CDMP_PLAYER_FIELD(kPhysics_AxisX);
    CDMP_PLAYER_FIELD(kPhysics_AxisY);
    CDMP_PLAYER_FIELD(kPhysics_AxisZ);
    CDMP_PLAYER_FIELD(kPhysics_Position);
    CDMP_PLAYER_FIELD(kStatus_StatContainer);
    CDMP_PLAYER_FIELD(kStatContainer_Entries);
    CDMP_PLAYER_FIELD(kStatContainer_Count);
    CDMP_PLAYER_FIELD(kStatEntry_Size);
    CDMP_PLAYER_FIELD(kStatEntry_Id);
    CDMP_PLAYER_FIELD(kStatEntry_Current);
    CDMP_PLAYER_FIELD(kStatEntry_Max);
    CDMP_PLAYER_FIELD(kStatId_Health);
    CDMP_PLAYER_FIELD(kStatId_Stamina);
    CDMP_PLAYER_FIELD(kMaxStatEntries);
    CDMP_PLAYER_FIELD(kLoadingParkHeight);
    CDMP_PLAYER_FIELD(kLoadingParkRadius);
#undef CDMP_PLAYER_FIELD
    const float stat_units = 1000.0f; // native 2976 unit conversion
    const uint32_t stat_id_bytes = 8; // preserve the legacy reader's width
    // Presence selects the HP-only native reader. A target table must supply
    // adopted slots/hops; missing or inconsistent definitions mean not-ready.
    std::optional<HealthDefinitionLayout> native_hp_definitions;
};
inline constexpr PlayerLayout kPlayerLayout2976{};

// R3 approved HP-only scope: A026 dynamic definitions; A027 retired; A028 guarded park/chain/HP readiness.
inline constexpr PlayerLayout kPlayerLayout3663{
    .kWorldSystemSlot = build_3663::core_rvas::A031_kWorldSystemSlot,
    .kChunkOrigin = build_3663::core_rvas::A001_kChunkOrigin,
    .kStaticPlayerPos = build_3663::core_rvas::A002_kStaticPlayerPos,
    .kVtClientActorManager = build_3663::core_rvas::A003_kVtClientActorManager,
    .kVtControlledActor = build_3663::core_rvas::A004_kVtControlledActor,
    .kVtCharacterControl = build_3663::core_rvas::A005_kVtCharacterControl,
    .kVtStatusComponent = build_3663::core_rvas::A006_kVtStatusComponent,
    .kVtPhysicsObject = build_3663::core_rvas::A007_kVtPhysicsObject,
    .kWorldSystem_ActorManager = build_3663::player_layout::kWorldSystem_ActorManager,
    .kActorManager_ControlledActor = build_3663::player_layout::kActorManager_ControlledActor,
    .kActor_ComponentTable = build_3663::player_layout::kActor_ComponentTable,
    .kComponents_Status = build_3663::player_layout::kComponents_Status,
    .kComponents_CharacterControl = build_3663::player_layout::kComponents_CharacterControl,
    .kControl_Inner = build_3663::player_layout::kControl_Inner,
    .kControlInner_Physics = build_3663::player_layout::kControlInner_Physics,
    .kPhysics_AxisX = build_3663::player_layout::kPhysics_AxisX,
    .kPhysics_AxisY = build_3663::player_layout::kPhysics_AxisY,
    .kPhysics_AxisZ = build_3663::player_layout::kPhysics_AxisZ,
    .kPhysics_Position = build_3663::player_layout::kPhysics_Position,
    .kStatus_StatContainer = build_3663::player_layout::kStatus_StatContainer,
    .kStatContainer_Entries = build_3663::player_layout::kStatContainer_Entries,
    .kStatContainer_Count = build_3663::player_layout::kStatContainer_Count,
    .kStatEntry_Size = build_3663::player_layout::kStatEntry_Size,
    .kStatEntry_Id = build_3663::player_layout::kStatEntry_Id,
    .kStatEntry_Current = build_3663::player_layout::kStatEntry_Current,
    .kStatEntry_Max = build_3663::player_layout::kStatEntry_Max,
    .kStatId_Health = 0,
    .kStatId_Stamina = 0,
    .kMaxStatEntries = 64,
    .kLoadingParkHeight = 1000.0f,
    .kLoadingParkRadius = 5.0f,
    .stat_units = build_3663::player_layout::kNativeStatUnits,
    .stat_id_bytes = build_3663::player_layout::kStatIdReadWidth,
    .native_hp_definitions = HealthDefinitionLayout{
        build_3663::core_rvas::A032_kHpDefinitionsSlot, build_3663::core_rvas::A033_kHpDefinitionsAltSlot,
        build_3663::player_layout::kHpDefinitionsRoot, build_3663::player_layout::kHpDefinitionsTables,
        build_3663::player_layout::kHpDefinitionsObject, build_3663::player_layout::kHpDefinitionId},
};


inline const PlayerLayout* player_layout(const build_guard::ImageIdentity& id) {
    if (id.kind == build_guard::IdentityKind::Supported && id.timestamp_known &&
        id.timestamp == build_2976::kPeTimestamp && id.mapped_base == build_2976::kImageBase)
        return &kPlayerLayout2976;
    if (build_guard::select_3663_tiers(id, {true,false,false,false},
            {build_3663::kTierAComplete,false,false,false}).tier_a_reads)
        return &kPlayerLayout3663;
    return nullptr;
}
} // namespace cdmp::game
