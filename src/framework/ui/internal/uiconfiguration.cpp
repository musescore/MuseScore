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

#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QSettings>
#include <QApplication>

#include "uiconfiguration.h"
#include "global/configreader.h"

#include "internal/baseapplication.h"
#include "io/path.h"
#include "io/fileinfo.h"
#include "settings.h"
#include "themeconverter.h"

#include "imainwindow.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace Qt::Literals;
using namespace muse;
using namespace muse::ui;
using namespace muse::async;

static const Settings::Key UI_THEMES_KEY("ui", "ui/application/themes");
static const Settings::Key UI_CURRENT_THEME_CODE_KEY("ui", "ui/application/currentThemeCode");
static const Settings::Key UI_CUSTOM_COLORS_KEY("ui", "ui/application/customColors");
static const Settings::Key UI_FOLLOW_SYSTEM_THEME_KEY("ui", "ui/application/followSystemTheme");
static const Settings::Key UI_FONT_FAMILY_KEY("ui", "ui/theme/fontFamily");
static const Settings::Key UI_FONT_SIZE_KEY("ui", "ui/theme/fontSize");
static const Settings::Key UI_ICONS_FONT_FAMILY_KEY("ui", "ui/theme/iconsFontFamily");
static const Settings::Key UI_MUSICAL_FONT_FAMILY_KEY("ui", "ui/theme/musicalFontFamily");
static const Settings::Key UI_MUSICAL_FONT_SIZE_KEY("ui", "ui/theme/musicalFontSize");
static const Settings::Key UI_MUSICAL_TEXT_FONT_FAMILY_KEY("ui", "ui/theme/musicalTextFontFamily");
static const Settings::Key UI_MUSICAL_TEXT_FONT_SIZE_KEY("ui", "ui/theme/musicalTextFontSize");

static const int FLICKABLE_MAX_VELOCITY = 4000;

static const int TOOLTIP_DELAY = 500;

// read custom colors saved by Qt < 6.9
// see: https://github.com/qt/qtbase/blob/v6.2.4/src/gui/kernel/qplatformdialoghelper.cpp#L292-L302
static std::vector<Val> readLegacyCustomColors()
{
    constexpr size_t customColorCount = 16;

    QSettings settings(QSettings::UserScope, u"QtProject"_s);
    std::vector<Val> legacyValues(customColorCount, Val(QColorConstants::White));
    for (size_t i = 0; i < customColorCount; ++i) {
        const QVariant value = settings.value(u"Qt/customColors/"_s + QString::number(i));
        if (value.isValid()) {
            legacyValues[i] = Val(QColor::fromRgb(value.toUInt()));
        }
    }

    return legacyValues;
}

