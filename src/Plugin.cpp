#include "Settings.h"
#include "TomeMetadata.h"

namespace
{
    constexpr std::size_t kAdvanceMovieVtableIndex = 0x05;

    constexpr std::uint32_t kSortCaseInsensitive = 1;
    constexpr std::uint32_t kSortNumeric = 16;
    constexpr std::uint32_t kBookReadFlag = 0x08;

    constexpr auto kLearnedField = "sstsLearnedOrder";
    constexpr auto kTomeField = "sstsTomeOrder";
    constexpr auto kSchoolField = "sstsSchoolOrder";
    constexpr auto kRankField = "sstsRankOrder";
    constexpr auto kNameField = "sstsSpellName";
    constexpr auto kSortAttributesMember = "__sstsSortAttributes";
    constexpr auto kSortOptionsMember = "__sstsSortOptions";
    constexpr auto kSortAppliedMember = "__sstsSortApplied";

    constexpr auto kMagicSchoolField = "sstsMagicSchoolOrder";
    constexpr auto kMagicRankField = "sstsMagicRankOrder";
    constexpr auto kMagicNameField = "sstsMagicSpellName";
    constexpr auto kMagicPreparedField = "__sstsMagicPrepared";
    constexpr auto kMagicFavoriteField = "__sstsMagicFavorite";
    constexpr auto kMagicSortAttributesMember = "__sstsMagicSortAttributes";
    constexpr auto kMagicSortOptionsMember = "__sstsMagicSortOptions";
    constexpr auto kMagicSortAppliedMember = "__sstsMagicSortApplied";
    constexpr auto kMagicOriginalInvalidateMember = "__sstsOriginalInvalidateListData";
    constexpr auto kMagicInvalidateWrappedMember = "__sstsInvalidateListDataWrapped";

    constexpr std::array<const char*, 5> kVendorSortAttributes = {
        kLearnedField,
        kTomeField,
        kSchoolField,
        kRankField,
        kNameField
    };
    constexpr std::array<std::uint32_t, 5> kVendorSortOptions = {
        kSortNumeric,
        kSortNumeric,
        kSortNumeric,
        kSortNumeric,
        kSortCaseInsensitive
    };
    constexpr std::array<const char*, 3> kMagicSortAttributes = {
        kMagicSchoolField,
        kMagicRankField,
        kMagicNameField
    };
    constexpr std::array<std::uint32_t, 3> kMagicSortOptions = {
        kSortNumeric,
        kSortNumeric,
        kSortCaseInsensitive
    };

    bool g_reportedUnsupportedMenu{ false };
    bool g_reportedMagicWrapperFailure{ false };

    void InitializeLogging()
    {
        auto logDirectory = SKSE::log::log_directory();
        if (!logDirectory) {
            return;
        }

        *logDirectory /= "SmartSpellTomeSorting.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory->string(), true);
        auto logger = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

        spdlog::set_default_logger(std::move(logger));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
    }

    [[nodiscard]] smart_tomes::School GetSchool(const RE::ActorValue a_skill) noexcept
    {
        switch (a_skill) {
        case RE::ActorValue::kAlteration:
            return smart_tomes::School::kAlteration;
        case RE::ActorValue::kConjuration:
            return smart_tomes::School::kConjuration;
        case RE::ActorValue::kDestruction:
            return smart_tomes::School::kDestruction;
        case RE::ActorValue::kIllusion:
            return smart_tomes::School::kIllusion;
        case RE::ActorValue::kRestoration:
            return smart_tomes::School::kRestoration;
        default:
            return smart_tomes::School::kOther;
        }
    }

    [[nodiscard]] smart_tomes::Rank GetRank(RE::SpellItem& a_spell) noexcept
    {
        const auto effect = a_spell.GetCostliestEffectItem();
        const auto minimumSkill = effect && effect->baseEffect ? effect->baseEffect->GetMinimumSkillLevel() : 0;
        return smart_tomes::RankFromMinimumSkill(minimumSkill);
    }

    void SetNumberMember(RE::GFxValue& a_object, const char* a_name, const std::uint8_t a_value)
    {
        a_object.SetMember(a_name, RE::GFxValue(a_value));
    }

    void SetManagedStringMember(
        RE::GFxMovie& a_view,
        RE::GFxValue& a_object,
        const char* a_name,
        const std::string_view a_value)
    {
        RE::GFxValue managedString;
        a_view.CreateString(&managedString, std::string(a_value).c_str());
        a_object.SetMember(a_name, managedString);
    }

