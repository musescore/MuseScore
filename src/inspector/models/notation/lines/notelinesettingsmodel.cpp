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

#include "notelinesettingsmodel.h"

using namespace mu::inspector;

NoteLineSettingsModel::NoteLineSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : TextLineSettingsModel(parent, repository, mu::engraving::ElementType::NOTELINE)
{
    setModelType(InspectorModelType::TYPE_NOTELINE);
    setTitle(muse::qtrc("inspector", "Note-anchored line"));
    setIcon(muse::ui::IconCode::Code::NOTE_ANCHORED_LINE);

    createProperties();
}

void NoteLineSettingsModel::createProperties()
{
    TextLineSettingsModel::createProperties();

    isLineVisible()->setIsVisible(true);
    allowDiagonal()->setIsVisible(false);
    placement()->setIsVisible(false);
}