void UiConfiguration::init()
{
    m_config = ConfigReader::read(":/configs/ui.cfg");

    settings()->setDefaultValue(UI_CURRENT_THEME_CODE_KEY, Val(LIGHT_THEME_CODE));
    settings()->setDefaultValue(UI_CUSTOM_COLORS_KEY, Val(readLegacyCustomColors()));
    settings()->setDefaultValue(UI_FOLLOW_SYSTEM_THEME_KEY, Val(false));
    settings()->setDefaultValue(UI_FONT_FAMILY_KEY, Val(defaultFontFamily()));
    settings()->setDefaultValue(UI_FONT_SIZE_KEY, Val(defaultFontSize()));
    settings()->setDefaultValue(UI_ICONS_FONT_FAMILY_KEY, Val("MusescoreIcon"));
    settings()->setDefaultValue(UI_MUSICAL_FONT_FAMILY_KEY, Val("Leland"));
    settings()->setDefaultValue(UI_MUSICAL_FONT_SIZE_KEY, Val(24));
    settings()->setDefaultValue(UI_MUSICAL_TEXT_FONT_FAMILY_KEY, Val("Leland Text"));
    settings()->setDefaultValue(UI_MUSICAL_TEXT_FONT_SIZE_KEY, Val(defaultFontSize()));
    settings()->setDefaultValue(UI_THEMES_KEY, Val(""));

    settings()->valueChanged(UI_THEMES_KEY).onReceive(this, [this](const Val&) {
        updateThemes();
        notifyAboutCurrentThemeChanged();
    });

    settings()->valueChanged(UI_CURRENT_THEME_CODE_KEY).onReceive(this, [this](const Val&) {
        notifyAboutCurrentThemeChanged();
    });

    settings()->valueChanged(UI_FOLLOW_SYSTEM_THEME_KEY).onReceive(this, [this](const Val& val) {
        m_isFollowSystemTheme.set(val.toBool());
        updateSystemThemeListeningStatus();
    });

    settings()->valueChanged(UI_FONT_FAMILY_KEY).onReceive(this, [this](const Val&) {
        m_fontChanged.notify();
    });

    settings()->valueChanged(UI_FONT_SIZE_KEY).onReceive(this, [this](const Val&) {
        m_fontChanged.notify();
        m_iconsFontChanged.notify();
    });

    settings()->valueChanged(UI_ICONS_FONT_FAMILY_KEY).onReceive(this, [this](const Val&) {
        m_iconsFontChanged.notify();
    });

    settings()->valueChanged(UI_MUSICAL_FONT_FAMILY_KEY).onReceive(this, [this](const Val&) {
        m_musicalFontChanged.notify();
    });

    settings()->valueChanged(UI_MUSICAL_FONT_SIZE_KEY).onReceive(this, [this](const Val&) {
        m_musicalFontChanged.notify();
    });

    platformTheme()->platformThemeChanged().onNotify(this, [this]() {
        synchThemeWithSystemIfNecessary();
    });

    m_themeWatcher.fileChanged().onReceive(this, [this](const std::string&){
        initThemes();
        notifyAboutCurrentThemeChanged();
    });

    correctUserFontIfNeeded();

    initThemes();
}

void UiConfiguration::deinit()
{
    platformTheme()->stopListening();
    m_themeWatcher.stopWatching();
}

void UiConfiguration::initThemes()
{
    m_isFollowSystemTheme.val = settings()->value(UI_FOLLOW_SYSTEM_THEME_KEY).toBool();

    updateSystemThemeListeningStatus();

    for (const ThemeCode& codeKey : allStandardThemeCodes()) {
        m_themes.push_back(makeStandardTheme(codeKey));
    }

    updateThemes();
    updateCurrentTheme();
}

void UiConfiguration::correctUserFontIfNeeded()
{
    QString userFontFamily = QString::fromStdString(fontFamily());
    if (!QFontDatabase::hasFamily(userFontFamily)) {
        std::string fallbackFontFamily = defaultFontFamily();
        LOGI() << "The user font " << userFontFamily << " is missing, we will use the fallback font " << fallbackFontFamily;

        setFontFamily(fallbackFontFamily);
    }
}

void UiConfiguration::updateCurrentTheme()
{
    ThemeCode currentCodeKey = currentThemeCodeKey();

    for (size_t i = 0; i < m_themes.size(); ++i) {
        if (m_themes[i].codeKey == currentCodeKey) {
            m_currentThemeIndex = i;
            break;
        }
    }

    platformTheme()->applyPlatformStyleOnAppForTheme(currentCodeKey);
}

void UiConfiguration::updateThemes()
{
    ThemeList modifiedThemes = readThemes();

    if (modifiedThemes.empty()) {
        m_themes.clear();
        for (const ThemeCode& codeKey : allStandardThemeCodes()) {
            m_themes.push_back(makeStandardTheme(codeKey));
        }

        return;
    }

    for (ThemeInfo& theme: m_themes) {
        auto it = std::find_if(modifiedThemes.begin(), modifiedThemes.end(), [theme](const ThemeInfo& modifiedTheme) {
            return modifiedTheme.codeKey == theme.codeKey;
        });

        bool isModified = it != modifiedThemes.end();
        if (isModified) {
            for (auto key : it->values.keys()) {
                if (key != UNKNOWN) {
                    theme.values[key] = it->values[key];
                }
            }
        }
    }
}

