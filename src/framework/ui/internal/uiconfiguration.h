/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "iuiconfiguration.h"

#include "iglobalconfiguration.h"
#include "global/types/config.h"
#include "modularity/ioc.h"
#include "internal/iplatformtheme.h"
#include "io/filewatcher.h"

#include "types/val.h"
#include "async/asyncable.h"

namespace muse::ui {
class UiConfiguration : public IUiConfiguration, public async::Asyncable
{
    GlobalInject<IPlatformTheme> platformTheme;
    GlobalInject<IGlobalConfiguration> globalConfiguration;

public:

    UiConfiguration() = default;

    void init();
    void deinit();

    ThemeList themes() const override;
    QStringList possibleAccentColors() const override;
    QStringList possibleFontFamilies() const override;
    QStringList nonTextFonts() const override;
    void setNonTextFonts(const QStringList& fontFamilies) override;

    bool isDarkMode() const override;
    void setIsDarkMode(bool dark) override;

    bool isHighContrast() const override;
    void setIsHighContrast(bool highContrast) override;

    const ThemeInfo& currentTheme() const override;
    async::Notification currentThemeChanged() const override;
    void setCurrentTheme(const ThemeCode& codeKey) override;
    void setCurrentThemeStyleValue(ThemeStyleKey key, const Val& val) override;
    void resetThemes() override;

    bool isFollowSystemThemeAvailable() const override;
    ValNt<bool> isFollowSystemTheme() const override;
    void setFollowSystemTheme(bool follow) override;

    std::string fontFamily() const override;
    void setFontFamily(const std::string& family) override;
    int fontSize(FontSizeType type = FontSizeType::BODY) const override;
    void setBodyFontSize(int size) override;
    async::Notification fontChanged() const override;

    std::string iconsFontFamily() const override;
    int iconsFontSize(IconSizeType type) const override;
    async::Notification iconsFontChanged() const override;

    io::path_t appIconPath() const override;

    std::string musicalFontFamily() const override;
    int musicalFontSize() const override;
    async::Notification musicalFontChanged() const override;

    std::string musicalTextFontFamily() const override;
    int musicalTextFontSize() const override;
    async::Notification musicalTextFontChanged() const override;

    QFont defaultFont() const override;

    void resetFonts() override;

    void setCustomPhysicalDotsPerInch(std::optional<double> dpi) override;
    std::optional<double> customPhysicalDotsPerInch() const override;

    bool isGlobalMenuAvailable() const override;
    bool isSystemDragSupported() const override;

    void applyPlatformStyle(QWindow* window) override;

    int flickableMaxVelocity() const override;

    int tooltipDelay() const override;

    std::vector<QColor> colorDialogCustomColors() const override;
    void setColorDialogCustomColors(const std::vector<QColor>&) override;

private:
    void initThemes();
    void correctUserFontIfNeeded();

    void notifyAboutCurrentThemeChanged();

    void updateCurrentTheme();
    void updateThemes();

    void updateSystemThemeListeningStatus();
    void synchThemeWithSystemIfNecessary();

    void doSetIsDarkMode(bool dark);
    void doSetCurrentTheme(const ThemeCode& themeCode);

    ThemeCode currentThemeCodeKey() const;
    ThemeInfo makeStandardTheme(const ThemeCode& codeKey) const;

    ThemeList readThemes() const;
    void writeThemes(const ThemeList& themes);

    async::Notification m_currentThemeChanged;
    async::Notification m_fontChanged;
    async::Notification m_musicalFontChanged;
    async::Notification m_musicalTextFontChanged;
    async::Notification m_iconsFontChanged;

    ValNt<bool> m_isFollowSystemTheme;

    ThemeList m_themes;
    size_t m_currentThemeIndex = 0;
    std::optional<double> m_customDPI;

    QStringList m_nonTextFonts;

    Config m_config;

    mutable io::FileWatcher m_themeWatcher;
};
}
