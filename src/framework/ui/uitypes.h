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
#ifndef MUSE_UI_UITYPES_H
#define MUSE_UI_UITYPES_H

#include <vector>
#include <QString>
#include <QMetaType>
#include <QMap>
#include <QQuickItem>

#include "view/iconcodes.h" // IWYU pragma: export
#include "workspace/workspacetypes.h"

namespace muse::ui {
//! NOTE Same as QSGRendererInterface::Api
enum class GraphicsApi {
    Default,
    Software,
    OpenVG,
    OpenGL,
    Direct3D11,
    Vulkan,
    Metal,
    Null
};

using ThemeCode = std::string;

inline ThemeCode themeCodeFromString(const QString& str)
{
    return str.toStdString();
}

static const ThemeCode LIGHT_THEME_CODE("light");
static const ThemeCode DARK_THEME_CODE("dark");
static const ThemeCode HIGH_CONTRAST_WHITE_THEME_CODE("high_contrast_white");
static const ThemeCode HIGH_CONTRAST_BLACK_THEME_CODE("high_contrast_black");

inline std::vector<ThemeCode> allStandardThemeCodes()
{
    return {
        LIGHT_THEME_CODE,
        DARK_THEME_CODE,
        HIGH_CONTRAST_WHITE_THEME_CODE,
        HIGH_CONTRAST_BLACK_THEME_CODE
    };
}

inline bool isDarkTheme(const ThemeCode& themeCode)
{
    return themeCode == DARK_THEME_CODE
           || themeCode == HIGH_CONTRAST_BLACK_THEME_CODE;
}

inline bool isHighContrastTheme(const ThemeCode& themeCode)
{
    return themeCode == HIGH_CONTRAST_WHITE_THEME_CODE
           || themeCode == HIGH_CONTRAST_BLACK_THEME_CODE;
}

enum ThemeStyleKey
{
    UNKNOWN = -1,

    BACKGROUND_PRIMARY_COLOR = 0,
    BACKGROUND_SECONDARY_COLOR,
    BACKGROUND_TERTIARY_COLOR,
    BACKGROUND_QUARTERNARY_COLOR,
    POPUP_BACKGROUND_COLOR,
    PROJECT_TAB_COLOR,
    TEXT_FIELD_COLOR,
    ACCENT_COLOR,
    STROKE_COLOR,
    STROKE_SECONDARY_COLOR,
    BUTTON_COLOR,
    FONT_PRIMARY_COLOR,
    FONT_SECONDARY_COLOR,
    LINK_COLOR,
    FOCUS_COLOR,
    WHITE_COLOR,
    BLACK_COLOR,
    PLAY_COLOR,
    RECORD_COLOR,

    BORDER_WIDTH,
    NAVIGATION_CONTROL_BORDER_WIDTH,

    ACCENT_OPACITY_NORMAL,
    ACCENT_OPACITY_HOVER,
    ACCENT_OPACITY_HIT,

    BUTTON_OPACITY_NORMAL,
    BUTTON_OPACITY_HOVER,
    BUTTON_OPACITY_HIT,

    ITEM_OPACITY_DISABLED
};

struct ThemeInfo
{
    ThemeCode codeKey;
    std::string title;
    QMap<ThemeStyleKey, QVariant> values;
};

using ThemeList = std::vector<ThemeInfo>;

enum class FontSizeType {
    BODY,
    BODY_LARGE,
    TAB,
    HEADER,
    TITLE
};

enum class IconSizeType {
    Regular,
    Toolbar
};

class ContainerType
{
    Q_GADGET
public:
    enum Type
    {
        Undefined = 0,
        PrimaryPage,
        QmlDialog,
        QWidgetDialog
    };
    Q_ENUM(Type)
};

struct ContainerMeta
{
    ContainerType::Type type = ContainerType::Undefined;
    QString qmlPath;
    int widgetMetaTypeId = QMetaType::UnknownType;

    ContainerMeta() = default;

    ContainerMeta(const ContainerType::Type& type)
        : type(type) {}
    ContainerMeta(const ContainerType::Type& type, const QString& qmlPath)
        : type(type), qmlPath(qmlPath) {}
    ContainerMeta(const ContainerType::Type& type, int widgetMetaTypeId)
        : type(type), widgetMetaTypeId(widgetMetaTypeId) {}
};

// workspaces
inline const workspace::DataKey WS_UiSettings("ui_settings");
inline const workspace::DataKey WS_UiStates("ui_states");
inline const workspace::DataKey WS_UiToolConfigs("ui_toolconfigs");
}

#endif // MUSE_UI_UITYPES_H
