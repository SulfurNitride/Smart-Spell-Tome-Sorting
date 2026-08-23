#include "TomeMetadata.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>

namespace
{
    bool ExpectEqual(std::string_view a_actual, std::string_view a_expected, std::string_view a_case)
    {
        if (a_actual == a_expected) {
            return true;
        }

        std::cerr << a_case << ": expected \"" << a_expected << "\", got \"" << a_actual << "\"\n";
        return false;
    }

    bool ExpectTrue(const bool a_result, std::string_view a_case)
    {
        if (a_result) {
            return true;
        }

        std::cerr << a_case << ": check failed\n";
        return false;
    }

    constexpr bool KeyLess(const smart_tomes::SortKey& a_left, const smart_tomes::SortKey& a_right)
    {
        if (a_left.learnedOrder != a_right.learnedOrder) {
            return a_left.learnedOrder < a_right.learnedOrder;
        }
        if (a_left.tomeOrder != a_right.tomeOrder) {
            return a_left.tomeOrder < a_right.tomeOrder;
        }
        if (a_left.schoolOrder != a_right.schoolOrder) {
            return a_left.schoolOrder < a_right.schoolOrder;
        }
        return a_left.rankOrder < a_right.rankOrder;
    }
}

int main()
{
    using smart_tomes::FormatTomeName;
    using smart_tomes::FormatSpellName;
    using smart_tomes::MakeOtherBookSortKey;
    using smart_tomes::MakeTomeSortKey;
    using smart_tomes::Rank;
    using smart_tomes::RankFromMinimumSkill;
    using smart_tomes::School;

    bool passed = true;
    passed &= ExpectTrue(RankFromMinimumSkill(0) == Rank::kNovice, "zero is novice");
    passed &= ExpectTrue(RankFromMinimumSkill(24) == Rank::kNovice, "24 is novice");
    passed &= ExpectTrue(RankFromMinimumSkill(25) == Rank::kApprentice, "25 is apprentice");
    passed &= ExpectTrue(RankFromMinimumSkill(50) == Rank::kAdept, "50 is adept");
    passed &= ExpectTrue(RankFromMinimumSkill(75) == Rank::kExpert, "75 is expert");
    passed &= ExpectTrue(RankFromMinimumSkill(100) == Rank::kMaster, "100 is master");
    passed &= ExpectTrue(RankFromMinimumSkill(150) == Rank::kMaster, "over 100 remains master");

    passed &= ExpectEqual(
        FormatTomeName(School::kAlteration, Rank::kNovice, "Candlelight"),
        "Alt. Nov. Spell Tome: Candlelight",
        "compact alteration label");
    passed &= ExpectEqual(
        FormatTomeName(School::kDestruction, Rank::kApprentice, "Firebolt"),
        "Dest. Appr. Spell Tome: Firebolt",
        "compact destruction label");
    passed &= ExpectEqual(
        FormatSpellName(School::kDestruction, Rank::kNovice, "Flames"),
        "Dest. Nov. Spell: Flames",
        "compact Magic menu label");

    const auto unknownNovice = MakeTomeSortKey(false, School::kAlteration, Rank::kNovice);
    const auto unknownApprentice = MakeTomeSortKey(false, School::kAlteration, Rank::kApprentice);
    const auto unknownDestruction = MakeTomeSortKey(false, School::kDestruction, Rank::kNovice);
    const auto otherBook = MakeOtherBookSortKey();
    const auto learnedNovice = MakeTomeSortKey(true, School::kAlteration, Rank::kNovice);

    passed &= ExpectTrue(KeyLess(unknownNovice, unknownApprentice), "novice precedes apprentice");
    passed &= ExpectTrue(KeyLess(unknownApprentice, unknownDestruction), "school groups precede later schools");
    passed &= ExpectTrue(KeyLess(unknownDestruction, otherBook), "unknown tomes precede ordinary books");
    passed &= ExpectTrue(KeyLess(otherBook, learnedNovice), "learned tomes sort last");

    return passed ? 0 : 1;
}
