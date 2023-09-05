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
#include "pluginsactioncontroller.h"

#include "containers.h"
#include "translation.h"
#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;
using namespace mu::actions;

void PluginsActionController::init()
{
    registerPlugins();

    service()->pluginsChanged().onNotify(this, [this](){
        registerPlugins();
    });
}

void PluginsActionController::registerPlugins()
{
    dispatcher()->unReg(this);

    for (const PluginInfo& plugin : values(service()->plugins().val)) {
        dispatcher()->reg(this, plugin.codeKey.toStdString(), [this, codeKey = plugin.codeKey]() {
            onPluginTriggered(codeKey);
        });
    }

    dispatcher()->reg(this, "manage-plugins", [this]() {
        interactive()->open("musescore://home?section=plugins");
    });
}

void PluginsActionController::onPluginTriggered(const CodeKey& codeKey)
{
    auto plugins = service()->plugins().val;
    bool enabled = false;
    bool found = false;
    QString pluginName;

    for (const PluginInfo& plugin : values(plugins)) {
        if (plugin.codeKey == codeKey) {
            enabled = plugin.enabled;
            found = true;
            pluginName = plugin.name;
        }
    }

    if (!found) {
        return;
    }

    if (enabled) {
        service()->run(codeKey);
        return;
    }

    IInteractive::Result result = interactive()->warning(
        qtrc("plugins", "The plugin “%1” is currently disabled. Do you want to enable it now?").arg(pluginName).toStdString(),
        trc("plugins", "Alternatively, you can enable it at any time from Home > Plugins."),
        { interactive()->buttonData(IInteractive::Button::No),
          interactive()->buttonData(IInteractive::Button::Yes) }, 0);

    if (result.standardButton() == IInteractive::Button::Yes) {
        service()->setEnable(codeKey, true);
        service()->run(codeKey);
    }
}
