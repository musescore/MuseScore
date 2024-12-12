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
#ifndef MU_INSPECTOR_DYNAMICSETTINGSMODEL_H
#define MU_INSPECTOR_DYNAMICSETTINGSMODEL_H

#include "models/inspectormodelwithvoiceandpositionoptions.h"

namespace mu::inspector {
class DynamicsSettingsModel : public InspectorModelWithVoiceAndPositionOptions
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * avoidBarLines READ avoidBarLines CONSTANT)
    Q_PROPERTY(PropertyItem * dynamicSize READ dynamicSize CONSTANT)
    Q_PROPERTY(PropertyItem * centerOnNotehead READ centerOnNotehead CONSTANT)

    // Border-related settings
    Q_PROPERTY(PropertyItem * borderType READ borderType CONSTANT)
    Q_PROPERTY(PropertyItem * borderColor READ borderColor CONSTANT)
    Q_PROPERTY(PropertyItem * borderFillColor READ borderFillColor CONSTANT)
    Q_PROPERTY(PropertyItem * borderThickness READ borderThickness CONSTANT)
    Q_PROPERTY(PropertyItem * borderMargin READ borderMargin CONSTANT)
    Q_PROPERTY(PropertyItem * borderCornerRadius READ borderCornerRadius CONSTANT)

public:
    explicit DynamicsSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* avoidBarLines() const;
    PropertyItem* dynamicSize() const;
    PropertyItem* centerOnNotehead() const;

    PropertyItem* borderType() const;
    PropertyItem* borderColor() const;
    PropertyItem* borderFillColor() const;
    PropertyItem* borderThickness() const;
    PropertyItem* borderMargin() const;
    PropertyItem* borderCornerRadius() const;

private:
    void updateBorderPropertiesAvailability();

private:
    PropertyItem* m_avoidBarLines = nullptr;
    PropertyItem* m_dynamicSize = nullptr;
    PropertyItem* m_centerOnNotehead = nullptr;

    PropertyItem* m_borderType = nullptr;
    PropertyItem* m_borderColor = nullptr;
    PropertyItem* m_borderFillColor = nullptr;
    PropertyItem* m_borderThickness = nullptr;
    PropertyItem* m_borderMargin = nullptr;
    PropertyItem* m_borderCornerRadius = nullptr;
};
}

#endif // MU_INSPECTOR_DYNAMICSETTINGSMODEL_H
