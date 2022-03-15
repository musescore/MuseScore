/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "uiconfiguration.h"

#include "settings.h"
#include "log.h"
#include "translation.h"
#include "themeconverter.h"

#include <QScreen>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace mu::ui;
using namespace mu::framework;
using namespace mu::async;

static const Settings::Key UI_THEMES_KEY("ui", "ui/application/themes");
static const Settings::Key UI_CURRENT_THEME_CODE_KEY("ui", "ui/application/currentThemeCode");
static const Settings::Key UI_FONT_FAMILY_KEY("ui", "ui/theme/fontFamily");
static const Settings::Key UI_FONT_SIZE_KEY("ui", "ui/theme/fontSize");
static const Settings::Key UI_ICONS_FONT_FAMILY_KEY("ui", "ui/theme/iconsFontFamily");
static const Settings::Key UI_MUSICAL_FONT_FAMILY_KEY("ui", "ui/theme/musicalFontFamily");
static const Settings::Key UI_MUSICAL_FONT_SIZE_KEY("ui", "ui/theme/musicalFontSize");

static const QString WINDOW_GEOMETRY_KEY("window");

static const int FLICKABLE_MAX_VELOCITY = 1500;

static const QMap<ThemeStyleKey, QVariant> LIGHT_THEME_VALUES {
    { BACKGROUND_PRIMARY_COLOR, "#F5F5F6" },
    { BACKGROUND_SECONDARY_COLOR, "#E6E9ED" },
    { POPUP_BACKGROUND_COLOR, "#F5F5F6" },
    { TEXT_FIELD_COLOR, "#FFFFFF" },
    { ACCENT_COLOR, "#70AFEA" },
    { STROKE_COLOR, "#CED1D4" },
    { BUTTON_COLOR, "#CFD5DD" },
    { FONT_PRIMARY_COLOR, "#111132" },
    { FONT_SECONDARY_COLOR, "#FFFFFF" },
    { LINK_COLOR, "#70AFEA" },
    { FOCUS_COLOR, "#75507b" },

    { BORDER_WIDTH, 0 },
    { NAVIGATION_CONTROL_BORDER_WIDTH, 2.0 },

    { ACCENT_OPACITY_NORMAL, 0.5 },
    { ACCENT_OPACITY_HOVER, 0.3 },
    { ACCENT_OPACITY_HIT, 0.7 },

    { BUTTON_OPACITY_NORMAL, 0.7 },
    { BUTTON_OPACITY_HOVER, 0.5 },
    { BUTTON_OPACITY_HIT, 1.0 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

static const QMap<ThemeStyleKey, QVariant> DARK_THEME_VALUES {
    { BACKGROUND_PRIMARY_COLOR, "#2D2D30" },
    { BACKGROUND_SECONDARY_COLOR, "#363638" },
    { POPUP_BACKGROUND_COLOR, "#323236" },
    { TEXT_FIELD_COLOR, "#242427" },
    { ACCENT_COLOR, "#FF4848" },
    { STROKE_COLOR, "#1E1E1E" },
    { BUTTON_COLOR, "#595959" },
    { FONT_PRIMARY_COLOR, "#EBEBEB" },
    { FONT_SECONDARY_COLOR, "#BDBDBD" },
    { LINK_COLOR, "#70AFEA" },
    { FOCUS_COLOR, "#75507b" },

    { BORDER_WIDTH, 0 },
    { NAVIGATION_CONTROL_BORDER_WIDTH, 2.0 },

    { ACCENT_OPACITY_NORMAL, 0.5 },
    { ACCENT_OPACITY_HOVER, 0.3 },
    { ACCENT_OPACITY_HIT, 0.7 },

    { BUTTON_OPACITY_NORMAL, 0.7 },
    { BUTTON_OPACITY_HOVER, 0.5 },
    { BUTTON_OPACITY_HIT, 1.0 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

static const QMap<ThemeStyleKey, QVariant> HIGH_CONTRAST_BLACK_THEME_VALUES {
    { BACKGROUND_PRIMARY_COLOR, "#000000" },
    { BACKGROUND_SECONDARY_COLOR, "#000000" },
    { POPUP_BACKGROUND_COLOR, "#000000" },
    { TEXT_FIELD_COLOR, "#000000" },
    { ACCENT_COLOR, "#0071DA" },
    { STROKE_COLOR, "#FFFFFF" },
    { BUTTON_COLOR, "#000000" },
    { FONT_PRIMARY_COLOR, "#FFFD38" },
    { FONT_SECONDARY_COLOR, "#BDBDBD" },
    { LINK_COLOR, "#70AFEA" },
    { FOCUS_COLOR, "#75507b" },

    { BORDER_WIDTH, 1.0 },
    { NAVIGATION_CONTROL_BORDER_WIDTH, 2.0 },

    { ACCENT_OPACITY_NORMAL, 0.5 },
    { ACCENT_OPACITY_HOVER, 0.3 },
    { ACCENT_OPACITY_HIT, 0.7 },

    { BUTTON_OPACITY_NORMAL, 0.7 },
    { BUTTON_OPACITY_HOVER, 0.5 },
    { BUTTON_OPACITY_HIT, 1.0 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

static const QMap<ThemeStyleKey, QVariant> HIGH_CONTRAST_WHITE_THEME_VALUES {
    { BACKGROUND_PRIMARY_COLOR, "#FFFFFF" },
    { BACKGROUND_SECONDARY_COLOR, "#FFFFFF" },
    { POPUP_BACKGROUND_COLOR, "#FFFFFF" },
    { TEXT_FIELD_COLOR, "#FFFFFF" },
    { ACCENT_COLOR, "#00D87D" },
    { STROKE_COLOR, "#000000" },
    { BUTTON_COLOR, "#FFFFFF" },
    { FONT_PRIMARY_COLOR, "#1E0073" },
    { FONT_SECONDARY_COLOR, "#000000" },
    { LINK_COLOR, "#70AFEA" },
    { FOCUS_COLOR, "#75507b" },

    { BORDER_WIDTH, 1.0 },
    { NAVIGATION_CONTROL_BORDER_WIDTH, 2.0 },

    { ACCENT_OPACITY_NORMAL, 0.5 },
    { ACCENT_OPACITY_HOVER, 0.3 },
    { ACCENT_OPACITY_HIT, 0.7 },

    { BUTTON_OPACITY_NORMAL, 0.7 },
    { BUTTON_OPACITY_HOVER, 0.5 },
    { BUTTON_OPACITY_HIT, 1.0 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

void UiConfiguration::init()
{
    settings()->setDefaultValue(UI_CURRENT_THEME_CODE_KEY, Val(LIGHT_THEME_CODE));
    settings()->setDefaultValue(UI_FONT_FAMILY_KEY, Val("Fira Sans"));
    settings()->setDefaultValue(UI_FONT_SIZE_KEY, Val(12));
    settings()->setDefaultValue(UI_ICONS_FONT_FAMILY_KEY, Val("MusescoreIcon"));
    settings()->setDefaultValue(UI_MUSICAL_FONT_FAMILY_KEY, Val("Leland"));
    settings()->setDefaultValue(UI_MUSICAL_FONT_SIZE_KEY, Val(24));
    settings()->setDefaultValue(UI_THEMES_KEY, Val(""));

    settings()->valueChanged(UI_THEMES_KEY).onReceive(this, [this](const Val&) {
        updateThemes();
        notifyAboutCurrentThemeChanged();
    });

    settings()->valueChanged(UI_CURRENT_THEME_CODE_KEY).onReceive(this, [this](const Val&) {
        notifyAboutCurrentThemeChanged();
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

    m_uiArrangement.stateChanged(WINDOW_GEOMETRY_KEY).onNotify(this, [this]() {
        m_windowGeometryChanged.notify();
    });

    initThemes();
}

void UiConfiguration::load()
{
    m_uiArrangement.load();
}

void UiConfiguration::deinit()
{
    platformTheme()->stopListening();
}

bool UiConfiguration::needFollowSystemTheme() const
{
    return settings()->value(UI_CURRENT_THEME_CODE_KEY).isNull()
           && platformTheme()->isFollowSystemThemeAvailable();
}

void UiConfiguration::initThemes()
{
    platformTheme()->themeCodeChanged().onReceive(nullptr, [this](ThemeCode) {
        notifyAboutCurrentThemeChanged();
    });

    for (const ThemeCode& codeKey : allStandardThemeCodes()) {
        m_themes.push_back(makeStandardTheme(codeKey));
    }

    updateThemes();
    updateCurrentTheme();
}

void UiConfiguration::updateCurrentTheme()
{
    if (needFollowSystemTheme()) {
        platformTheme()->startListening();
    } else {
        platformTheme()->stopListening();
    }

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
            theme = *it;
        }
    }
}

void UiConfiguration::notifyAboutCurrentThemeChanged()
{
    updateCurrentTheme();
    m_currentThemeChanged.notify();
}

ThemeInfo UiConfiguration::makeStandardTheme(const ThemeCode& codeKey) const
{
    ThemeInfo theme;
    theme.codeKey = codeKey;

    if (codeKey == LIGHT_THEME_CODE) {
        theme.title = trc("ui", "Light");
        theme.values = LIGHT_THEME_VALUES;
    } else if (codeKey == DARK_THEME_CODE) {
        theme.title = trc("ui", "Dark");
        theme.values = DARK_THEME_VALUES;
    } else if (codeKey == HIGH_CONTRAST_WHITE_THEME_CODE) {
        theme.title = trc("ui", "White");
        theme.values = HIGH_CONTRAST_WHITE_THEME_VALUES;
    } else if (codeKey == HIGH_CONTRAST_BLACK_THEME_CODE) {
        theme.title = trc("ui", "Black");
        theme.values = HIGH_CONTRAST_BLACK_THEME_VALUES;
    }

    return theme;
}

ThemeList UiConfiguration::readThemes() const
{
    TRACEFUNC;

    ThemeList result;

    QByteArray json = settings()->value(UI_THEMES_KEY).toQString().toUtf8();
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

QStringList UiConfiguration::possibleFontFamilies() const
{
    QFontDatabase db;
    return db.families();
}

QStringList UiConfiguration::possibleAccentColors() const
{
    static const QStringList lightAccentColors {
        "#F36565",
        "#F39048",
        "#FFC52F",
        "#63D47B",
        "#70AFEA",
        "#A488F2",
        "#F87BDC"
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

    if (currentTheme().codeKey == DARK_THEME_CODE) {
        return darkAccentColors;
    }

    return lightAccentColors;
}

void UiConfiguration::resetCurrentThemeToDefault(const ThemeCode& codeKey)
{
    for (ThemeInfo& theme : m_themes) {
        if (theme.codeKey != codeKey) {
            continue;
        }

        theme = makeStandardTheme(codeKey);
        writeThemes(m_themes);

        if (theme.codeKey == currentThemeCodeKey()) {
            notifyAboutCurrentThemeChanged();
        }

        return;
    }
}

const ThemeInfo& UiConfiguration::currentTheme() const
{
    return m_themes[m_currentThemeIndex];
}

ThemeCode UiConfiguration::currentThemeCodeKey() const
{
    if (needFollowSystemTheme()) {
        return platformTheme()->themeCode();
    }

    ThemeCode preferredThemeCode = settings()->value(UI_CURRENT_THEME_CODE_KEY).toString();

    return preferredThemeCode.empty() ? LIGHT_THEME_CODE : preferredThemeCode;
}

bool UiConfiguration::isHighContrast() const
{
    ThemeCode currentThemeCode = currentThemeCodeKey();

    return currentThemeCode == HIGH_CONTRAST_WHITE_THEME_CODE || currentThemeCode == HIGH_CONTRAST_BLACK_THEME_CODE;
}

void UiConfiguration::setIsHighContrast(bool highContrast)
{
    ThemeCode currentThemeCode = currentThemeCodeKey();
    if (highContrast) {
        if (currentThemeCode == LIGHT_THEME_CODE) {
            setCurrentTheme(HIGH_CONTRAST_WHITE_THEME_CODE);
        } else {
            setCurrentTheme(HIGH_CONTRAST_BLACK_THEME_CODE);
        }
    } else {
        if (currentThemeCode == HIGH_CONTRAST_WHITE_THEME_CODE) {
            setCurrentTheme(LIGHT_THEME_CODE);
        } else {
            setCurrentTheme(DARK_THEME_CODE);
        }
    }
}

void UiConfiguration::setCurrentTheme(const ThemeCode& codeKey)
{
    settings()->setSharedValue(UI_CURRENT_THEME_CODE_KEY, Val(codeKey));
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

Notification UiConfiguration::currentThemeChanged() const
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

Notification UiConfiguration::fontChanged() const
{
    return m_fontChanged;
}

std::string UiConfiguration::iconsFontFamily() const
{
    return settings()->value(UI_ICONS_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::iconsFontSize(IconSizeType type) const
{
    switch (type) {
    case IconSizeType::Regular: return 16;
    case IconSizeType::Toolbar: return 18;
    }

    return 16;
}

Notification UiConfiguration::iconsFontChanged() const
{
    return m_iconsFontChanged;
}

std::string UiConfiguration::musicalFontFamily() const
{
    return settings()->value(UI_MUSICAL_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::musicalFontSize() const
{
    return settings()->value(UI_MUSICAL_FONT_SIZE_KEY).toInt();
}

Notification UiConfiguration::musicalFontChanged() const
{
    return m_musicalFontChanged;
}

std::string UiConfiguration::defaultFontFamily() const
{
    std::string family = qApp->font().family().toStdString();

#ifdef Q_OS_WIN
    static const QString defaultWinFamily = "Segoe UI";
    QFontDatabase fontDatabase;
    if (fontDatabase.hasFamily(defaultWinFamily)) {
        family = defaultWinFamily.toStdString();
    }
#endif

    return family;
}

int UiConfiguration::defaultFontSize() const
{
    return 12;
}

double UiConfiguration::guiScaling() const
{
    const QScreen* screen = mainWindow()->screen();
    return screen ? screen->devicePixelRatio() : 1;
}

void UiConfiguration::setPhysicalDotsPerInch(std::optional<double> dpi)
{
    m_customDPI = dpi;
}

double UiConfiguration::dpi() const
{
    if (m_customDPI) {
        return m_customDPI.value();
    }

    const QScreen* screen = mainWindow()->screen();
    return screen ? screen->logicalDotsPerInch() : 100;
}

mu::ValNt<QByteArray> UiConfiguration::pageState(const QString& pageName) const
{
    ValNt<QByteArray> result;
    result.val = m_uiArrangement.state(pageName);
    result.notification = m_uiArrangement.stateChanged(pageName);

    return result;
}

void UiConfiguration::setPageState(const QString& pageName, const QByteArray& state)
{
    m_uiArrangement.setState(pageName, state);
}

QByteArray UiConfiguration::windowGeometry() const
{
    return m_uiArrangement.state(WINDOW_GEOMETRY_KEY);
}

void UiConfiguration::setWindowGeometry(const QByteArray& geometry)
{
    m_uiArrangement.setState(WINDOW_GEOMETRY_KEY, geometry);
}

Notification UiConfiguration::windowGeometryChanged() const
{
    return m_windowGeometryChanged;
}

void UiConfiguration::applyPlatformStyle(QWindow* window)
{
    platformTheme()->applyPlatformStyleOnWindowForTheme(window, currentThemeCodeKey());
}

bool UiConfiguration::isVisible(const QString& key, bool def) const
{
    QString val = m_uiArrangement.value(key);
    bool ok = false;
    int valInt = val.toInt(&ok);
    return ok ? bool(valInt) : def;
}

void UiConfiguration::setIsVisible(const QString& key, bool val)
{
    m_uiArrangement.setValue(key, QString::number(val ? 1 : 0));
}

mu::async::Notification UiConfiguration::isVisibleChanged(const QString& key) const
{
    return m_uiArrangement.valueChanged(key);
}

ToolConfig UiConfiguration::toolConfig(const QString& toolName) const
{
    return m_uiArrangement.toolConfig(toolName);
}

void UiConfiguration::setToolConfig(const QString& toolName, const ToolConfig& config)
{
    m_uiArrangement.setToolConfig(toolName, config);
}

mu::async::Notification UiConfiguration::toolConfigChanged(const QString& toolName) const
{
    return m_uiArrangement.toolConfigChanged(toolName);
}

int UiConfiguration::flickableMaxVelocity() const
{
    return FLICKABLE_MAX_VELOCITY;
}