    void SetDefaultSortFields(RE::GFxValue& a_object)
    {
        const auto key = smart_tomes::MakeOtherBookSortKey();
        SetNumberMember(a_object, kLearnedField, key.learnedOrder);
        SetNumberMember(a_object, kTomeField, key.tomeOrder);
        SetNumberMember(a_object, kSchoolField, key.schoolOrder);
        SetNumberMember(a_object, kRankField, key.rankOrder);

        RE::GFxValue text;
        if (a_object.GetMember("text", &text) && (text.IsString() || text.IsStringW())) {
            a_object.SetMember(kNameField, text);
        } else {
            a_object.SetMember(kNameField, RE::GFxValue(""));
        }
    }

    void MarkTomeEntryRead(RE::GFxValue& a_object)
    {
        RE::GFxValue flags;
        if (a_object.GetMember("flags", &flags) && flags.IsNumber()) {
            const auto updatedFlags = static_cast<std::uint32_t>(flags.GetUInt()) | kBookReadFlag;
            a_object.SetMember("flags", RE::GFxValue(updatedFlags));
        }
        a_object.SetMember("isRead", RE::GFxValue(true));
    }

    void ExtendInventoryEntry(
        RE::GFxMovieView* a_view,
        RE::GFxValue* a_object,
        RE::InventoryEntryData* a_item)
    {
        if (!a_view || !a_object || !a_object->IsObject() || !a_item) {
            return;
        }

        SetDefaultSortFields(*a_object);

        const auto book = a_item->GetObject() ? a_item->GetObject()->As<RE::TESObjectBOOK>() : nullptr;
        if (!book || !book->TeachesSpell()) {
            return;
        }

        const auto spell = book->GetSpell();
        if (!spell) {
            return;
        }

        const auto spellName = std::string_view(spell->GetName() ? spell->GetName() : "");
        if (spellName.empty()) {
            return;
        }

        const auto school = GetSchool(spell->GetAssociatedSkill());
        const auto rank = GetRank(*spell);
        const auto player = RE::PlayerCharacter::GetSingleton();
        const auto learned = player && player->HasSpell(spell);
        const auto key = smart_tomes::MakeTomeSortKey(learned, school, rank);

        SetNumberMember(*a_object, kLearnedField, key.learnedOrder);
        SetNumberMember(*a_object, kTomeField, key.tomeOrder);
        SetNumberMember(*a_object, kSchoolField, key.schoolOrder);
        SetNumberMember(*a_object, kRankField, key.rankOrder);
        SetManagedStringMember(*a_view, *a_object, kNameField, spellName);

        const auto& settings = smart_tomes::GetSettings();
        if (learned && settings.knownSpellTomesShowAsRead) {
            MarkTomeEntryRead(*a_object);
        }

        if (settings.compactTomeNames) {
            const auto displayName = smart_tomes::FormatTomeName(school, rank, spellName);
            SetManagedStringMember(*a_view, *a_object, "text", displayName);
        }
    }

    [[nodiscard]] bool GetMemberObject(const RE::GFxValue& a_parent, const char* a_name, RE::GFxValue& a_result)
    {
        return a_parent.GetMember(a_name, &a_result) && a_result.IsObject();
    }

    [[nodiscard]] bool IsViewingVendorItems(const RE::GFxValue& a_inventoryLists)
    {
        RE::GFxValue categoryList;
        RE::GFxValue activeSegment;
        return GetMemberObject(a_inventoryLists, "categoryList", categoryList) &&
               categoryList.GetMember("activeSegment", &activeSegment) && activeSegment.IsNumber() &&
               activeSegment.GetUInt() == 0;
    }

    [[nodiscard]] bool IsPlainNameSort(const RE::GFxValue& a_layout)
    {
        RE::GFxValue attributes;
        if (!a_layout.GetMember("sortAttributes", &attributes) || !attributes.IsArray() ||
            attributes.GetArraySize() != 1) {
            return false;
        }

        RE::GFxValue primaryAttribute;
        return attributes.GetElement(0, &primaryAttribute) && primaryAttribute.IsString() &&
               std::string_view(primaryAttribute.GetString() ? primaryAttribute.GetString() : "") == "text";
    }

    [[nodiscard]] bool IsSmartSortApplied(
        const RE::GFxValue& a_inventoryLists,
        const char* a_appliedMember)
    {
        RE::GFxValue applied;
        return a_inventoryLists.GetMember(a_appliedMember, &applied) && applied.IsBool() && applied.GetBool();
    }

