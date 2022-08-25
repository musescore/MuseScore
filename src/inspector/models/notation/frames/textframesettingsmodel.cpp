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
#include "textframesettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

TextFrameSettingsModel::TextFrameSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TEXT_FRAME);
    setTitle(qtrc("inspector", "Text frame"));
    setIcon(ui::IconCode::Code::TEXT_FRAME);
    createProperties();
}

void TextFrameSettingsModel::createProperties()
{
    m_gapAbove = buildPropertyItem(mu::engraving::Pid::TOP_GAP);
    m_gapBelow = buildPropertyItem(mu::engraving::Pid::BOTTOM_GAP);
    m_frameLeftMargin = buildPropertyItem(mu::engraving::Pid::LEFT_MARGIN);
    m_frameRightMargin = buildPropertyItem(mu::engraving::Pid::RIGHT_MARGIN);
    m_frameTopMargin = buildPropertyItem(mu::engraving::Pid::TOP_MARGIN);
    m_frameBottomMargin = buildPropertyItem(mu::engraving::Pid::BOTTOM_MARGIN);
}

void TextFrameSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::TBOX);
}

void TextFrameSettingsModel::loadProperties()
{
    loadPropertyItem(m_gapAbove, formatDoubleFunc);
    loadPropertyItem(m_gapBelow, formatDoubleFunc);
    loadPropertyItem(m_frameLeftMargin);
    loadPropertyItem(m_frameRightMargin);
    loadPropertyItem(m_frameTopMargin);
    loadPropertyItem(m_frameBottomMargin);
}

void TextFrameSettingsModel::resetProperties()
{
    m_gapAbove->resetToDefault();
    m_gapBelow->resetToDefault();
    m_frameLeftMargin->resetToDefault();
    m_frameRightMargin->resetToDefault();
    m_frameTopMargin->resetToDefault();
    m_frameBottomMargin->resetToDefault();
}

void TextFrameSettingsModel::onNotationChanged()
{
    loadProperties();
}

PropertyItem* TextFrameSettingsModel::gapAbove() const
{
    return m_gapAbove;
}

PropertyItem* TextFrameSettingsModel::gapBelow() const
{
    return m_gapBelow;
}

PropertyItem* TextFrameSettingsModel::frameLeftMargin() const
{
    return m_frameLeftMargin;
}

PropertyItem* TextFrameSettingsModel::frameRightMargin() const
{
    return m_frameRightMargin;
}

PropertyItem* TextFrameSettingsModel::frameTopMargin() const
{
    return m_frameTopMargin;
}

PropertyItem* TextFrameSettingsModel::frameBottomMargin() const
{
    return m_frameBottomMargin;
}
