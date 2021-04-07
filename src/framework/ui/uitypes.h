//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_UI_UITYPES_H
#define MU_UI_UITYPES_H

#include <vector>
#include <optional>
#include <cstring>
#include <QString>
#include <QMetaType>

#include "ret.h"
#include "val.h"
#include "actions/actiontypes.h"
#include "view/iconcodes.h"

namespace mu::ui {
using ThemeCode = std::string;

static ThemeCode DARK_THEME_CODE("dark");
static ThemeCode LIGHT_THEME_CODE("light");
static ThemeCode HIGH_CONTRAST_THEME_CODE("high_contrast");

inline std::vector<ThemeCode> allStandardThemeCodes()
{
    return {
        LIGHT_THEME_CODE,
        DARK_THEME_CODE,
        HIGH_CONTRAST_THEME_CODE
    };
}

enum ThemeStyleKey
{
    UNKNOWN = -1,

    BACKGROUND_PRIMARY_COLOR = 0,
    BACKGROUND_SECONDARY_COLOR,
    POPUP_BACKGROUND_COLOR,
    TEXT_FIELD_COLOR,
    ACCENT_COLOR,
    STROKE_COLOR,
    BUTTON_COLOR,
    FONT_PRIMARY_COLOR,
    FONT_SECONDARY_COLOR,
    LINK_COLOR,
    FOCUS_COLOR,

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

// UiActions/Menu

struct UiContext
{
    UiContext() = default;
    constexpr UiContext(const char* ctx)
        : const_data(ctx) {}

    inline bool operator ==(const UiContext& ctx) const
    {
        return std::strcmp(const_data, ctx.const_data) == 0;
    }

private:
    const char* const_data = nullptr;
};

//! NOTE Only general UI contexts are declared here, which do not depend on the specifics of the application.
//! Application-specific UI contexts are declared in the `context/uicontext.h` file
static constexpr UiContext UiCtxUnknown = "UiCtxUnknown";
static constexpr UiContext UiCtxAny = "UiCtxAny";

enum class Checkable {
    No = 0,
    Yes
};

struct UiAction
{
    actions::ActionCode code;
    UiContext context = UiCtxAny;
    QString title;
    QString description;
    IconCode::Code iconCode = IconCode::Code::NONE;
    Checkable checkable = Checkable::No;
    std::string shortcut;

    UiAction() = default;
    UiAction(const actions::ActionCode& code, UiContext ctx, Checkable ch = Checkable::No)
        : code(code), context(ctx), checkable(ch) {}
    UiAction(const actions::ActionCode& code, UiContext ctx, const char* title, Checkable ch = Checkable::No)
        : code(code), context(ctx), title(title), checkable(ch) {}
    UiAction(const actions::ActionCode& code, UiContext ctx, const char* title, const char* desc, Checkable ch = Checkable::No)
        : code(code), context(ctx), title(title), description(desc), checkable(ch) {}
    UiAction(const actions::ActionCode& code, UiContext ctx, const char* title, const char* desc, IconCode::Code icon,
             Checkable ch = Checkable::No)
        : code(code), context(ctx), title(title), description(desc), iconCode(icon), checkable(ch) {}
    UiAction(const actions::ActionCode& code, UiContext ctx, const char* title, IconCode::Code icon, Checkable ch = Checkable::No)
        : code(code), context(ctx), title(title), iconCode(icon), checkable(ch) {}

    bool isValid() const
    {
        return !code.empty();
    }
};

class UiActionList : public std::vector<UiAction>
{
public:
    UiActionList() = default;
    UiActionList(std::initializer_list<UiAction> l)
        : std::vector<UiAction>(l) {}
    UiActionList(std::vector<UiAction>::iterator b, std::vector<UiAction>::iterator e)
        : std::vector<UiAction>(b, e) {}

    bool contains(const actions::ActionCode& code) const
    {
        auto it = std::find_if(cbegin(), cend(), [code](const UiAction& a) {
            return a.code == code;
        });
        return it != cend();
    }

    std::optional<size_t> indexOf(const actions::ActionCode& code) const
    {
        for (size_t i = 0; i < size(); ++i) {
            if (at(i).code == code) {
                return i;
            }
        }

        return std::nullopt;
    }
};

struct UiActionState
{
    bool enabled = false;
    bool checked = false;

    inline bool operator ==(const UiActionState& st) const
    {
        return st.enabled == enabled && st.checked == checked;
    }

    inline bool operator !=(const UiActionState& st) const
    {
        return !this->operator ==(st);
    }

    static UiActionState make_disabled(bool checked = false)
    {
        return UiActionState{ false, checked };
    }

    static UiActionState make_enabled(bool checked = false)
    {
        return UiActionState{ true, checked };
    }
};

struct MenuItem : public UiAction
{
    QString section;
    UiActionState state;
    bool selectable = false;
    bool selected = false;
    actions::ActionData args;
    QList<MenuItem> subitems;

    MenuItem() = default;
    MenuItem(const UiAction& a)
        : UiAction(a) {}

    QVariantMap toMap() const
    {
        QVariantList subitemsVariantList;
        for (const MenuItem& item: subitems) {
            subitemsVariantList << item.toMap();
        }

        return {
            { "code", QString::fromStdString(code) },
            { "shortcut", QString::fromStdString(shortcut) },
            { "title", title },
            { "description", description },
            { "section", section },
            { "icon", static_cast<int>(iconCode) },
            { "enabled", state.enabled },
            { "checkable", checkable == Checkable::Yes },
            { "checked", state.checked },
            { "selectable", selectable },
            { "selected", selected },
            { "subitems", subitemsVariantList }
        };
    }
};
using MenuItemList = QList<MenuItem>;
}

#endif // MU_UI_UIERRORS_H
