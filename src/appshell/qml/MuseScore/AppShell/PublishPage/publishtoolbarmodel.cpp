/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "uicomponents/qml/Muse/UiComponents/toolbaritem.h"

#include "project/projectcommands.h"

using namespace mu::appshell;
using namespace muse::uicomponents;
using namespace muse::rcommand;
using namespace mu::project;

void PublishToolBarModel::load()
{
    AbstractToolBarModel::load();

    ToolBarItemList items;
    const auto addItem = [this, &items](const Command& command, const muse::TranslatableString& description = {}) {
        auto* item = makeItem(command);
        item->setShowTitle(true);

        if (!description.isEmpty()) {
            item->setDescription(description);
        }

        items << item;
    };

    addItem(PROJECT_PRINT_COMMAND);
    addItem(PROJECT_PUBLISH_COMMAND, muse::TranslatableString("project/save", "Share this score and its audio on MuseScore.com"));
    addItem(PROJECT_SHARE_AUDIO_COMMAND, muse::TranslatableString("project/save", "Share the audio from this score on Audio.com"));
    addItem(PROJECT_EXPORT_COMMAND);

    setItems(items);
}