bool UiConfiguration::isFollowSystemThemeAvailable() const
{
    return platformTheme()->isFollowSystemThemeAvailable();
}

ValNt<bool> UiConfiguration::isFollowSystemTheme() const
{
    return m_isFollowSystemTheme;
}

void UiConfiguration::setFollowSystemTheme(bool follow)
{
    settings()->setSharedValue(UI_FOLLOW_SYSTEM_THEME_KEY, Val(follow));
}

void UiConfiguration::updateSystemThemeListeningStatus()
{
    if (isFollowSystemTheme().val) {
        platformTheme()->startListening();
        synchThemeWithSystemIfNecessary();
    } else {
        platformTheme()->stopListening();
    }
}

void UiConfiguration::synchThemeWithSystemIfNecessary()
{
    if (!m_isFollowSystemTheme.val) {
        return;
    }

    doSetIsDarkMode(platformTheme()->isSystemThemeDark());
}

void UiConfiguration::notifyAboutCurrentThemeChanged()
{
    updateCurrentTheme();
    m_currentThemeChanged.notify();
}

static QColor colorFromHex(const QString& hex)
{
    if (!hex.startsWith('#')) {
        return QColor();
    }

    QString str = hex;
    if (str.length() == 9) { // #RRGGBBAA
        // Convert to Qt notation: #AARRGGBB
        QString alpha = str.mid(7, 2);
        str.chop(2);
        str.insert(1, alpha);
    }

    return QColor::fromString(str);
}

ThemeInfo UiConfiguration::makeStandardTheme(const ThemeCode& codeKey) const
{
    ThemeInfo theme;
    theme.codeKey = codeKey;

    io::path_t themeFilePath = globalConfiguration()->appDataPath() + codeKey + ".cfg";

    // Hot reload is disabled in stable builds
    if (muse::BaseApplication::appUnstable() && io::FileInfo::exists(themeFilePath)) {
        m_themeWatcher.startWatching(themeFilePath.toStdString());
    } else {
        themeFilePath = ":/configs/" + codeKey + ".cfg";
    }

    Config config = ConfigReader::read(themeFilePath);

    theme.values = {
        { BACKGROUND_PRIMARY_COLOR, colorFromHex(config.value("background_primary_color").toQString()) },
        { BACKGROUND_SECONDARY_COLOR, colorFromHex(config.value("background_secondary_color").toQString()) },
        { BACKGROUND_TERTIARY_COLOR, colorFromHex(config.value("background_tertiary_color").toQString()) },
        { BACKGROUND_QUARTERNARY_COLOR, colorFromHex(config.value("background_quarternary_color").toQString()) },
        { POPUP_BACKGROUND_COLOR, colorFromHex(config.value("popup_background_color").toQString()) },
        { PROJECT_TAB_COLOR, colorFromHex(config.value("project_tab_color").toQString()) },
        { TEXT_FIELD_COLOR, colorFromHex(config.value("text_field_color").toQString()) },
        { ACCENT_COLOR, colorFromHex(config.value("accent_color").toQString()) },
        { STROKE_COLOR, colorFromHex(config.value("stroke_color").toQString()) },
        { STROKE_SECONDARY_COLOR, colorFromHex(config.value("stroke_secondary_color").toQString()) },
        { BUTTON_COLOR, colorFromHex(config.value("button_color").toQString()) },
        { FONT_PRIMARY_COLOR, colorFromHex(config.value("font_primary_color").toQString()) },
        { FONT_SECONDARY_COLOR, colorFromHex(config.value("font_secondary_color").toQString()) },
        { LINK_COLOR, colorFromHex(config.value("link_color").toQString()) },
        { FOCUS_COLOR, colorFromHex(config.value("focus_color").toQString()) },
        { WHITE_COLOR, colorFromHex(config.value("white_color").toQString()) },
        { BLACK_COLOR, colorFromHex(config.value("black_color").toQString()) },
        { PLAY_COLOR, colorFromHex(config.value("play_color").toQString()) },
        { RECORD_COLOR, colorFromHex(config.value("record_color").toQString()) },

        { BORDER_WIDTH, config.value("border_width").toDouble() },
        { NAVIGATION_CONTROL_BORDER_WIDTH, config.value("navigation_control_border_width").toDouble() },

        { ACCENT_OPACITY_NORMAL, config.value("accent_opacity_normal").toDouble() },
        { ACCENT_OPACITY_HOVER, config.value("accent_opacity_hover").toDouble() },
        { ACCENT_OPACITY_HIT, config.value("accent_opacity_hit").toDouble() },

        { BUTTON_OPACITY_NORMAL, config.value("button_opacity_normal").toDouble() },
        { BUTTON_OPACITY_HOVER, config.value("button_opacity_hover").toDouble() },
        { BUTTON_OPACITY_HIT, config.value("button_opacity_hit").toDouble() },

        { ITEM_OPACITY_DISABLED, config.value("item_opacity_disabled").toDouble() }
    };

    for (const auto& [key, val] : config) {
        if (val.type() == Val::Type::String) {
            QColor color = colorFromHex(val.toQString());
            if (color.isValid()) {
                theme.extra.insert(QString::fromStdString(key), color);
            } else {
                theme.extra.insert(QString::fromStdString(key), val.toQString());
            }
        } else {
            theme.extra.insert(QString::fromStdString(key), val.toDouble());
        }
    }

    return theme;
}