    void RestoreNativeSort(
        RE::GFxValue& a_inventoryLists,
        RE::GFxValue& a_layout,
        RE::GFxValue& a_sortFilter,
        const char* a_appliedMember)
    {
        if (!IsSmartSortApplied(a_inventoryLists, a_appliedMember)) {
            return;
        }

        RE::GFxValue attributes;
        RE::GFxValue options;
        if (a_layout.GetMember("sortAttributes", &attributes) && attributes.IsArray() &&
            a_layout.GetMember("sortOptions", &options) && options.IsArray()) {
            const std::array arguments{ attributes, options };
            a_sortFilter.Invoke("setSortBy", arguments);
        }

        a_inventoryLists.SetMember(a_appliedMember, RE::GFxValue(false));
    }

    template <std::size_t N>
    [[nodiscard]] bool GetOrCreateSortArrays(
        RE::GFxMovieView& a_view,
        RE::GFxValue& a_inventoryLists,
        const char* a_attributesMember,
        const char* a_optionsMember,
        const std::array<const char*, N>& a_attributeNames,
        const std::array<std::uint32_t, N>& a_optionValues,
        RE::GFxValue& a_attributes,
        RE::GFxValue& a_options)
    {
        if (a_inventoryLists.GetMember(a_attributesMember, &a_attributes) && a_attributes.IsArray() &&
            a_inventoryLists.GetMember(a_optionsMember, &a_options) && a_options.IsArray()) {
            return true;
        }

        a_view.CreateArray(&a_attributes);
        a_view.CreateArray(&a_options);
        if (!a_attributes.IsArray() || !a_options.IsArray()) {
            return false;
        }

        for (const auto attribute : a_attributeNames) {
            RE::GFxValue value;
            a_view.CreateString(&value, attribute);
            a_attributes.PushBack(value);
        }
        for (const auto option : a_optionValues) {
            a_options.PushBack(RE::GFxValue(option));
        }

        a_inventoryLists.SetMember(a_attributesMember, a_attributes);
        a_inventoryLists.SetMember(a_optionsMember, a_options);
        return true;
    }

    void ApplySmartSort(RE::BarterMenu* a_menu)
    {
        if (!a_menu || !a_menu->uiMovie) {
            return;
        }

        auto& root = a_menu->GetRuntimeData().root;
        RE::GFxValue inventoryLists;
        RE::GFxValue itemList;
        RE::GFxValue layout;
        RE::GFxValue sortFilter;
        if (!GetMemberObject(root, "inventoryLists", inventoryLists) ||
            !GetMemberObject(inventoryLists, "itemList", itemList) ||
            !GetMemberObject(itemList, "layout", layout) ||
            !GetMemberObject(inventoryLists, "_sortFilter", sortFilter)) {
            return;
        }

        const auto shouldUseSmartSort = IsViewingVendorItems(inventoryLists) &&
                                        IsPlainNameSort(layout);
        if (!shouldUseSmartSort) {
            RestoreNativeSort(inventoryLists, layout, sortFilter, kSortAppliedMember);
            return;
        }

        RE::GFxValue attributes;
        RE::GFxValue options;
        if (!GetOrCreateSortArrays(
                *a_menu->uiMovie,
                inventoryLists,
                kSortAttributesMember,
                kSortOptionsMember,
                kVendorSortAttributes,
                kVendorSortOptions,
                attributes,
                options)) {
            return;
        }

        const std::array arguments{ attributes, options };
        if (sortFilter.Invoke("setSortBy", arguments)) {
            inventoryLists.SetMember(kSortAppliedMember, RE::GFxValue(true));
        } else if (!g_reportedUnsupportedMenu) {
            g_reportedUnsupportedMenu = true;
            SKSE::log::warn("The loaded Barter Menu does not expose SkyUI's compatible sort filter");
        }
    }

    [[nodiscard]] bool IsMagicSpellCategory(const RE::GFxValue& a_inventoryLists)
    {
        RE::GFxValue categoryList;
        RE::GFxValue selectedEntry;
        RE::GFxValue flag;
        if (!GetMemberObject(a_inventoryLists, "categoryList", categoryList) ||
            !GetMemberObject(categoryList, "selectedEntry", selectedEntry) ||
            !selectedEntry.GetMember("flag", &flag) || !flag.IsNumber()) {
            return false;
        }

        switch (flag.GetSInt()) {
        case -257:  // All magic except active effects
        case 2:     // Alteration
        case 4:     // Illusion
        case 8:     // Destruction
        case 16:    // Conjuration
        case 32:    // Restoration
            return true;
        default:
            return false;
        }
    }

    void SetMagicEntryDefaults(RE::GFxValue& a_entry)
    {
        SetNumberMember(a_entry, kMagicSchoolField, 100);
        SetNumberMember(a_entry, kMagicRankField, 100);

        RE::GFxValue text;
        if (a_entry.GetMember("text", &text) && (text.IsString() || text.IsStringW())) {
            a_entry.SetMember(kMagicNameField, text);
        } else {
            a_entry.SetMember(kMagicNameField, RE::GFxValue(""));
        }
    }

