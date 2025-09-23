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
    m_listOrder = buildPropertyItem(Pid::FRET_FRAME_DIAGRAMS_ORDER,
                                    [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        m_chordListModel->load();
    });
}

void FretFrameChordsSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::FBOX);

    m_chordListModel->setFBox(fretBox());
    m_chordListModel->load();

    updateHasInvisibleChords();
}

void FretFrameChordsSettingsModel::loadProperties()
{
    loadProperties({ Pid::FRET_FRAME_DIAGRAMS_ORDER });
    m_chordListModel->load();
    updateHasInvisibleChords();
}

void FretFrameChordsSettingsModel::resetProperties()
{
    m_listOrder->resetToDefault();
}

mu::engraving::FBox* FretFrameChordsSettingsModel::fretBox() const
{
    if (m_elementList.isEmpty()) {
        return nullptr;
    }

    return toFBox(m_elementList.first());
}

void FretFrameChordsSettingsModel::onNotationChanged(const engraving::PropertyIdSet& changedPropertyIdSet,
                                                     const engraving::StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
    updateHasInvisibleChords();
}

void FretFrameChordsSettingsModel::loadProperties(const engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::FRET_FRAME_DIAGRAMS_ORDER)) {
        loadPropertyItem(m_listOrder);
    }
}

void FretFrameChordsSettingsModel::updateHasInvisibleChords()
{
    bool hasInvisibleChords = false;

    const FBox* fbox = fretBox();
    if (fbox) {
        for (EngravingItem* item : fbox->el()) {
            if (!item->visible()) {
                hasInvisibleChords = true;
                break;
            }
        }
    }

    if (hasInvisibleChords != m_hasInvisibleChords) {
        m_hasInvisibleChords = hasInvisibleChords;
        emit hasInvisibleChordsChanged(m_hasInvisibleChords);
    }
}

FretFrameChordListModel* FretFrameChordsSettingsModel::chordListModel() const
{
    return m_chordListModel.get();
}

PropertyItem* FretFrameChordsSettingsModel::listOrder() const
{
    return m_listOrder;
}

bool FretFrameChordsSettingsModel::hasInvisibleChords() const
{
    return m_hasInvisibleChords;
}

void FretFrameChordsSettingsModel::resetList()
{
    beginCommand(TranslatableString("undoableAction", "Reset fretboard diagram legend chords list"));

    FBox* fbox = fretBox();
    if (fbox) {
        fbox->undoResetProperty(Pid::FRET_FRAME_DIAGRAMS_ORDER);

        for (EngravingItem* item : fbox->el()) {
            item->undoResetProperty(Pid::VISIBLE);
        }
    }

    updateNotation();
    endCommand();

    loadProperties();
}
