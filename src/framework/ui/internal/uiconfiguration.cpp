#include "uiconfiguration.h"

#include "settings.h"

#include <QMainWindow>
#include <QScreen>

using namespace mu::framework;
using namespace mu::async;

using ThemeType = IUiConfiguration::ThemeType;

static const std::string module_name("ui");
static const Settings::Key THEME_TYPE_KEY(module_name, "ui/application/globalStyle");
static const Settings::Key FONT_FAMILY_KEY(module_name, "ui/theme/fontFamily");
static const Settings::Key SEMIBOLD_FONT_FAMILY_KEY(module_name, "ui/theme/semiBoldFontFamily");
static const Settings::Key FONT_SIZE_KEY(module_name, "ui/theme/fontSize");
static const Settings::Key ICONS_FONT_FAMILY_KEY(module_name, "ui/theme/iconsFontFamily");
static const Settings::Key MUSICAL_FONT_FAMILY_KEY(module_name, "ui/theme/musicalFontFamily");
static const Settings::Key MUSICAL_FONT_SIZE_KEY(module_name, "ui/theme/musicalFontSize");

void UiConfiguration::init()
{
    settings()->setDefaultValue(THEME_TYPE_KEY, Val(static_cast<int>(ThemeType::LIGHT_THEME)));
    settings()->setDefaultValue(FONT_FAMILY_KEY, Val("Fira Sans"));
    settings()->setDefaultValue(SEMIBOLD_FONT_FAMILY_KEY, Val("Fira Sans SemiBold"));
    settings()->setDefaultValue(FONT_SIZE_KEY, Val(12));
    settings()->setDefaultValue(ICONS_FONT_FAMILY_KEY, Val("MusescoreIcon"));
    settings()->setDefaultValue(MUSICAL_FONT_FAMILY_KEY, Val("Leland"));
    settings()->setDefaultValue(MUSICAL_FONT_SIZE_KEY, Val(12));

    settings()->valueChanged(THEME_TYPE_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentThemeTypeChannel.send(static_cast<ThemeType>(val.toInt()));
    });

    settings()->valueChanged(FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val&) {
        m_fontChanged.notify();
    });

    settings()->valueChanged(FONT_SIZE_KEY).onReceive(nullptr, [this](const Val&) {
        m_fontChanged.notify();
        m_iconsFontChanged.notify();
    });

    settings()->valueChanged(ICONS_FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val&) {
        m_iconsFontChanged.notify();
    });

    settings()->valueChanged(MUSICAL_FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val&) {
        m_musicalFontChanged.notify();
    });

    settings()->valueChanged(MUSICAL_FONT_SIZE_KEY).onReceive(nullptr, [this](const Val&) {
        m_musicalFontChanged.notify();
    });
}

ThemeType UiConfiguration::themeType() const
{
    return static_cast<ThemeType>(settings()->value(THEME_TYPE_KEY).toInt());
}

Channel<ThemeType> UiConfiguration::themeTypeChanged() const
{
    return m_currentThemeTypeChannel;
}

std::string UiConfiguration::fontFamily() const
{
    return settings()->value(FONT_FAMILY_KEY).toString();
}

std::string UiConfiguration::semiBoldFontFamily() const
{
    return settings()->value(SEMIBOLD_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::fontSize(FontSizeType type) const
{
    int bodyFontSize = settings()->value(FONT_SIZE_KEY).toInt();

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
    return settings()->value(ICONS_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::iconsFontSize() const
{
    return fontSize(FontSizeType::TAB);
}

Notification UiConfiguration::iconsFontChanged() const
{
    return m_iconsFontChanged;
}

std::string UiConfiguration::musicalFontFamily() const
{
    return settings()->value(MUSICAL_FONT_FAMILY_KEY).toString();
}

int UiConfiguration::musicalFontSize() const
{
    return settings()->value(MUSICAL_FONT_SIZE_KEY).toInt();
}

Notification UiConfiguration::musicalFontChanged() const
{
    return m_musicalFontChanged;
}

float UiConfiguration::guiScaling() const
{
    return mainWindow()->qMainWindow()->screen()->devicePixelRatio();
}

float UiConfiguration::physicalDotsPerInch() const
{
    return mainWindow()->qMainWindow()->screen()->physicalDotsPerInch();
}
