#include "uiconfiguration.h"

#include "settings.h"
#include "mscore/globals.h"

namespace mu {
namespace framework {
using ThemeType = mu::framework::IUiConfiguration::ThemeType;

static const std::string module_name("ui");
static const Settings::Key THEME_TYPE_KEY(module_name, "ui/application/globalStyle");
static const Settings::Key FONT_FAMILY_KEY(module_name, "ui/theme/fontFamily");
static const Settings::Key FONT_SIZE_KEY(module_name, "ui/theme/fontSize");

UiConfiguration::UiConfiguration()
{
    settings()->addItem(THEME_TYPE_KEY, Val(static_cast<int>(ThemeType::LIGHT_THEME)));
    settings()->addItem(FONT_FAMILY_KEY, Val("FreeSans"));
    settings()->addItem(FONT_SIZE_KEY, Val(12));

    settings()->valueChanged(THEME_TYPE_KEY).onReceive(nullptr, [this](const Val& val) {
                m_currentThemeTypeChannel.send(static_cast<ThemeType>(val.toInt()));
            });

    settings()->valueChanged(FONT_FAMILY_KEY).onReceive(nullptr, [this](const Val& val) {
                m_currentFontFamilyChannel.send(QString::fromStdString(val.toString()));
            });

    settings()->valueChanged(FONT_SIZE_KEY).onReceive(nullptr, [this](const Val& val) {
                m_currentFontSizeChannel.send(val.toInt());
            });
}

ThemeType UiConfiguration::themeType() const
{
    return static_cast<ThemeType>(settings()->value(THEME_TYPE_KEY).toInt());
}

async::Channel<ThemeType> UiConfiguration::themeTypeChanged()
{
    return m_currentThemeTypeChannel;
}

QString UiConfiguration::fontFamily() const
{
    return QString::fromStdString(settings()->value(FONT_FAMILY_KEY).toString());
}

async::Channel<QString> UiConfiguration::fontFamilyChanged()
{
    return m_currentFontFamilyChannel;
}

int UiConfiguration::fontSize() const
{
    return settings()->value(FONT_SIZE_KEY).toInt();
}

async::Channel<int> UiConfiguration::fontSizeChanged()
{
    return m_currentFontSizeChannel;
}

float UiConfiguration::guiScaling() const
{
    return static_cast<float>(Ms::guiScaling);
}
}
}
