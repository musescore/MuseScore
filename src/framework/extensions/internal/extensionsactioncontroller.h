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
#ifndef MUSE_EXTENSIONS_EXTENSIONSACTIONCONTROLLER_H
#define MUSE_EXTENSIONS_EXTENSIONSACTIONCONTROLLER_H

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "extensions/iextensionsprovider.h"
#include "ui/iuiactionsregister.h"

namespace muse::extensions {
class ExtensionsUiActions;
class ExtensionsActionController : public muse::actions::Actionable, public async::Asyncable
{
    Inject<IInteractive> interactive;
    Inject<muse::actions::IActionsDispatcher> dispatcher;
    Inject<extensions::IExtensionsProvider> provider;
    Inject<ui::IUiActionsRegister> uiActionsRegister;

public:
    void init();

private:
    void registerPlugins();

    void onPluginTriggered(const UriQuery& uri);

    std::shared_ptr<ExtensionsUiActions> m_uiActions;
};
}

#endif // MUSE_EXTENSIONS_EXTENSIONSACTIONCONTROLLER_H
