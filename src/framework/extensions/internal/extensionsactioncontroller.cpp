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
#include "extensionsactioncontroller.h"

#include "translation.h"

#include "extensionsuiactions.h"

#include "log.h"

using namespace muse::extensions;

void ExtensionsActionController::init()
{
    m_uiActions = std::make_shared<ExtensionsUiActions>();
    registerPlugins();

    provider()->manifestListChanged().onNotify(this, [this](){
        registerPlugins();
    });
}

void ExtensionsActionController::registerPlugins()
{
    dispatcher()->unReg(this);

    for (const Manifest& m : provider()->manifestList()) {
        for (const Action& a : m.actions) {
            UriQuery q = makeUriQuery(m.uri, a.code);
            dispatcher()->reg(this, q.toString(), [this, q]() {
                onPluginTriggered(q);
            });
        }
    }

    uiActionsRegister()->reg(m_uiActions);
}

void ExtensionsActionController::onPluginTriggered(const UriQuery& q)
{
    const Manifest& m = provider()->manifest(q.uri());
    if (!m.isValid()) {
        LOGE() << "Not found extension, uri: " << q.uri().toString();
        return;
    }

    if (m.config.enabled) {
        provider()->perform(q);
        return;
    }

    IInteractive::Result result = interactive()->warning(
        muse::qtrc("extensions", "The plugin “%1” is currently disabled. Do you want to enable it now?").arg(m.title).toStdString(),
        muse::trc("extensions", "Alternatively, you can enable it at any time from Home > Plugins."),
        { IInteractive::Button::No, IInteractive::Button::Yes });

    if (result.standardButton() == IInteractive::Button::Yes) {
        provider()->setEnable(q.uri(), true);
        provider()->perform(q);
    }
}
