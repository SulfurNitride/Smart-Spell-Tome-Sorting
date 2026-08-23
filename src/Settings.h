#pragma once

#include <filesystem>

namespace smart_tomes
{
    struct Settings
    {
        bool compactTomeNames{ true };
        bool prefixMagicSpellNames{ true };
        bool sortVendorMenu{ true };
        bool sortMagicMenu{ true };
        bool knownSpellTomesShowAsRead{ true };
    };

    [[nodiscard]] const Settings& GetSettings() noexcept;
    void LoadSettings(const std::filesystem::path& a_path);
}
