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

#pragma once

#include "notation/view/abstractelementpopupmodel.h"

#include "models/text/textsettingsmodel.h"

namespace mu::inspector {
class ElementRepositoryService;

class TextStylePopupModel : public notation::AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(TextSettingsModel * textSettingsModel READ textSettingsModel CONSTANT)

public:
    explicit TextStylePopupModel(QObject* parent = nullptr);

    TextSettingsModel* textSettingsModel() const;

    Q_INVOKABLE void init() override;

private:
    TextSettingsModel* m_textSettingsModel = nullptr;

    ElementRepositoryService* m_elementRepositoryService = nullptr;
};
}
