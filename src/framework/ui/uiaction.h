#ifndef MUSE_UI_UIACTION_H
#define MUSE_UI_UIACTION_H

#include <optional>

#include "actions/actiontypes.h"
#include "global/types/mnemonicstring.h"

#include "view/iconcodes.h"

namespace muse::ui {
struct UiContext
{
    UiContext() = default;
    constexpr UiContext(const char* ctx)
        : const_data(ctx) {}

    inline bool operator ==(const UiContext& ctx) const
    {
        return std::strcmp(const_data, ctx.const_data) == 0;
    }

    inline bool operator !=(const UiContext& ctx) const
    {
        return !this->operator ==(ctx);
    }

    std::string toString() const { return const_data ? std::string(const_data) : std::string(); }

private:
    const char* const_data = nullptr;
};

//! NOTE Only general UI contexts are declared here, which do not depend on the specifics of the application.
//! Application-specific UI contexts are declared in the `context/uicontext.h` file
static constexpr UiContext UiCtxUnknown = "UiCtxUnknown";
static constexpr UiContext UiCtxAny = "UiCtxAny";

// pages
static constexpr ui::UiContext UiCtxHomeOpened = "UiCtxHomeOpened";
static constexpr ui::UiContext UiCtxProjectOpened = "UiCtxProjectOpened";
static constexpr ui::UiContext UiCtxProjectFocused = "UiCtxProjectFocused";

enum class Checkable {
    No = 0,
    Yes
};

struct UiAction
{
    actions::ActionCode code;
    UiContext uiCtx = UiCtxAny;
    std::string scCtx = "any";
    MnemonicString title;
    TranslatableString description;
    IconCode::Code iconCode = IconCode::Code::NONE;
    QString iconColor;
    Checkable checkable = Checkable::No;
    std::vector<std::string> shortcuts;

    UiAction() = default;
    UiAction(const actions::ActionCode& code, UiContext ctx, std::string scCtx, Checkable ch = Checkable::No)
        : code(code), uiCtx(ctx), scCtx(scCtx), checkable(ch) {}

    UiAction(const actions::ActionCode& code, UiContext ctx, std::string scCtx, const MnemonicString& title,
             Checkable ch = Checkable::No)
        : code(code), uiCtx(ctx), scCtx(scCtx), title(title), checkable(ch) {}

    UiAction(const actions::ActionCode& code, UiContext ctx, std::string scCtx, const MnemonicString& title,
             const TranslatableString& desc, Checkable ch = Checkable::No)
        : code(code), uiCtx(ctx), scCtx(scCtx), title(title), description(desc),  checkable(ch) {}

    UiAction(const actions::ActionCode& code, UiContext ctx, std::string scCtx, const MnemonicString& title,
             const TranslatableString& desc, IconCode::Code icon, Checkable ch = Checkable::No)
        : code(code), uiCtx(ctx), scCtx(scCtx), title(title), description(desc), iconCode(icon), checkable(ch) {}

    UiAction(const actions::ActionCode& code, UiContext ctx, std::string scCtx, const MnemonicString& title,
             const TranslatableString& desc, IconCode::Code icon, QString iconColor, Checkable ch = Checkable::No)
        : code(code), uiCtx(ctx), scCtx(scCtx), title(title), description(desc), iconCode(icon), iconColor(iconColor), checkable(ch) {}

    UiAction(const actions::ActionCode& code, UiContext ctx, std::string scCtx, const MnemonicString& title, IconCode::Code icon,
             Checkable ch = Checkable::No)
        : code(code), uiCtx(ctx), scCtx(scCtx), title(title), iconCode(icon), checkable(ch) {}

    bool isValid() const
    {
        return !code.empty();
    }

    bool operator==(const UiAction& other) const
    {
        return code == other.code
               && uiCtx == other.uiCtx
               && scCtx == other.scCtx
               && title == other.title
               && description == other.description
               && iconCode == other.iconCode
               && iconColor == other.iconColor
               && checkable == other.checkable
               && shortcuts == other.shortcuts;
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
        return UiActionState { false, checked };
    }

    static UiActionState make_enabled(bool checked = false)
    {
        return UiActionState { true, checked };
    }

    QVariantMap toMap() const
    {
        return {
            { "enabled", enabled },
            { "checked", checked }
        };
    }
};

struct ToolConfig
{
    struct Item
    {
        actions::ActionCode action;
        bool show = true;

        Item() = default;
        Item(const actions::ActionCode& a, bool sh)
            : action(a), show(sh) {}

        bool isSeparator() const
        {
            return action.empty();
        }

        bool operator ==(const Item& other) const
        {
            return action == other.action
                   && show == other.show;
        }
    };

    QList<Item> items;

    bool isValid() const { return !items.isEmpty(); }
};
}

#endif // MUSE_UI_UIACTION_H
