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
#include "notationsettingsproxymodel.h"

#include "translation.h"

using namespace mu::inspector;

NotationSettingsProxyModel::NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository,
                                                       const QSet<Ms::ElementType>& elementSet)
    : AbstractInspectorProxyModel(parent)
{
    setSectionType(InspectorSectionType::SECTION_NOTATION);

    QList<AbstractInspectorModel::InspectorModelType> modelTypes = this->modelTypes(elementSet);

    if (modelTypes.count() == 1) {
        auto model = inspectorModelCreator()->newInspectorModel(modelTypes.first(), parent, repository);
        setTitle(model->title());
        addModel(model);
    } else {
        setTitle(qtrc("inspector", "Notation"));
        for (const Ms::ElementType elementType : elementSet) {
            AbstractInspectorModel::InspectorSectionType sectionType = AbstractInspectorModel::sectionTypeFromElementType(elementType);
            if (sectionType == AbstractInspectorModel::InspectorSectionType::SECTION_UNDEFINED) {
                continue;
            }

            AbstractInspectorModel::InspectorModelType modelType = AbstractInspectorModel::notationElementModelType(elementType);
            addModel(inspectorModelCreator()->newInspectorModel(modelType, parent, repository));
        }
    }
}

bool NotationSettingsProxyModel::isTypeSupported(Ms::ElementType elementType) const
{
    return AbstractInspectorModel::notationElementModelType(elementType) != AbstractInspectorModel::InspectorModelType::TYPE_UNDEFINED;
}

QList<AbstractInspectorModel::InspectorModelType> NotationSettingsProxyModel::modelTypes(const QSet<Ms::ElementType>& elements)
const
{
    static QList<AbstractInspectorModel::InspectorModelType> notePartTypes {
        AbstractInspectorModel::InspectorModelType::TYPE_NOTE,
        AbstractInspectorModel::InspectorModelType::TYPE_NOTEHEAD,
        AbstractInspectorModel::InspectorModelType::TYPE_STEM,
        AbstractInspectorModel::InspectorModelType::TYPE_HOOK,
        AbstractInspectorModel::InspectorModelType::TYPE_BEAM
    };

    QList<AbstractInspectorModel::InspectorModelType> types;

    AbstractInspectorModel::InspectorModelType noteModelType = AbstractInspectorModel::InspectorModelType::TYPE_UNDEFINED;
    for (const Ms::ElementType elementType : elements) {
        AbstractInspectorModel::InspectorModelType modelType = AbstractInspectorModel::notationElementModelType(elementType);
        if (modelType == AbstractInspectorModel::InspectorModelType::TYPE_UNDEFINED) {
            continue;
        }

        if (types.contains(modelType)) {
            continue;
        }

        if (!types.isEmpty() && notePartTypes.contains(modelType)) { // if element is a part of the note
            if (noteModelType == AbstractInspectorModel::InspectorModelType::TYPE_UNDEFINED) { // if element is the first element of the parts of the note
                types << modelType;
                continue;
            }

            if (noteModelType == modelType) { // if such model has already been added
                continue;
            }

            // if the element is another part of the note, then there will be one note model
            types.removeAll(noteModelType);
            modelType = AbstractInspectorModel::InspectorModelType::TYPE_NOTE;
        }

        types << modelType;
    }

    return types;
}
