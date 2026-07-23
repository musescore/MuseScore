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

#include "notesettingsproxymodel.h"

#include "notation/inotationselection.h" // IWYU pragma: keep

#include "propertiespanelmodelfactory.h"
#include "translation.h"

using namespace mu::propertiespanel;

static const QMap<mu::engraving::ElementType, PropertiesPanelModelType> NOTE_PART_TYPES {
    { mu::engraving::ElementType::NOTEHEAD, PropertiesPanelModelType::TYPE_NOTEHEAD },
    { mu::engraving::ElementType::CHORD, PropertiesPanelModelType::TYPE_CHORD },
    { mu::engraving::ElementType::STEM, PropertiesPanelModelType::TYPE_STEM },
    { mu::engraving::ElementType::BEAM, PropertiesPanelModelType::TYPE_BEAM },
    { mu::engraving::ElementType::HOOK, PropertiesPanelModelType::TYPE_HOOK },
};

NoteSettingsProxyModel::NoteSettingsProxyModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                               IElementRepositoryService* repository)
    : PropertiesPanelAbstractProxyModel(parent, iocCtx, repository)
{
    setModelType(PropertiesPanelModelType::TYPE_NOTE);
    setTitle(muse::qtrc("propertiespanel", "Note"));
    setIcon(muse::ui::IconCode::Code::MUSIC_NOTES);

    QList<PropertiesPanelAbstractModel*> models;
    for (PropertiesPanelModelType modelType : NOTE_PART_TYPES) {
        models << PropertiesPanelModelFactory::newPropertiesPanelListModel(modelType, this, iocCtx, repository);
    }

    for (PropertiesPanelAbstractModel* model : models) {
        model->init();
    }

    setModels(models);

    m_repository->elementsUpdated().onReceive(this, [this](const QList<mu::engraving::EngravingItem*>& newElements) {
        updateProperties();
        onElementsUpdated(newElements);
    }, Mode::SetReplace /* override PropertiesPanelAbstractModel's callback */);
}

void NoteSettingsProxyModel::onElementsUpdated(const QList<mu::engraving::EngravingItem*>& newElements)
{
    if (!selection() || selection()->isRange()) {
        // Don't update the default sub model if the selection is a range (see issue #30581)
        return;
    }
    PropertiesPanelModelType defaultType = resolveDefaultSubModelType(newElements);
    setDefaultSubModelType(defaultType);
}

PropertiesPanelModelType NoteSettingsProxyModel::resolveDefaultSubModelType(const QList<mu::engraving::EngravingItem*>& newElements) const
{
    PropertiesPanelModelType defaultModelType = PropertiesPanelModelType::TYPE_NOTEHEAD;

    for (const mu::engraving::EngravingItem* element : newElements) {
        if (NOTE_PART_TYPES.contains(element->type())) {
            defaultModelType = NOTE_PART_TYPES[element->type()];
        }
    }

    return defaultModelType;
}
