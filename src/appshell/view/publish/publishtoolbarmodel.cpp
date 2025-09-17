/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "publishtoolbarmodel.h"

#include "uicomponents/view/toolbaritem.h"

using namespace mu::appshell;
using namespace muse::uicomponents;
using namespace muse::actions;

void PublishToolBarModel::load()
{
    AbstractToolBarModel::load();

    ToolBarItemList items;
    const auto addItem = [this, &items](const muse::actions::ActionCode& code, const muse::TranslatableString& description = {}) {
        auto* item = makeItem(code);
        item->setShowTitle(true);

        if (!description.isEmpty()) {
            item->setDescription(description);
        }

        items << item;
    };

    addItem("file-save-to-cloud", muse::TranslatableString("project/save", "Save this score to your MuseScore.com account"));
    addItem("print");
    addItem("file-export");
    items << makeSeparator();
    addItem("file-publish", muse::TranslatableString("project/save", "Share this score and its audio on MuseScore.com"));
    addItem("file-share-audio", muse::TranslatableString("project/save", "Share the audio from this score on Audio.com"));

    setItems(items);
}
