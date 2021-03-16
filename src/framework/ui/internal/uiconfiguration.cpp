#include "uiconfiguration.h"

#include "settings.h"
#include "log.h"
#include "translation.h"

#include <QMainWindow>
#include <QScreen>

using namespace mu::ui;
using namespace mu::framework;
using namespace mu::async;

static const Settings::Key UI_THEME_TYPE_KEY("ui", "ui/application/globalStyle");
static const Settings::Key UI_FONT_FAMILY_KEY("ui", "ui/theme/fontFamily");
static const Settings::Key UI_FONT_SIZE_KEY("ui", "ui/theme/fontSize");
static const Settings::Key UI_ICONS_FONT_FAMILY_KEY("ui", "ui/theme/iconsFontFamily");
static const Settings::Key UI_MUSICAL_FONT_FAMILY_KEY("ui", "ui/theme/musicalFontFamily");
static const Settings::Key UI_MUSICAL_FONT_SIZE_KEY("ui", "ui/theme/musicalFontSize");

static const std::string STATES_PATH("ui/states");

static const std::map<ThemeStyleKey, QVariant> LIGHT_THEME {
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

    { ACCENT_OPACITY_NORMAL, 0.3 },
    { ACCENT_OPACITY_HOVER, 0.15 },
    { ACCENT_OPACITY_HIT, 0.5 },

    { BUTTON_OPACITY_NORMAL, 0.7 },
    { BUTTON_OPACITY_HOVER, 0.5 },
    { BUTTON_OPACITY_HIT, 1.0 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

static const std::map<ThemeStyleKey, QVariant> DARK_THEME {
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

    { ACCENT_OPACITY_NORMAL, 0.8 },
    { ACCENT_OPACITY_HOVER, 1.0 },
    { ACCENT_OPACITY_HIT, 0.5 },

    { BUTTON_OPACITY_NORMAL, 0.8 },
    { BUTTON_OPACITY_HOVER, 1.0 },
    { BUTTON_OPACITY_HIT, 0.5 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

static const std::map<ThemeStyleKey, QVariant> HIGH_CONTRAST_THEME {
    { BACKGROUND_PRIMARY_COLOR, "#000000" },
    { BACKGROUND_SECONDARY_COLOR, "#000000" },
    { POPUP_BACKGROUND_COLOR, "#FFFFFF" },
    { TEXT_FIELD_COLOR, "#FFFFFF" },
    { ACCENT_COLOR, "#19EBFF" },
    { STROKE_COLOR, "#FFFFFF" },
    { BUTTON_COLOR, "#000000" },
    { FONT_PRIMARY_COLOR, "#FFFFFF" },
    { FONT_SECONDARY_COLOR, "#FFFFFF" },
    { LINK_COLOR, "#70AFEA" },

    { ACCENT_OPACITY_NORMAL, 0.3 },
    { ACCENT_OPACITY_HOVER, 0.15 },
    { ACCENT_OPACITY_HIT, 0.5 },

    { BUTTON_OPACITY_NORMAL, 0.7 },
    { BUTTON_OPACITY_HOVER, 0.5 },
    { BUTTON_OPACITY_HIT, 1.0 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

void UiConfiguration::init()
{
    settings()->setDefaultValue(UI_THEME_TYPE_KEY, Val(static_cast<int>(ThemeType::LIGHT_THEME)));
    settings()->setDefaultValue(UI_FONT_FAMILY_KEY, Val("Fira Sans"));
    settings()->setDefaultValue(UI_FONT_SIZE_KEY, Val(12));
    settings()->setDefaultValue(UI_ICONS_FONT_FAMILY_KEY, Val("MusescoreIcon"));
    settings()->setDefaultValue(UI_MUSICAL_FONT_FAMILY_KEY, Val("Leland"));
    settings()->setDefaultValue(UI_MUSICAL_FONT_SIZE_KEY, Val(12));

    settings()->valueChanged(UI_THEME_TYPE_KEY).onReceive(nullptr, [this](const Val&) {
        m_currentThemeChanged.notify();
    });

    settings()->valueChanged(UI_THEME_TYPE_KEY).onReceive(nullptr, [this](const Val&) {
        m_currentThemeChanged.notify();
    });

    platformTheme()->darkModeSwitched().onReceive(nullptr, [this](bool) {
        m_currentThemeChanged.notify();
    });

    settings()->valueChanged(UI_FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val&) {
        m_fontChanged.notify();
    });

    settings()->valueChanged(UI_FONT_SIZE_KEY).onReceive(nullptr, [this](const Val&) {
        m_fontChanged.notify();
        m_iconsFontChanged.notify();
    });

    settings()->valueChanged(UI_ICONS_FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val&) {
        m_iconsFontChanged.notify();
    });

    settings()->valueChanged(UI_MUSICAL_FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val&) {
        m_musicalFontChanged.notify();
    });

    settings()->valueChanged(UI_MUSICAL_FONT_SIZE_KEY).onReceive(nullptr, [this](const Val&) {
        m_musicalFontChanged.notify();
    });

    workspaceSettings()->valuesChanged().onNotify(nullptr, [this]() {
        m_pageStateChanged.notify();
    });
}

ThemeList UiConfiguration::themes() const
{
    static const std::vector<ThemeType> allTypes {
        ThemeType::LIGHT_THEME,
        ThemeType::DARK_THEME,
        ThemeType::HIGH_CONTRAST
    };

    ThemeList result;
    for (ThemeType type : allTypes) {
        result.push_back(makeTheme(type));
    }

    return result;
}

ThemeInfo UiConfiguration::makeTheme(ThemeType type) const
{
    ThemeInfo theme;
    theme.type = type;

    switch (type) {
    case ThemeType::LIGHT_THEME:
        theme.title = trc("ui", "Light");
        theme.values = LIGHT_THEME;
        break;
    case ThemeType::DARK_THEME:
        theme.title = trc("ui", "Dark");
        theme.values = DARK_THEME;
        break;
    case ThemeType::HIGH_CONTRAST:
        theme.title = trc("ui", "High contrast");
        theme.values = HIGH_CONTRAST_THEME;
        break;
    }

    return theme;
}

ThemeInfo UiConfiguration::currentTheme() const
{
    ThemeType currentType = currentThemeType();

    for (const ThemeInfo& theme: themes()) {
        if (theme.type == currentType) {
            return theme;
        }
    }

    return ThemeInfo();
}

ThemeType UiConfiguration::currentThemeType() const
{
    Val preferredThemeType = settings()->value(UI_THEME_TYPE_KEY);
    bool followSystemTheme = preferredThemeType.isNull();

    if (followSystemTheme) {
        return platformTheme()->isDarkMode() ? ThemeType::DARK_THEME : ThemeType::LIGHT_THEME;
    }

    return static_cast<ThemeType>(preferredThemeType.toInt());
}

void UiConfiguration::setCurrentThemeType(ThemeType type)
{
    settings()->setValue(UI_THEME_TYPE_KEY, Val(static_cast<int>(type)));
}

Notification UiConfiguration::currentThemeChanged() const
{
    return m_currentThemeChanged;
}

std::string UiConfiguration::fontFamily() const
{
    return settings()->value(UI_FONT_FAMILY_KEY).toString();
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
    case IconSizeType::Toolbar: return 20;
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

float UiConfiguration::guiScaling() const
{
    if (mainWindow()) {
        return mainWindow()->qMainWindow()->screen()->devicePixelRatio();
    }
    return 1;
}

void UiConfiguration::setPhysicalDotsPerInch(std::optional<float> dpi)
{
    m_customDPI = dpi;
}

float UiConfiguration::physicalDotsPerInch() const
{
    if (m_customDPI) {
        return m_customDPI.value();
    }

    if (mainWindow()) {
        return mainWindow()->qMainWindow()->screen()->physicalDotsPerInch();
    }
    return 100;
}

QByteArray UiConfiguration::pageState(const std::string& pageName) const
{
    TRACEFUNC;
    std::string key = STATES_PATH + "/" + pageName;
    std::string stateString;

    if (workspaceSettings()->isManage(workspace::WorkspaceTag::UiArrangement)) {
        IWorkspaceSettings::Key workspaceKey{ workspace::WorkspaceTag::UiArrangement, key };
        stateString = workspaceSettings()->value(workspaceKey).toString();
    } else {
        Settings::Key settingsKey{ "global", key };
        stateString = settings()->value(settingsKey).toString();
    }

    return stringToByteArray(stateString);
}

void UiConfiguration::setPageState(const std::string& pageName, const QByteArray& state)
{
    TRACEFUNC;
    std::string key = STATES_PATH + "/" + pageName;
    Val value = Val(byteArrayToString(state));

    if (workspaceSettings()->isManage(workspace::WorkspaceTag::UiArrangement)) {
        IWorkspaceSettings::Key workspaceKey{ workspace::WorkspaceTag::UiArrangement, key };
        workspaceSettings()->setValue(workspaceKey, value);
        return;
    }

    Settings::Key settingsKey{ "global", key };
    settings()->setValue(settingsKey, value);
}

Notification UiConfiguration::pageStateChanged() const
{
    return m_pageStateChanged;
}

QByteArray UiConfiguration::stringToByteArray(const std::string& string) const
{
    QString qString = QString::fromStdString(string);
    QByteArray byteArray64(qString.toUtf8());
    QByteArray byteArray = QByteArray::fromBase64(byteArray64);

    return byteArray;
}

std::string UiConfiguration::byteArrayToString(const QByteArray& byteArray) const
{
    QByteArray byteArray64 = byteArray.toBase64();
    return byteArray64.toStdString();
}
