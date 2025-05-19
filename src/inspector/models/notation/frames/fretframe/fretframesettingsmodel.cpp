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
#include "fretframesettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

FretFrameSettingsModel::FretFrameSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_FRET_FRAME_SETTINGS);
    createProperties();
}

void FretFrameSettingsModel::createProperties()
{
    m_textScale = buildPropertyItem(Pid::FRET_FRAME_TEXT_SCALE,
                                    [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    });
    m_diagramScale = buildPropertyItem(Pid::FRET_FRAME_DIAGRAM_SCALE,
                                       [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    });
    m_columnGap = buildPropertyItem(Pid::FRET_FRAME_COLUMN_GAP);
    m_rowGap = buildPropertyItem(Pid::FRET_FRAME_ROW_GAP);
    m_chordsPerRow = buildPropertyItem(Pid::FRET_FRAME_CHORDS_PER_ROW);

    m_horizontalAlignment
        = buildPropertyItem(mu::engraving::Pid::FRET_FRAME_H_ALIGN, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toInt());

        emit requestReloadPropertyItems();
    });

    m_frameHeight = buildPropertyItem(Pid::BOX_HEIGHT);
    m_gapAbove = buildPropertyItem(Pid::TOP_GAP);
    m_gapBelow = buildPropertyItem(Pid::BOTTOM_GAP);
    m_frameLeftMargin = buildPropertyItem(Pid::LEFT_MARGIN);
    m_frameRightMargin = buildPropertyItem(Pid::RIGHT_MARGIN);
    m_frameTopMargin = buildPropertyItem(Pid::TOP_MARGIN);
    m_frameBottomMargin = buildPropertyItem(Pid::BOTTOM_MARGIN);
    m_isSizeSpatiumDependent = buildPropertyItem(Pid::SIZE_SPATIUM_DEPENDENT);
}

void FretFrameSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::FBOX);
}

void FretFrameSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::FRET_FRAME_TEXT_SCALE,
        Pid::FRET_FRAME_DIAGRAM_SCALE,
        Pid::FRET_FRAME_COLUMN_GAP,
        Pid::FRET_FRAME_ROW_GAP,
        Pid::FRET_FRAME_CHORDS_PER_ROW,
        Pid::FRET_FRAME_H_ALIGN,
        Pid::BOX_HEIGHT,
        Pid::TOP_GAP,
        Pid::BOTTOM_GAP,
        Pid::LEFT_MARGIN,
        Pid::RIGHT_MARGIN,
        Pid::TOP_MARGIN,
        Pid::BOTTOM_MARGIN,
        Pid::SIZE_SPATIUM_DEPENDENT
    };

    loadProperties(propertyIdSet);
}

void FretFrameSettingsModel::resetProperties()
{
    m_textScale->resetToDefault();
    m_diagramScale->resetToDefault();
    m_columnGap->resetToDefault();
    m_rowGap->resetToDefault();
    m_chordsPerRow->resetToDefault();
    m_horizontalAlignment->resetToDefault();
    m_frameHeight->resetToDefault();
    m_gapAbove->resetToDefault();
    m_gapBelow->resetToDefault();
    m_frameLeftMargin->resetToDefault();
    m_frameRightMargin->resetToDefault();
    m_frameTopMargin->resetToDefault();
    m_frameBottomMargin->resetToDefault();
    m_isSizeSpatiumDependent->resetToDefault();
}

void FretFrameSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void FretFrameSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::FRET_FRAME_TEXT_SCALE)) {
        loadPropertyItem(m_textScale, [](const QVariant& elementPropertyValue) -> QVariant {
            return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
        });
    }

    if (muse::contains(propertyIdSet, Pid::FRET_FRAME_DIAGRAM_SCALE)) {
        loadPropertyItem(m_diagramScale, [](const QVariant& elementPropertyValue) -> QVariant {
            return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
        });
    }

    if (muse::contains(propertyIdSet, Pid::FRET_FRAME_COLUMN_GAP)) {
        loadPropertyItem(m_columnGap, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::FRET_FRAME_ROW_GAP)) {
        loadPropertyItem(m_rowGap, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::FRET_FRAME_CHORDS_PER_ROW)) {
        loadPropertyItem(m_chordsPerRow);
    }

    if (muse::contains(propertyIdSet, Pid::FRET_FRAME_H_ALIGN)) {
        loadPropertyItem(m_horizontalAlignment);
    }

    if (muse::contains(propertyIdSet, Pid::BOX_HEIGHT)) {
        loadPropertyItem(m_frameHeight, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::TOP_GAP)) {
        loadPropertyItem(m_gapAbove, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::BOTTOM_GAP)) {
        loadPropertyItem(m_gapBelow, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::LEFT_MARGIN)) {
        loadPropertyItem(m_frameLeftMargin);
    }

    if (muse::contains(propertyIdSet, Pid::RIGHT_MARGIN)) {
        loadPropertyItem(m_frameRightMargin);
    }

    if (muse::contains(propertyIdSet, Pid::TOP_MARGIN)) {
        loadPropertyItem(m_frameTopMargin);
    }

    if (muse::contains(propertyIdSet, Pid::BOTTOM_MARGIN)) {
        loadPropertyItem(m_frameBottomMargin);
    }

    if (muse::contains(propertyIdSet, Pid::SIZE_SPATIUM_DEPENDENT)) {
        loadPropertyItem(m_isSizeSpatiumDependent);
    }
}

PropertyItem* FretFrameSettingsModel::textScale() const
{
    return m_textScale;
}

PropertyItem* FretFrameSettingsModel::diagramScale() const
{
    return m_diagramScale;
}

PropertyItem* FretFrameSettingsModel::columnGap() const
{
    return m_columnGap;
}

PropertyItem* FretFrameSettingsModel::rowGap() const
{
    return m_rowGap;
}

PropertyItem* FretFrameSettingsModel::chordsPerRow() const
{
    return m_chordsPerRow;
}

PropertyItem* FretFrameSettingsModel::horizontalAlignment() const
{
    return m_horizontalAlignment;
}

PropertyItem* FretFrameSettingsModel::frameHeight() const
{
    return m_frameHeight;
}

PropertyItem* FretFrameSettingsModel::gapAbove() const
{
    return m_gapAbove;
}

PropertyItem* FretFrameSettingsModel::gapBelow() const
{
    return m_gapBelow;
}

PropertyItem* FretFrameSettingsModel::frameLeftMargin() const
{
    return m_frameLeftMargin;
}

PropertyItem* FretFrameSettingsModel::frameRightMargin() const
{
    return m_frameRightMargin;
}

PropertyItem* FretFrameSettingsModel::frameTopMargin() const
{
    return m_frameTopMargin;
}

PropertyItem* FretFrameSettingsModel::frameBottomMargin() const
{
    return m_frameBottomMargin;
}

PropertyItem* FretFrameSettingsModel::isSizeSpatiumDependent() const
{
    return m_isSizeSpatiumDependent;
}
