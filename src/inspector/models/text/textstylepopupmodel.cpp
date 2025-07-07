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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "textstylepopupmodel.h"

#include "internal/services/elementrepositoryservice.h"

using namespace mu::notation;
using namespace mu::inspector;

TextStylePopupModel::TextStylePopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_TEXT, parent)
{
    m_elementRepositoryService = new ElementRepositoryService(this);

    m_textSettingsModel = new TextSettingsModel(this, m_elementRepositoryService);
    m_textSettingsModel->init();
}

void TextStylePopupModel::init()
{
    AbstractElementPopupModel::init();

    m_elementRepositoryService->updateElementList({ m_item }, notation::SelectionState::LIST);
}

TextSettingsModel* TextStylePopupModel::textSettingsModel() const
{
    return m_textSettingsModel;
}
