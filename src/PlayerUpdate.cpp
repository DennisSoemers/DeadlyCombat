#include <SKSE/SKSE.h>

#include "PlayerUpdate.h"

using namespace PlayerUpdate;

OnPlayerUpdate& OnPlayerUpdate::GetSingleton() noexcept {
    static OnPlayerUpdate instance;
    return instance;
}

void OnPlayerUpdate::InstallHook() {
    REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{RE::VTABLE_PlayerCharacter[0]};
    _Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
}

void OnPlayerUpdate::Update(RE::PlayerCharacter* player, float delta) {
    if (!RE::UI::GetSingleton()->GameIsPaused()) {
        auto& Singleton = GetSingleton();

        if (player->IsBlocking()) {
            Singleton.playerTimeSpentBlocking += delta;
            Singleton.playerTimeSpentCoolingDown = 0.f;
        } else if (Singleton.playerTimeSpentBlocking > 0.f) {
            Singleton.playerTimeSpentCoolingDown += delta;
            if (Singleton.playerTimeSpentCoolingDown >= 0.25f) {
                // Finished cooling down
                Singleton.playerTimeSpentBlocking = 0.f;
            }
        }
    }

    // Call the normal game's code
    _Update(player, delta);
}