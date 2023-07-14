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
                    // Timed block against NPC: no damage
                    hitData.totalDamage = 0.f;

                    auto source = hitData.sourceRef.get().get();
                    bool isMelee = true;

                    if (source) {
                        auto sourceWeapon = source->As<RE::TESObjectWEAP>();
                        auto weaponType = sourceWeapon->GetWeaponType();
                        isMelee = (weaponType != RE::WEAPON_TYPE::kBow && weaponType != RE::WEAPON_TYPE::kCrossbow);
                    }

                    if (isMelee) {
                        if (hitData.flags.any(RE::HitData::Flag::kPowerAttack)) {
                            // Power attack: attacker recoils
                            SKSE::GetTaskInterface()->AddTask([aggressor]() { 
                                aggressor->NotifyAnimationGraph("recoilLargeStart");
                            });
                            
                        } else {
                            // Not power attack: attacker staggers: thanks Valhalla code
                            SKSE::GetTaskInterface()->AddTask(
                                [aggressor, victim]() { 
                                    auto headingAngle = aggressor->GetHeadingAngle(victim->GetPosition(), false);
                                    auto direction = (headingAngle >= 0.0f) ? headingAngle / 360.0f : (360.0f + headingAngle) / 360.0f;
                                    aggressor->SetGraphVariableFloat("staggerDirection", direction);
                                    aggressor->SetGraphVariableFloat("StaggerMagnitude", 0.3f);
                                    aggressor->NotifyAnimationGraph("staggerStart");
                                });
                        }
                    }
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
                victim->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -extraStaminaCost);
            }
        }
    }

    // Call the normal game's code
    _ProcessHit(victim, hitData);
}
