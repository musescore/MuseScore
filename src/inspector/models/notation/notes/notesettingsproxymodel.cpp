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
#include "notesettingsproxymodel.h"

#include "translation.h"

#include "stems/stemsettingsmodel.h"
#include "noteheads/noteheadsettingsmodel.h"
#include "beams/beamsettingsmodel.h"
#include "hooks/hooksettingsmodel.h"

using namespace mu::inspector;

NoteSettingsProxyModel::NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository,
                                               InspectorModelType preferedSubModelType)
    : AbstractInspectorProxyModel(parent, preferedSubModelType)
{
    setModelType(InspectorModelType::TYPE_NOTE);
    setTitle(qtrc("inspector", "Note"));
    setIcon(ui::IconCode::Code::MUSIC_NOTES);

    addModel(new StemSettingsModel(this, repository));
    addModel(new NoteheadSettingsModel(this, repository));
    addModel(new BeamSettingsModel(this, repository));
    addModel(new HookSettingsModel(this, repository));
}
