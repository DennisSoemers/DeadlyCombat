#pragma once

#include <RE/Skyrim.h>

namespace HitHook {
#pragma warning(push)
#pragma warning(disable : 4251)

    class __declspec(dllexport) OnMeleeHit {
    public:
        [[nodiscard]] static OnMeleeHit& GetSingleton() noexcept;

        static void InstallHook();

    private:
        OnMeleeHit() = default;
        OnMeleeHit(const OnMeleeHit&) = delete;
        OnMeleeHit(OnMeleeHit&&) = delete;
        ~OnMeleeHit() = default;

        OnMeleeHit& operator=(const OnMeleeHit&) = delete;
        OnMeleeHit& operator=(OnMeleeHit&&) = delete;

        /** Our new function */
        static void ProcessHit(RE::Actor* victim, RE::HitData& hitData);

        /** The original function */
        static inline REL::Relocation<decltype(ProcessHit)> _ProcessHit;
    };

#pragma warning(pop)
}  // namespace HitHook