    [[nodiscard]] bool PrepareMagicEntries(RE::GFxMovie& a_view, RE::GFxValue& a_itemList)
    {
        RE::GFxValue entries;
        if (!a_itemList.GetMember("entryList", &entries) || !entries.IsArray()) {
            return false;
        }

        bool changed = false;
        for (std::uint32_t index = 0; index < entries.GetArraySize(); ++index) {
            RE::GFxValue entry;
            if (!entries.GetElement(index, &entry) || !entry.IsObject()) {
                continue;
            }

            RE::GFxValue formID;
            if (!entry.GetMember("formId", &formID) || !formID.IsNumber()) {
                continue;
            }
            const auto currentFormID = static_cast<RE::FormID>(formID.GetUInt());

            RE::GFxValue favorite;
            const auto isFavorite = entry.GetMember("favorite", &favorite) &&
                                    ((favorite.IsBool() && favorite.GetBool()) ||
                                     (favorite.IsNumber() && favorite.GetNumber() != 0.0));

            RE::GFxValue preparedFavorite;
            const auto favoriteChanged =
                !entry.GetMember(kMagicFavoriteField, &preparedFavorite) || !preparedFavorite.IsBool() ||
                preparedFavorite.GetBool() != isFavorite;
            if (favoriteChanged) {
                entry.SetMember(kMagicFavoriteField, RE::GFxValue(isFavorite));
                changed = true;
            }

            RE::GFxValue prepared;
            if (entry.GetMember(kMagicPreparedField, &prepared) && prepared.IsNumber() &&
                prepared.GetUInt() == currentFormID) {
                continue;
            }

            SetMagicEntryDefaults(entry);

            const auto spell = RE::TESForm::LookupByID<RE::SpellItem>(currentFormID);
            if (spell && spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell) {
                const auto spellName = std::string_view(spell->GetName() ? spell->GetName() : "");
                const auto school = GetSchool(spell->GetAssociatedSkill());
                const auto rank = GetRank(*spell);
                SetNumberMember(entry, kMagicSchoolField, static_cast<std::uint8_t>(school));
                SetNumberMember(entry, kMagicRankField, static_cast<std::uint8_t>(rank));
                SetManagedStringMember(a_view, entry, kMagicNameField, spellName);
                if (smart_tomes::GetSettings().prefixMagicSpellNames) {
                    const auto displayName = smart_tomes::FormatSpellName(school, rank, spellName);
                    SetManagedStringMember(a_view, entry, "text", displayName);
                }
            }

            entry.SetMember(kMagicPreparedField, RE::GFxValue(currentFormID));
            changed = true;
        }

        return changed;
    }

    class MagicListInvalidationHandler final : public RE::GFxFunctionHandler
    {
    public:
        void Call(Params& a_params) override
        {
            if (!a_params.thisPtr || !a_params.thisPtr->IsObject()) {
                return;
            }

            RE::GFxValue itemList;
            if (a_params.movie && GetMemberObject(*a_params.thisPtr, "itemList", itemList)) {
                static_cast<void>(PrepareMagicEntries(*a_params.movie, itemList));
            }

            if (!a_params.thisPtr->Invoke(
                    kMagicOriginalInvalidateMember,
                    a_params.retVal,
                    a_params.args,
                    a_params.argCount) &&
                !g_reportedMagicWrapperFailure) {
                g_reportedMagicWrapperFailure = true;
                SKSE::log::warn("Could not invoke SkyUI's original Magic-list invalidation callback");
            }
        }
    };

    RE::GPtr<MagicListInvalidationHandler> g_magicListInvalidationHandler;

    void InstallMagicListInvalidationWrapper(RE::GFxMovieView& a_view, RE::GFxValue& a_inventoryLists)
    {
        RE::GFxValue wrapped;
        if (a_inventoryLists.GetMember(kMagicInvalidateWrappedMember, &wrapped) && wrapped.IsBool() &&
            wrapped.GetBool()) {
            return;
        }

        RE::GFxValue original;
        if (!a_inventoryLists.GetMember("InvalidateListData", &original)) {
            return;
        }

        if (!a_inventoryLists.SetMember(kMagicOriginalInvalidateMember, original)) {
            return;
        }

        if (!g_magicListInvalidationHandler) {
            g_magicListInvalidationHandler.reset(new MagicListInvalidationHandler());
        }

        RE::GFxValue replacement;
        a_view.CreateFunction(&replacement, g_magicListInvalidationHandler.get());
        if (!a_inventoryLists.SetMember("InvalidateListData", replacement)) {
            return;
        }

        a_inventoryLists.SetMember(kMagicInvalidateWrappedMember, RE::GFxValue(true));
    }