ThemeList UiConfiguration::readThemes() const
{
    TRACEFUNC;

    ThemeList result;

    QByteArray json = QByteArray::fromStdString(settings()->value(UI_THEMES_KEY).toString());
    if (json.isEmpty()) {
        return result;
    }

    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError) {
        LOGE() << "Couldn't read themes: " << err.errorString();
        return result;
    }

    if (!jsodDoc.isArray()) {
        return result;
    }

    QVariantList objList = jsodDoc.array().toVariantList();

    for (const QVariant& themeObj: objList) {
        result.push_back(ThemeConverter::fromMap(themeObj.toMap()));
    }

    return result;
}

void UiConfiguration::writeThemes(const ThemeList& themes)
{
    TRACEFUNC;

    QJsonArray jsonArray;
    for (const ThemeInfo& theme : themes) {
        QVariantMap map = ThemeConverter::toMap(theme);
        jsonArray << QJsonObject::fromVariantMap(map);
    }

    QJsonDocument jsonDoc(jsonArray);

    Val value(jsonDoc.toJson(QJsonDocument::Compact).constData());
    settings()->setSharedValue(UI_THEMES_KEY, value);
}

ThemeList UiConfiguration::themes() const
{
    return m_themes;
}

QStringList UiConfiguration::possibleAccentColors() const
{
    static const QStringList lightAccentColors {
        "#F28585",
        "#EDB17A",
        "#E0CC87",
        "#8BC9C5",
        "#70AFEA",
        "#A09EEF",
        "#DBA0C7"
    };

    static const QStringList darkAccentColors {
        "#F25555",
        "#E1720B",
        "#AC8C1A",
        "#27A341",
        "#2093FE",
        "#926BFF",
        "#E454C4"
    };

    if (isDarkMode()) {
        return darkAccentColors;
    }

    return lightAccentColors;
}

QStringList UiConfiguration::possibleFontFamilies() const
{
    QStringList allFonts = QFontDatabase::families();
    for (const QString& fontFamily : m_nonTextFonts) {
        allFonts.removeAll(fontFamily);
    }
    return allFonts;
}

QStringList UiConfiguration::nonTextFonts() const
{
    return m_nonTextFonts;
}

void UiConfiguration::setNonTextFonts(const QStringList& fontFamilies)
{
    m_nonTextFonts = fontFamilies;
}

void UiConfiguration::resetThemes()
{
    m_themes.clear();
    for (const ThemeCode& codeKey : allStandardThemeCodes()) {
        m_themes.push_back(makeStandardTheme(codeKey));
    }

    writeThemes(m_themes);

    notifyAboutCurrentThemeChanged();
}

const ThemeInfo& UiConfiguration::currentTheme() const
{
    return m_themes[m_currentThemeIndex];
}

