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
#include "fretframechordssettingsmodel.h"

#include "dom/box.h"
#include "dom/fret.h"

#include "fretframechorditem.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

FretFrameChordsSettingsModel::FretFrameChordsSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_FRET_FRAME_CHORDS);
    createProperties();

    m_chordListModel = std::make_shared<FretFrameChordListModel>(this);
}

void FretFrameChordsSettingsModel::createProperties()
{
}

void FretFrameChordsSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::FBOX);
    loadListItems();
}

void FretFrameChordsSettingsModel::loadProperties()
{
}

void FretFrameChordsSettingsModel::resetProperties()
{
}

mu::engraving::FBox* FretFrameChordsSettingsModel::fretBox() const
{
    if (m_elementList.isEmpty()) {
        return nullptr;
    }

    return toFBox(m_elementList.first());
}

void FretFrameChordsSettingsModel::onNotationChanged(const engraving::PropertyIdSet& changedPropertyIdSet,
                                                     const engraving::StyleIdSet& changedStyleIdSet)
{
    loadProperties(changedPropertyIdSet);
}

void FretFrameChordsSettingsModel::loadProperties(const engraving::PropertyIdSet& propertyIdSet)
{
    loadListItems();
}

void FretFrameChordsSettingsModel::loadListItems()
{
    m_chordListModel->setChordItems({});

    FBox* box = fretBox();
    if (!box) {
        return;
    }

    QList<FretFrameChordListModel::Item*> items;

    for (FretDiagram* diagram : box->fretDiagrams()) {
        auto chordItem = new FretFrameChordItem(m_chordListModel.get());
        chordItem->setTitle(diagram->harmony()->plainText());
        // TODO: set isVisible based on diagram visibility

        items << chordItem;
    }

    m_chordListModel->setChordItems(items);
}

FretFrameChordListModel* FretFrameChordsSettingsModel::chordListModel() const
{
    return m_chordListModel.get();
}
