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
#include "notesettingsproxymodel.h"

#include "translation.h"

#include "stems/stemsettingsmodel.h"
#include "noteheads/noteheadsettingsmodel.h"
#include "beams/beamsettingsmodel.h"
#include "hooks/hooksettingsmodel.h"

using namespace mu::inspector;

static const QMap<mu::engraving::ElementType, InspectorModelType> NOTE_PART_TYPES {
    { mu::engraving::ElementType::NOTEHEAD, InspectorModelType::TYPE_NOTEHEAD },
    { mu::engraving::ElementType::CHORD, InspectorModelType::TYPE_CHORD },
    { mu::engraving::ElementType::STEM, InspectorModelType::TYPE_STEM },
    { mu::engraving::ElementType::BEAM, InspectorModelType::TYPE_BEAM },
    { mu::engraving::ElementType::HOOK, InspectorModelType::TYPE_HOOK },
};

NoteSettingsProxyModel::NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_NOTE);
    setTitle(muse::qtrc("inspector", "Note"));
    setIcon(muse::ui::IconCode::Code::MUSIC_NOTES);

    QList<AbstractInspectorModel*> models;
    for (InspectorModelType modelType : NOTE_PART_TYPES) {
        models << inspectorModelCreator()->newInspectorModel(modelType, this, repository);
    }

    for (AbstractInspectorModel* model : models) {
        model->init();
    }

    setModels(models);

    connect(m_repository->getQObject(), SIGNAL(elementsUpdated(const QList<mu::engraving::EngravingItem*>&)), this,
            SLOT(onElementsUpdated(const QList<mu::engraving::EngravingItem*>&)));
}

void NoteSettingsProxyModel::onElementsUpdated(const QList<mu::engraving::EngravingItem*>& newElements)
{
    InspectorModelType defaultType = resolveDefaultSubModelType(newElements);

    setDefaultSubModelType(defaultType);
}

InspectorModelType NoteSettingsProxyModel::resolveDefaultSubModelType(const QList<mu::engraving::EngravingItem*>& newElements) const
{
    InspectorModelType defaultModelType = InspectorModelType::TYPE_NOTEHEAD;

    for (const mu::engraving::EngravingItem* element : newElements) {
        if (NOTE_PART_TYPES.contains(element->type())) {
            defaultModelType = NOTE_PART_TYPES[element->type()];
        }
    }

    return defaultModelType;
}
