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
#include "verticalframesettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

VerticalFrameSettingsModel::VerticalFrameSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_VERTICAL_FRAME);
    setTitle(muse::qtrc("inspector", "Vertical frame"));
    setIcon(muse::ui::IconCode::Code::VERTICAL_FRAME);
    createProperties();
}

void VerticalFrameSettingsModel::createProperties()
{
    m_frameHeight = buildPropertyItem(Pid::BOX_HEIGHT);
    m_gapAbove = buildPropertyItem(Pid::TOP_GAP);
    m_gapBelow = buildPropertyItem(Pid::BOTTOM_GAP);
    m_frameLeftMargin = buildPropertyItem(Pid::LEFT_MARGIN);
    m_frameRightMargin = buildPropertyItem(Pid::RIGHT_MARGIN);
    m_frameTopMargin = buildPropertyItem(Pid::TOP_MARGIN);
    m_frameBottomMargin = buildPropertyItem(Pid::BOTTOM_MARGIN);
    m_isSizeSpatiumDependent = buildPropertyItem(Pid::SIZE_SPATIUM_DEPENDENT);
    m_paddingToNotationAbove = buildPropertyItem(Pid::PADDING_TO_NOTATION_ABOVE);
    m_paddingToNotationBelow = buildPropertyItem(Pid::PADDING_TO_NOTATION_BELOW);
}

void VerticalFrameSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::VBOX);
}

void VerticalFrameSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::BOX_HEIGHT,
        Pid::TOP_GAP,
        Pid::BOTTOM_GAP,
        Pid::LEFT_MARGIN,
        Pid::RIGHT_MARGIN,
        Pid::TOP_MARGIN,
        Pid::BOTTOM_MARGIN,
        Pid::SIZE_SPATIUM_DEPENDENT,
        Pid::PADDING_TO_NOTATION_ABOVE,
        Pid::PADDING_TO_NOTATION_BELOW,
    };

    loadProperties(propertyIdSet);
}

void VerticalFrameSettingsModel::resetProperties()
{
    m_frameHeight->resetToDefault();
    m_gapAbove->resetToDefault();
    m_gapBelow->resetToDefault();
    m_frameLeftMargin->resetToDefault();
    m_frameRightMargin->resetToDefault();
    m_frameTopMargin->resetToDefault();
    m_frameBottomMargin->resetToDefault();
    m_isSizeSpatiumDependent->resetToDefault();
    m_paddingToNotationAbove->resetToDefault();
    m_paddingToNotationBelow->resetToDefault();
}

void VerticalFrameSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void VerticalFrameSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
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

    if (muse::contains(propertyIdSet, Pid::PADDING_TO_NOTATION_ABOVE)) {
        loadPropertyItem(m_paddingToNotationAbove);
    }

    if (muse::contains(propertyIdSet, Pid::PADDING_TO_NOTATION_BELOW)) {
        loadPropertyItem(m_paddingToNotationBelow);
    }
}

PropertyItem* VerticalFrameSettingsModel::frameHeight() const
{
    return m_frameHeight;
}

PropertyItem* VerticalFrameSettingsModel::gapAbove() const
{
    return m_gapAbove;
}

PropertyItem* VerticalFrameSettingsModel::gapBelow() const
{
    return m_gapBelow;
}

PropertyItem* VerticalFrameSettingsModel::frameLeftMargin() const
{
    return m_frameLeftMargin;
}

PropertyItem* VerticalFrameSettingsModel::frameRightMargin() const
{
    return m_frameRightMargin;
}

PropertyItem* VerticalFrameSettingsModel::frameTopMargin() const
{
    return m_frameTopMargin;
}

PropertyItem* VerticalFrameSettingsModel::frameBottomMargin() const
{
    return m_frameBottomMargin;
}

PropertyItem* VerticalFrameSettingsModel::isSizeSpatiumDependent() const
{
    return m_isSizeSpatiumDependent;
}

PropertyItem* VerticalFrameSettingsModel::paddingToNotationAbove() const
{
    return m_paddingToNotationAbove;
}

PropertyItem* VerticalFrameSettingsModel::paddingToNotationBelow() const
{
    return m_paddingToNotationBelow;
}
