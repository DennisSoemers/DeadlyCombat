#include <SKSE/SKSE.h>

#include "OnMeleeHitHook.h"
#include "PlayerUpdate.h"

using namespace HitHook;

OnMeleeHit& OnMeleeHit::GetSingleton() noexcept {
    static OnMeleeHit instance;
    return instance;
}

void OnMeleeHit::InstallHook() {
    SKSE::AllocTrampoline(1 << 4);

    REL::Relocation<uintptr_t> hook{RELOCATION_ID(37673, 38627)};
    auto& trampoline = SKSE::GetTrampoline();
    _ProcessHit = trampoline.write_call<5>(hook.address() + REL::Relocate(0x3C0, 0x4A8), ProcessHit);
}

void OnMeleeHit::ProcessHit(RE::Actor* victim, RE::HitData& hitData) {
    if (victim && victim->IsPlayerRef()) {
        if (hitData.flags.any(RE::HitData::Flag::kBlocked)) {
            const float timeSpentBlocking = PlayerUpdate::OnPlayerUpdate::GetSingleton().playerTimeSpentBlocking;
            if (timeSpentBlocking > 0.f && timeSpentBlocking < 0.25f) {
                auto aggressor = hitData.aggressor.get().get();
                if (aggressor && aggressor->HasKeywordString("ActorTypeNPC")) {
                    // Timed block against NPC
                    hitData.totalDamage = 0.f;
                }
            } else if (timeSpentBlocking > 1.f) {
                float staminaCostTimeBlockingMult = timeSpentBlocking;
                if (staminaCostTimeBlockingMult > 3.f) {
                    staminaCostTimeBlockingMult = 3.f;
                }

                const float shieldPerksAV = victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kShieldPerks);
                float staminaCostPerksMult = 3.2f - 2.f * shieldPerksAV;
                if (staminaCostPerksMult < 0.f) {
                    staminaCostPerksMult = 0.f;
                }

                const float extraStaminaCost = staminaCostTimeBlockingMult * staminaCostPerksMult * 15.f;
                victim->AsActorValueOwner()->ModActorValue(RE::ActorValue::kStamina, -extraStaminaCost);
            }
        }
    }

    // Call the normal game's code
    _ProcessHit(victim, hitData);
}