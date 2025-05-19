/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "fretframesettingsproxymodel.h"

#include "fretframe/fretframesettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

FretFrameSettingsProxyModel::FretFrameSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_FRET_FRAME);
    setTitle(muse::qtrc("inspector", "Fretboard diagram legend"));
    setIcon(muse::ui::IconCode::Code::FRET_FRAME);

    QList<AbstractInspectorModel*> models {
        inspectorModelCreator()->newInspectorModel(InspectorModelType::TYPE_FRET_FRAME_SETTINGS, this, repository)
    };

    for (AbstractInspectorModel* model : models) {
        model->init();
    }

    setModels(models);
}
