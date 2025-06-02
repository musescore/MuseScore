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
#include "extensionstoolbarmodel.h"

#include "uicomponents/view/toolbaritem.h"

#include "log.h"

using namespace muse::extensions;
using namespace muse::uicomponents;

void ExtensionsToolBarModel::load()
{
    extensionsProvider()->manifestListChanged().onNotify(this, [this]() {
        load();
    });

    extensionsProvider()->manifestChanged().onReceive(this, [this](const Manifest&) {
        load();
    });

    ToolBarItemList items;
    ManifestList enabledExtensions = extensionsProvider()->manifestList(Filter::Enabled);
    for (const Manifest& m : enabledExtensions) {
        for (const muse::extensions::Action& a : m.actions) {
            if (!a.showOnToolbar) {
                continue;
            }

            items << makeItem(makeActionCode(m.uri, a.code));
        }
    }

    setItems(items);

    AbstractToolBarModel::load();
}
