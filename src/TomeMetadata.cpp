#include "TomeMetadata.h"

#include <array>

namespace smart_tomes
{
    namespace
    {
        constexpr std::array<std::string_view, 6> kSchoolLabels = {
            "Alt.",
            "Conj.",
            "Dest.",
            "Ill.",
            "Rest.",
            "Magic"
        };

        constexpr std::array<std::string_view, 5> kRankLabels = {
            "Nov.",
            "Appr.",
            "Adept",
            "Expert",
            "Master"
        };
    }

    Rank RankFromMinimumSkill(const std::int32_t a_minimumSkill) noexcept
    {
        if (a_minimumSkill >= 100) {
            return Rank::kMaster;
        }
        if (a_minimumSkill >= 75) {
            return Rank::kExpert;
        }
        if (a_minimumSkill >= 50) {
            return Rank::kAdept;
        }
        if (a_minimumSkill >= 25) {
            return Rank::kApprentice;
        }
        return Rank::kNovice;
    }

    SortKey MakeTomeSortKey(const bool a_learned, const School a_school, const Rank a_rank) noexcept
    {
        return {
            static_cast<std::uint8_t>(a_learned ? 1 : 0),
            0,
            static_cast<std::uint8_t>(a_school),
            static_cast<std::uint8_t>(a_rank)
        };
    }

    SortKey MakeOtherBookSortKey() noexcept
    {
        return { 0, 1, 0, 0 };
    }

    std::string FormatTomeName(const School a_school, const Rank a_rank, const std::string_view a_spellName)
    {
        const auto school = kSchoolLabels[static_cast<std::size_t>(a_school)];
        const auto rank = kRankLabels[static_cast<std::size_t>(a_rank)];

        std::string result;
        result.reserve(school.size() + rank.size() + a_spellName.size() + 14);
        result.append(school);
        result.push_back(' ');
        result.append(rank);
        result.append(" Spell Tome: ");
        result.append(a_spellName);
        return result;
    }

    std::string FormatSpellName(const School a_school, const Rank a_rank, const std::string_view a_spellName)
    {
        const auto school = kSchoolLabels[static_cast<std::size_t>(a_school)];
        const auto rank = kRankLabels[static_cast<std::size_t>(a_rank)];

        std::string result;
        result.reserve(school.size() + rank.size() + a_spellName.size() + 9);
        result.append(school);
        result.push_back(' ');
        result.append(rank);
        result.append(" Spell: ");
        result.append(a_spellName);
        return result;
    }
}
