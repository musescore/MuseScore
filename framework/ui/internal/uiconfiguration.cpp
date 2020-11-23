#include "uiconfiguration.h"

#include "settings.h"
#include "mscore/globals.h"

#include <QMainWindow>
#include <QScreen>

using namespace mu::framework;
using namespace mu::async;

using ThemeType = IUiConfiguration::ThemeType;

static const std::string module_name("ui");
static const Settings::Key THEME_TYPE_KEY(module_name, "ui/application/globalStyle");
static const Settings::Key FONT_FAMILY_KEY(module_name, "ui/theme/fontFamily");
static const Settings::Key FONT_SIZE_KEY(module_name, "ui/theme/fontSize");
static const Settings::Key MUSICAL_FONT_FAMILY_KEY(module_name, "ui/theme/musicalFontFamily");
static const Settings::Key MUSICAL_FONT_SIZE_KEY(module_name, "ui/theme/musicalFontSize");

void UiConfiguration::init()
{
    settings()->setDefaultValue(THEME_TYPE_KEY, Val(static_cast<int>(ThemeType::LIGHT_THEME)));
    settings()->setDefaultValue(FONT_FAMILY_KEY, Val("FreeSans"));
    settings()->setDefaultValue(FONT_SIZE_KEY, Val(12));
    settings()->setDefaultValue(MUSICAL_FONT_FAMILY_KEY, Val("Leland"));
    settings()->setDefaultValue(MUSICAL_FONT_SIZE_KEY, Val(12));

    settings()->valueChanged(THEME_TYPE_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentThemeTypeChannel.send(static_cast<ThemeType>(val.toInt()));
    });

    settings()->valueChanged(FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentFontFamilyChannel.send(QString::fromStdString(val.toString()));
    });

    settings()->valueChanged(FONT_SIZE_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentFontSizeChannel.send(val.toInt());
    });

    settings()->valueChanged(MUSICAL_FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentMusicalFontFamilyChannel.send(QString::fromStdString(val.toString()));
    });

    settings()->valueChanged(MUSICAL_FONT_SIZE_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentMusicalFontSizeChannel.send(val.toInt());
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

QString UiConfiguration::fontFamily() const
{
    return QString::fromStdString(settings()->value(FONT_FAMILY_KEY).toString());
}

Channel<QString> UiConfiguration::fontFamilyChanged() const
{
    return m_currentFontFamilyChannel;
}

int UiConfiguration::fontSize() const
{
    return settings()->value(FONT_SIZE_KEY).toInt();
}

Channel<int> UiConfiguration::fontSizeChanged() const
{
    return m_currentFontSizeChannel;
}

QString UiConfiguration::musicalFontFamily() const
{
    return QString::fromStdString(settings()->value(MUSICAL_FONT_FAMILY_KEY).toString());
}

Channel<QString> UiConfiguration::musicalFontFamilyChanged() const
{
    return m_currentMusicalFontFamilyChannel;
}

int UiConfiguration::musicalFontSize() const
{
    return settings()->value(MUSICAL_FONT_SIZE_KEY).toInt();
}

Channel<int> UiConfiguration::musicalFontSizeChanged() const
{
    return m_currentMusicalFontSizeChannel;
}

float UiConfiguration::guiScaling() const
{
    return mainWindow()->qMainWindow()->screen()->devicePixelRatio();
}

float UiConfiguration::physicalDotsPerInch() const
{
    return mainWindow()->qMainWindow()->screen()->physicalDotsPerInch();
}
