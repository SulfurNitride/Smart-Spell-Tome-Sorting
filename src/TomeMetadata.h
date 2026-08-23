#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace smart_tomes
{
    enum class School : std::uint8_t
    {
        kAlteration,
        kConjuration,
        kDestruction,
        kIllusion,
        kRestoration,
        kOther
    };

    enum class Rank : std::uint8_t
    {
        kNovice,
        kApprentice,
        kAdept,
        kExpert,
        kMaster
    };

    struct SortKey
    {
        std::uint8_t learnedOrder;
        std::uint8_t tomeOrder;
        std::uint8_t schoolOrder;
        std::uint8_t rankOrder;
    };

    [[nodiscard]] Rank RankFromMinimumSkill(std::int32_t a_minimumSkill) noexcept;
    [[nodiscard]] SortKey MakeTomeSortKey(bool a_learned, School a_school, Rank a_rank) noexcept;
    [[nodiscard]] SortKey MakeOtherBookSortKey() noexcept;
    [[nodiscard]] std::string FormatTomeName(School a_school, Rank a_rank, std::string_view a_spellName);
    [[nodiscard]] std::string FormatSpellName(School a_school, Rank a_rank, std::string_view a_spellName);
}
