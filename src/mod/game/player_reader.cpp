#include "mod/game/player_reader.h"

#include <windows.h>

#include <cmath>

#include "common/math3d.h"
#include "mod/game/player_layout.h"
#include "mod/game/safe_memory.h"
#include "mod/game/stat_id.h"

namespace cdmp::game {

namespace {

std::optional<proto::Vec3> read_vec3(uintptr_t addr) {
    float f[3];
    if (!safe_read(addr, f, sizeof f)) return std::nullopt;
    proto::Vec3 v{f[0], f[1], f[2]};
    if (!math::is_finite(v)) return std::nullopt;
    return v;
}

}  // namespace

bool PlayerReader::init() {
    base_ = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    const auto identity = build_guard::identify_mapped(base_,
        [](uintptr_t address, void* out, size_t size) { return safe_read(address, out, size); });
    layout_ = player_layout(identity);
    enabled_ = layout_ != nullptr;
    if (!enabled_) last_failure_ = "unsupported game build";
    return enabled_;
}

std::nullopt_t PlayerReader::fail(const char* why) {
    last_failure_ = why;
    return std::nullopt;
}

std::optional<uintptr_t> PlayerReader::resolve_actor() {
    auto world = read_ptr(base_ + layout_->kWorldSystemSlot);
    if (!world) return fail("WorldSystem not created");
    auto manager = read_ptr(*world + layout_->kWorldSystem_ActorManager);
    if (!manager || !has_vtable(*manager, base_ + layout_->kVtClientActorManager))
        return fail("ClientActorManager missing");
    auto actor = read_ptr(*manager + layout_->kActorManager_ControlledActor);
    if (!actor) return fail("no controlled actor (not in world)");
    if (!has_vtable(*actor, base_ + layout_->kVtControlledActor))
        return fail("controlled actor has unexpected type");
    return actor;
}

std::optional<uintptr_t> PlayerReader::character_control() {
    if (!enabled_) return fail("unsupported game build");
    auto actor = resolve_actor();
    if (!actor) return std::nullopt;
    auto components = read_ptr(*actor + layout_->kActor_ComponentTable);
    if (!components) return fail("component table missing");
    auto control = read_ptr(*components + layout_->kComponents_CharacterControl);
    if (!control || !has_vtable(*control, base_ + layout_->kVtCharacterControl))
        return fail("CharacterControl component missing");
    return control;
}

std::optional<PlayerSnapshot> PlayerReader::read() {
    if (!enabled_) return fail("unsupported game build");

    auto actor = resolve_actor();
    if (!actor) return std::nullopt;

    auto components = read_ptr(*actor + layout_->kActor_ComponentTable);
    if (!components) return fail("component table missing");
    auto control = read_ptr(*components + layout_->kComponents_CharacterControl);
    if (!control || !has_vtable(*control, base_ + layout_->kVtCharacterControl))
        return fail("CharacterControl component missing");
    auto inner = read_ptr(*control + layout_->kControl_Inner);
    if (!inner) return fail("control inner missing");
    auto physics = read_ptr(*inner + layout_->kControlInner_Physics);
    if (!physics || !has_vtable(*physics, base_ + layout_->kVtPhysicsObject))
        return fail("physics object missing");

    auto local = read_vec3(*physics + layout_->kPhysics_Position);
    auto origin = read_vec3(base_ + layout_->kChunkOrigin);
    auto ax = read_vec3(*physics + layout_->kPhysics_AxisX);
    auto ay = read_vec3(*physics + layout_->kPhysics_AxisY);
    auto az = read_vec3(*physics + layout_->kPhysics_AxisZ);
    if (!local || !origin || !ax || !ay || !az) return fail("physics data unreadable");
    if (!math::is_orthonormal(*ax, *ay, *az)) return fail("physics basis not a rotation");

    PlayerSnapshot snap;
    snap.position = math::add(*local, *origin);
    snap.local_position = *local;
    snap.chunk_origin = *origin;
    snap.rotation = math::quat_from_axes(*ax, *ay, *az);

    // The game keeps its own world-space copy; a large mismatch means our
    // chunk-origin math or chain is off (it lags by a frame, so allow some).
    if (auto global = read_vec3(base_ + layout_->kStaticPlayerPos)) {
        snap.static_pos_error = math::length(math::sub(snap.position, *global));
    }

    // While a save loads, the game parks the actor at (0, 1000, 0) for tens
    // of seconds before placing it. Don't broadcast that as a real position.
    if (std::fabs(snap.position.x) < layout_->kLoadingParkRadius &&
        std::fabs(snap.position.z) < layout_->kLoadingParkRadius &&
        std::fabs(snap.position.y - layout_->kLoadingParkHeight) < layout_->kLoadingParkRadius) {
        return fail("loading (actor parked at 0,1000,0)");
    }

    const bool stats_ready = read_stats(*components, snap);
    if (layout_->native_hp_definitions && !stats_ready)
        return fail("HP definitions or unique native stat not ready");
    if (layout_->native_hp_definitions &&
        (resolve_actor() != actor || read_ptr(*actor + layout_->kActor_ComponentTable) != components ||
         read_ptr(*components + layout_->kComponents_CharacterControl) != control ||
         read_ptr(*control + layout_->kControl_Inner) != inner ||
         read_ptr(*inner + layout_->kControlInner_Physics) != physics))
        return fail("player chain changed during snapshot");
    last_failure_.clear();
    return snap;
}

bool PlayerReader::read_stats(uintptr_t components, PlayerSnapshot& out) {
    auto status = read_ptr(components + layout_->kComponents_Status);
    if (!status || !has_vtable(*status, base_ + layout_->kVtStatusComponent)) return false;
    auto container = read_ptr(*status + layout_->kStatus_StatContainer);
    if (!container) return false;
    auto entries = read_ptr(*container + layout_->kStatContainer_Entries);
    auto count = game::read<uint32_t>(*container + layout_->kStatContainer_Count);
    if (!entries || !count || *count > layout_->kMaxStatEntries) return false;

    if (layout_->native_hp_definitions) {
        const auto read = [](uintptr_t address, void* buffer, size_t size) {
            return safe_read(address, buffer, size);
        };
        const auto& definition = *layout_->native_hp_definitions;
        auto before = consensus_health_definition(base_, definition, read);
        if (!before) return false;
        auto health = read_unique_native_health(*entries, *count, layout_->kStatEntry_Size,
            layout_->kStatEntry_Id, layout_->kStatEntry_Current, layout_->kStatEntry_Max,
            before->normal.key, layout_->stat_units, read);
        auto after = consensus_health_definition(base_, definition, read);
        if (!health || !after || *before != *after ||
            read_ptr(components + layout_->kComponents_Status) != status ||
            read_ptr(*status + layout_->kStatus_StatContainer) != container ||
            read_ptr(*container + layout_->kStatContainer_Entries) != entries ||
            game::read<uint32_t>(*container + layout_->kStatContainer_Count) != count) return false;
        out.health = health->current;
        out.health_max = health->maximum;
        // This native path intentionally reads HP only; no unproved stamina ID.
        return true;
    }

    // Look stats up by id: the entry order changes between patches.
    for (uint32_t i = 0; i < *count; ++i) {
        const uintptr_t e = *entries + static_cast<uintptr_t>(i) * layout_->kStatEntry_Size;
        auto id = read_stat_id(e + layout_->kStatEntry_Id, layout_->stat_id_bytes,
            [](uintptr_t address, void* out, size_t size) { return safe_read(address, out, size); });
        if (!id) continue;
        float* cur_out = nullptr;
        float* max_out = nullptr;
        if (*id == layout_->kStatId_Health) {
            cur_out = &out.health;
            max_out = &out.health_max;
        } else if (*id == layout_->kStatId_Stamina) {
            cur_out = &out.stamina;
            max_out = &out.stamina_max;
        } else {
            continue;
        }
        auto cur = game::read<int64_t>(e + layout_->kStatEntry_Current);
        auto max = game::read<int64_t>(e + layout_->kStatEntry_Max);
        if (!cur || !max || *max <= 0 || *cur < 0 || *cur > *max) continue;
        *cur_out = static_cast<float>(*cur) / layout_->stat_units;
        *max_out = static_cast<float>(*max) / layout_->stat_units;
    }
    return true;
}

}  // namespace cdmp::game