ThemeCode UiConfiguration::currentThemeCodeKey() const
{
    ThemeCode preferredThemeCode = settings()->value(UI_CURRENT_THEME_CODE_KEY).toString();
    return preferredThemeCode.empty() ? LIGHT_THEME_CODE : preferredThemeCode;
}

bool UiConfiguration::isDarkMode() const
{
    return isDarkTheme(currentThemeCodeKey());
}

void UiConfiguration::setIsDarkMode(bool dark)
{
    setFollowSystemTheme(false);

    doSetIsDarkMode(dark);
}

void UiConfiguration::doSetIsDarkMode(bool dark)
{
    if (isHighContrast()) {
        doSetCurrentTheme(dark ? HIGH_CONTRAST_BLACK_THEME_CODE : HIGH_CONTRAST_WHITE_THEME_CODE);
    } else {
        doSetCurrentTheme(dark ? DARK_THEME_CODE : LIGHT_THEME_CODE);
    }
}

bool UiConfiguration::isHighContrast() const
{
    return isHighContrastTheme(currentThemeCodeKey());
}

void UiConfiguration::setIsHighContrast(bool highContrast)
{
    if (isDarkMode()) {
        doSetCurrentTheme(highContrast ? HIGH_CONTRAST_BLACK_THEME_CODE : DARK_THEME_CODE);
    } else {
        doSetCurrentTheme(highContrast ? HIGH_CONTRAST_WHITE_THEME_CODE : LIGHT_THEME_CODE);
    }
}

void UiConfiguration::setCurrentTheme(const ThemeCode& themeCode)
{
    setFollowSystemTheme(false);

    doSetCurrentTheme(themeCode);
}

void UiConfiguration::doSetCurrentTheme(const ThemeCode& themeCode)
{
    settings()->setSharedValue(UI_CURRENT_THEME_CODE_KEY, Val(themeCode));
}

void UiConfiguration::setCurrentThemeStyleValue(ThemeStyleKey key, const Val& val)
{
    ThemeInfo& currentTheme = m_themes[m_currentThemeIndex];
    currentTheme.values[key] = val.toQVariant();

    ThemeList modifiedThemes = readThemes();

    auto it = std::find_if(modifiedThemes.begin(), modifiedThemes.end(), [currentTheme](const ThemeInfo& theme) {
        return theme.codeKey == currentTheme.codeKey;
    });

    if (it != modifiedThemes.end()) {
        modifiedThemes.erase(it);
    }

    modifiedThemes.push_back(currentTheme);
    writeThemes(modifiedThemes);
}

muse::async::Notification UiConfiguration::currentThemeChanged() const
{
    return m_currentThemeChanged;
}

std::string UiConfiguration::fontFamily() const
{
    return settings()->value(UI_FONT_FAMILY_KEY).toString();
}

void UiConfiguration::setFontFamily(const std::string& family)
{
    settings()->setSharedValue(UI_FONT_FAMILY_KEY, Val(family));
}

int UiConfiguration::fontSize(FontSizeType type) const
{
    int bodyFontSize = settings()->value(UI_FONT_SIZE_KEY).toInt();

    /*
     * DEFAULT SIZE:
     * body: 12
     * body large: 14
     * tab: 16
     * header: 22
     * title: 32
     */
    switch (type) {
    case FontSizeType::BODY: return bodyFontSize;
    case FontSizeType::BODY_LARGE: return bodyFontSize + bodyFontSize / 6;
    case FontSizeType::TAB: return bodyFontSize + bodyFontSize / 3;
    case FontSizeType::HEADER: return bodyFontSize + bodyFontSize / 1.2;
    case FontSizeType::TITLE: return bodyFontSize + bodyFontSize / 0.6;
    }

    return bodyFontSize;
}

void UiConfiguration::setBodyFontSize(int size)
{
    settings()->setSharedValue(UI_FONT_SIZE_KEY, Val(size));
}

muse::async::Notification UiConfiguration::fontChanged() const
{
    return m_fontChanged;
}

