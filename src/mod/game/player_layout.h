#pragma once
// One reader, explicit per-build data. No fallback from a recognized new build
// to legacy offsets. A 3663 table is published only with complete R2 A proofs.
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

inline const PlayerLayout* player_layout(const build_guard::ImageIdentity& id) {
    if (id.kind == build_guard::IdentityKind::Supported && id.timestamp_known &&
        id.timestamp == build_2976::kPeTimestamp && id.mapped_base == build_2976::kImageBase)
        return &kPlayerLayout2976;
    // Prevent enabling A by editing a boolean while its table is still absent.
    static_assert(!build_3663::kTierAComplete, "Publish the proved 3663 PlayerLayout before arming Tier A");
    return nullptr;
}
} // namespace cdmp::game
