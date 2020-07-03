#include "uiconfiguration.h"

#include "settings.h"

namespace mu {
namespace framework {

using ThemeType = mu::framework::IUiConfiguration::ThemeType;

static const std::string module_name("ui");
static const Settings::Key THEME_TYPE_KEY(module_name, "ui/application/globalStyle");

UiConfiguration::UiConfiguration()
{
    settings()->addItem(THEME_TYPE_KEY, Val(static_cast<int>(ThemeType::LIGHT_THEME)));

    settings()->valueChanged(THEME_TYPE_KEY).onReceive(nullptr, [this] (const Val& val) {
        m_currentThemeTypeChannel.send(static_cast<ThemeType>(val.toInt()));
    });
}

ThemeType UiConfiguration::currentThemeType() const
{
    return static_cast<ThemeType>(settings()->value(THEME_TYPE_KEY).toInt());
}

async::Channel<ThemeType> UiConfiguration::currentThemeTypeChanged()
{
    return m_currentThemeTypeChannel;
}

}
}