    void UpdateMagicMenu(RE::MagicMenu* a_menu)
    {
        if (!a_menu || !a_menu->uiMovie) {
            return;
        }

        auto& root = a_menu->GetRuntimeData().root;
        RE::GFxValue inventoryLists;
        RE::GFxValue itemList;
        RE::GFxValue layout;
        RE::GFxValue sortFilter;
        if (!GetMemberObject(root, "inventoryLists", inventoryLists) ||
            !GetMemberObject(inventoryLists, "itemList", itemList) ||
            !GetMemberObject(itemList, "layout", layout) ||
            !GetMemberObject(inventoryLists, "_sortFilter", sortFilter)) {
            return;
        }

        InstallMagicListInvalidationWrapper(*a_menu->uiMovie, inventoryLists);
        const auto metadataChanged = PrepareMagicEntries(*a_menu->uiMovie, itemList);

        const auto shouldUseSmartSort = smart_tomes::GetSettings().sortMagicMenu &&
                                        IsMagicSpellCategory(inventoryLists) && IsPlainNameSort(layout);
        if (!shouldUseSmartSort) {
            RestoreNativeSort(inventoryLists, layout, sortFilter, kMagicSortAppliedMember);
            if (metadataChanged && smart_tomes::GetSettings().prefixMagicSpellNames) {
                itemList.Invoke("InvalidateData");
            }
            return;
        }

        RE::GFxValue attributes;
        RE::GFxValue options;
        if (!GetOrCreateSortArrays(
                *a_menu->uiMovie,
                inventoryLists,
                kMagicSortAttributesMember,
                kMagicSortOptionsMember,
                kMagicSortAttributes,
                kMagicSortOptions,
                attributes,
                options)) {
            return;
        }

        const std::array arguments{ attributes, options };
        if (sortFilter.Invoke("setSortBy", arguments)) {
            inventoryLists.SetMember(kMagicSortAppliedMember, RE::GFxValue(true));
            if (metadataChanged) {
                // SkyUI may update a favorite in place without re-running its
                // cached sort filter. Rebuild synchronously when row metadata
                // or favorite state changes so its delayed requestInvalidate
                // does not expose native names/order for several frames.
                itemList.Invoke("InvalidateData");
            }
        }
    }

    struct BarterMenuAdvanceHook
    {
        static void Thunk(RE::BarterMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
        {
            _original(a_menu, a_interval, a_currentTime);
            ApplySmartSort(a_menu);
        }

        static void Install()
        {
            REL::Relocation<std::uintptr_t> vtable{ RE::VTABLE_BarterMenu[0] };
            _original = vtable.write_vfunc(kAdvanceMovieVtableIndex, Thunk);
            SKSE::log::info("Installed BarterMenu smart-sort hook");
        }

        static inline REL::Relocation<decltype(Thunk)> _original;
    };

    struct MagicMenuAdvanceHook
    {
        static void Thunk(RE::MagicMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
        {
            _original(a_menu, a_interval, a_currentTime);
            UpdateMagicMenu(a_menu);
        }

        static void Install()
        {
            REL::Relocation<std::uintptr_t> vtable{ RE::VTABLE_MagicMenu[0] };
            _original = vtable.write_vfunc(kAdvanceMovieVtableIndex, Thunk);
            SKSE::log::info("Installed MagicMenu smart-sort hook");
        }

        static inline REL::Relocation<decltype(Thunk)> _original;
    };
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitializeLogging();
    SKSE::Init(a_skse);

    SKSE::log::info("Smart Spell Tome Sorting v{} loading", SMART_TOME_SORTING_VERSION);
    smart_tomes::LoadSettings("Data/SKSE/Plugins/SmartSpellTomeSorting.toml");

    const auto scaleform = SKSE::GetScaleformInterface();
    if (!scaleform) {
        SKSE::log::critical("SKSE Scaleform interface is unavailable");
        return false;
    }

    scaleform->Register(ExtendInventoryEntry);
    if (smart_tomes::GetSettings().sortVendorMenu) {
        BarterMenuAdvanceHook::Install();
    }
    if (smart_tomes::GetSettings().sortMagicMenu || smart_tomes::GetSettings().prefixMagicSpellNames) {
        MagicMenuAdvanceHook::Install();
    }
    SKSE::log::info("Smart Spell Tome Sorting loaded");
    return true;
}
