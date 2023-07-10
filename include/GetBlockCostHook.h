#pragma once

#include <RE/Skyrim.h>

namespace BlockCostHook {
#pragma warning(push)
#pragma warning(disable : 4251)

    class __declspec(dllexport) GetBlockCostHook {
    public:
        [[nodiscard]] static GetBlockCostHook& GetSingleton() noexcept;

        static void InstallHook();

    private:
        GetBlockCostHook() = default;
        GetBlockCostHook(const GetBlockCostHook&) = delete;
        GetBlockCostHook(GetBlockCostHook&&) = delete;
        ~GetBlockCostHook() = default;

        GetBlockCostHook& operator=(const GetBlockCostHook&) = delete;
        GetBlockCostHook& operator=(GetBlockCostHook&&) = delete;

        /** Our new function */
        static float GetBlockCost(RE::HitData& hitData);

        /** The original function */
        static inline REL::Relocation<decltype(GetBlockCost)> _GetBlockCost;
    };

#pragma warning(pop)
}  // namespace BlockCostHook