/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "expressionsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

ExpressionSettingsModel::ExpressionSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : InspectorModelWithVoiceAndPositionOptions(parent, repository)
{
    setModelType(InspectorModelType::TYPE_EXPRESSION);
    setTitle(muse::qtrc("inspector ", "Expression"));
    setIcon(muse::ui::IconCode::Code::EXPRESSION);
    createProperties();
}

void ExpressionSettingsModel::createProperties()
{
    InspectorModelWithVoiceAndPositionOptions::createProperties();

    m_snapExpression = buildPropertyItem(mu::engraving::Pid::SNAP_TO_DYNAMICS);
}

void ExpressionSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::EXPRESSION);
}

void ExpressionSettingsModel::loadProperties()
{
    InspectorModelWithVoiceAndPositionOptions::loadProperties();

    loadPropertyItem(m_snapExpression);
}

void ExpressionSettingsModel::resetProperties()
{
    InspectorModelWithVoiceAndPositionOptions::resetProperties();

    m_snapExpression->resetToDefault();
}

PropertyItem* ExpressionSettingsModel::snapExpression() const
{
    return m_snapExpression;
}
