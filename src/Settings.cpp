#include "Settings.h"

#include <fstream>

namespace smart_tomes
{
    namespace
    {
        Settings g_settings;

        [[nodiscard]] std::string_view Trim(std::string_view a_value) noexcept
        {
            constexpr std::string_view whitespace = " \t\r\n";
            const auto first = a_value.find_first_not_of(whitespace);
            if (first == std::string_view::npos) {
                return {};
            }
            return a_value.substr(first, a_value.find_last_not_of(whitespace) - first + 1);
        }

        void SetBoolean(std::string_view a_key, const bool a_value)
        {
            if (a_key == "CompactTomeNames") {
                g_settings.compactTomeNames = a_value;
            } else if (a_key == "PrefixMagicSpellNames") {
                g_settings.prefixMagicSpellNames = a_value;
            } else if (a_key == "SortVendorMenu") {
                g_settings.sortVendorMenu = a_value;
            } else if (a_key == "SortMagicMenu") {
                g_settings.sortMagicMenu = a_value;
            } else if (a_key == "KnownSpellTomesShowAsRead") {
                g_settings.knownSpellTomesShowAsRead = a_value;
            }
        }
    }

    const Settings& GetSettings() noexcept
    {
        return g_settings;
    }

    void LoadSettings(const std::filesystem::path& a_path)
    {
        g_settings = {};

        std::ifstream input(a_path);
        if (!input) {
            SKSE::log::warn("Settings file not found at {}; using defaults", a_path.string());
            return;
        }

        std::string line;
        std::size_t lineNumber = 0;
        while (std::getline(input, line)) {
            ++lineNumber;
            const auto comment = line.find('#');
            const auto content = Trim(std::string_view(line).substr(0, comment));
            if (content.empty()) {
                continue;
            }

            const auto separator = content.find('=');
            if (separator == std::string_view::npos) {
                SKSE::log::warn("Ignoring malformed setting on line {}", lineNumber);
                continue;
            }

            const auto key = Trim(content.substr(0, separator));
            const auto value = Trim(content.substr(separator + 1));
            if (value == "true") {
                SetBoolean(key, true);
            } else if (value == "false") {
                SetBoolean(key, false);
            } else {
                SKSE::log::warn("Ignoring non-boolean setting '{}' on line {}", key, lineNumber);
            }
        }

        SKSE::log::info(
            "Settings: compact tome names={}, prefixed Magic names={}, vendor sort={}, Magic menu sort={}, known tomes show as read={}",
            g_settings.compactTomeNames,
            g_settings.prefixMagicSpellNames,
            g_settings.sortVendorMenu,
            g_settings.sortMagicMenu,
            g_settings.knownSpellTomesShowAsRead);
    }
}
