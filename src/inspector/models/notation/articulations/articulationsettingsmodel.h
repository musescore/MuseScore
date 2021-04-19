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
#ifndef MU_INSPECTOR_ARTICULATIONSETTINGSMODEL_H
#define MU_INSPECTOR_ARTICULATIONSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ArticulationSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * direction READ direction CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

public:
    explicit ArticulationSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void openChannelAndMidiProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* direction() const;
    PropertyItem* placement() const;

private:
    PropertyItem* m_direction = nullptr;
    PropertyItem* m_placement = nullptr;
};
}

#endif // MU_INSPECTOR_ARTICULATIONSETTINGSMODEL_H
