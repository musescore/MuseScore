/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#pragma once

#include <vector>
#include <map>

#include "global/types/uri.h"
#include "global/types/string.h"
#include "global/io/path.h"
#include "global/types/translatablestring.h"
#include "ui/uiaction.h"
#include "ui/view/iconcodes.h"
#include "shortcuts/shortcutcontext.h"

#include "log.h"

namespace muse::extensions {
//! NOTE Api versions:
//! 1 - plugins from 3х
//! 2 - extensions
constexpr int DEFAULT_API_VERSION = 2;

//! NOTE Default extension dialog modality
constexpr bool DEFAULT_MODAL = false;

constexpr const char* SINGLE_FILE_EXT = "mext";

//! NOTE Should a project be open...
//!
//! Must match contexts in `context/uicontext.h`
//! Slightly different values ​​were intentionally made
//! so that it would look neater in the manifest and there would be a conversion function,
//! i.e. we had the ability to change the internal names.
//! And the external names (specified in the manifest) remained unchanged.
//! And there was a list of contexts that we specifically provide for extensions.
//!
//! If contexts appear outside the muse framework,
//! then we need to add an interface for conversion, and require the application to implement it.
//!
constexpr const char16_t* DEFAULT_UI_CONTEXT = u"ProjectOpened";
static inline ui::UiContext toUiContext(const String& ctx)
{
    if (ctx == u"ProjectOpened") {
        return ui::UiCtxProjectOpened;
    } else if (ctx == u"Any") {
        return ui::UiCtxAny;
    }
    LOGE() << "unknown ui context: " << ctx << ", will be use default: " << DEFAULT_UI_CONTEXT;
    return ui::UiCtxProjectOpened;
}

//! NOTE There is currently no separate context shortcut in the manifest properties.
//! But if needed, it needs to be added.
//! Also see the comments above for the UiContext

static inline std::string toScContext(const String& uiCtx)
{
    if (uiCtx == u"ProjectOpened") {
        return shortcuts::CTX_PROJECT_OPENED;
    } else if (uiCtx == u"Any") {
        return shortcuts::CTX_ANY;
    }
    LOGE() << "unknown ui context: " << uiCtx << ", will be use default: " << DEFAULT_UI_CONTEXT;
    return shortcuts::CTX_PROJECT_OPENED;
}

enum class Type {
    Undefined = 0,
    Form,       // Have UI, controls, user interaction
    Macros,     // Without UI, they just do some script
    Composite   // Composite with some UI and script
};

static inline Type typeFromString(const std::string& str)
{
    if (str == "form") {
        return Type::Form;
    } else if (str == "macros") {
        return Type::Macros;
    } else if (str == "composite") {
        return Type::Composite;
    }
    return Type::Undefined;
}

static inline std::string typeToString(const Type& type)
{
    switch (type) {
    case Type::Undefined: return "undefined";
    case Type::Form: return "form";
    case Type::Macros: return "macros";
    case Type::Composite: return "composite";
    }
    return std::string();
}

enum Filter {
    Enabled,
    All
};

//! NOTE Exec points
using ExecPointName = std::string;
struct ExecPoint {
    ExecPointName name;
    TranslatableString title;

    inline bool operator==(const ExecPoint& p) const { return p.name == name; }
    inline bool operator!=(const ExecPoint& p) const { return !this->operator==(p); }

    inline bool isNull() const { return name.empty(); }
};

static inline const ExecPointName EXEC_DISABLED = "disabled";
static inline const ExecPointName EXEC_MANUALLY = "manually";

struct Action {
    std::string code;
    Type type = Type::Undefined;
    bool modal = DEFAULT_MODAL;
    String title;
    ui::IconCode::Code icon = ui::IconCode::Code::NONE;
    String uiCtx = DEFAULT_UI_CONTEXT;
    bool showOnToolbar = false;
    bool showOnAppmenu = true;
    io::path_t path;
    String func = u"main";
    int apiversion = DEFAULT_API_VERSION;
    bool legacyPlugin = false;

    struct Config {
        ExecPointName execPoint = EXEC_DISABLED;
    };

    bool isValid() const { return type != Type::Undefined && !code.empty(); }
};

inline actions::ActionQuery makeActionQueryBase(const Uri& uri)
{
    UriQuery q(uri);
    q.setScheme("action");
    return q;
}

inline actions::ActionQuery makeActionQuery(const Uri& uri, const std::string& actionCode)
{
    UriQuery q = makeActionQueryBase(uri);
    q.addParam("action", Val(actionCode));
    return q;
}

inline UriQuery uriQueryFromActionQuery(const actions::ActionQuery& a)
{
    UriQuery q(a);
    q.setScheme("musescore");
    return q;
}

inline actions::ActionCode makeActionCodeBase(const Uri& uri)
{
    return makeActionQueryBase(uri).toString();
}

inline actions::ActionCode makeActionCode(const Uri& uri, const std::string& extActionCode)
{
    return makeActionQuery(uri, extActionCode).toString();
}

/*
manifest.json
{

"uri": String,                    // Example: muse://module/target/name
"type": String,                   // Values: form, macros
"title": String,                  //
"description": String,            //
"category": String,               //
"thumbnail": String,              //
"version": String,                //
"apiversion": String              // Optional default 2

"main": String                    // Path (name) of main file (qml or js)
}*/

struct Manifest {
    struct Config {
        std::map<std::string /*action*/, Action::Config> actions;

        const Action::Config& aconfig(const std::string& code) const
        {
            auto it = actions.find(code);
            if (it != actions.end()) {
                return it->second;
            }
            static Action::Config _dummy;
            return _dummy;
        }
    };

    Uri uri;
    Type type = Type::Undefined;
    String title;
    String description;
    String category;
    io::path_t thumbnail;
    String version;
    int apiversion = DEFAULT_API_VERSION;
    bool legacyPlugin = false;

    std::vector<Action> actions;

    Config config;

    bool isValid() const { return type != Type::Undefined && uri.isValid(); }

    bool enabled() const
    {
        //! NOTE If at least one is enabled, then it's enabled.
        for (const Action& a : actions) {
            auto it = config.actions.find(a.code);
            if (it == config.actions.end()) {
                continue;
            }
            const Action::Config& ac = it->second;
            if (!ac.execPoint.empty() && ac.execPoint != EXEC_DISABLED) {
                return true;
            }
        }
        return false;
    }

    Action action(const std::string& code) const
    {
        for (const Action& a : actions) {
            if (a.code == code) {
                return a;
            }
        }
        return Action();
    }
};

using ManifestList = std::vector<Manifest>;

using KnownCategories = std::map<std::string /*name*/, TranslatableString /*title*/>;
}