std::string UiConfiguration::iconsFontFamily() const
{
    return settings()->value(UI_ICONS_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::iconsFontSize(IconSizeType type) const
{
    int bodyFontSize = fontSize(FontSizeType::BODY) + 4;

    switch (type) {
    case IconSizeType::Regular: return bodyFontSize;
    case IconSizeType::Toolbar: return bodyFontSize + 2;
    }

    return bodyFontSize;
}

muse::async::Notification UiConfiguration::iconsFontChanged() const
{
    return m_iconsFontChanged;
}

io::path_t UiConfiguration::appIconPath() const
{
    return m_config.value("appIconPath").toPath();
}

std::string UiConfiguration::musicalFontFamily() const
{
    return settings()->value(UI_MUSICAL_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::musicalFontSize() const
{
    return settings()->value(UI_MUSICAL_FONT_SIZE_KEY).toInt();
}

muse::async::Notification UiConfiguration::musicalFontChanged() const
{
    return m_musicalFontChanged;
}

std::string UiConfiguration::musicalTextFontFamily() const
{
    return settings()->value(UI_MUSICAL_TEXT_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::musicalTextFontSize() const
{
    return settings()->value(UI_MUSICAL_TEXT_FONT_SIZE_KEY).toInt();
}

Notification UiConfiguration::musicalTextFontChanged() const
{
    return m_musicalTextFontChanged;
}

std::string UiConfiguration::defaultFontFamily() const
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont).family().toStdString();
}

int UiConfiguration::defaultFontSize() const
{
    return 12;
}

void UiConfiguration::resetFonts()
{
    settings()->setSharedValue(UI_FONT_FAMILY_KEY, settings()->defaultValue(UI_FONT_FAMILY_KEY));
    settings()->setSharedValue(UI_FONT_SIZE_KEY, settings()->defaultValue(UI_FONT_SIZE_KEY));
    settings()->setSharedValue(UI_ICONS_FONT_FAMILY_KEY, settings()->defaultValue(UI_ICONS_FONT_FAMILY_KEY));
    settings()->setSharedValue(UI_MUSICAL_FONT_FAMILY_KEY, settings()->defaultValue(UI_MUSICAL_FONT_FAMILY_KEY));
    settings()->setSharedValue(UI_MUSICAL_FONT_SIZE_KEY, settings()->defaultValue(UI_MUSICAL_FONT_SIZE_KEY));
}

void UiConfiguration::setCustomPhysicalDotsPerInch(std::optional<double> dpi)
{
    m_customDPI = dpi;
}

std::optional<double> UiConfiguration::customPhysicalDotsPerInch() const
{
    return m_customDPI;
}

bool UiConfiguration::isGlobalMenuAvailable() const
{
    return platformTheme()->isGlobalMenuAvailable();
}

bool UiConfiguration::isSystemDragSupported() const
{
#ifdef MUSE_MODULE_UI_SYSTEMDRAG_SUPPORTED
    return true;
#else
    return false;
#endif
}

void UiConfiguration::applyPlatformStyle(QWindow* window)
{
    platformTheme()->applyPlatformStyleOnWindowForTheme(window, currentThemeCodeKey());
}

int UiConfiguration::flickableMaxVelocity() const
{
    return FLICKABLE_MAX_VELOCITY;
}

int UiConfiguration::tooltipDelay() const
{
    return TOOLTIP_DELAY;
}

std::vector<QColor> UiConfiguration::colorDialogCustomColors() const
{
    const ValList colorVals = settings()->value(UI_CUSTOM_COLORS_KEY).toList();

    std::vector<QColor> customColors;
    customColors.reserve(colorVals.size());
    for (const auto& colorVal : colorVals) {
        customColors.push_back(colorVal.toQColor());
    }

    return customColors;
}

void UiConfiguration::setColorDialogCustomColors(const std::vector<QColor>& customColors)
{
    ValList colorVals;
    colorVals.reserve(customColors.size());
    for (const auto& color: customColors) {
        colorVals.emplace_back(color);
    }

    settings()->setLocalValue(UI_CUSTOM_COLORS_KEY, Val(colorVals));
}
