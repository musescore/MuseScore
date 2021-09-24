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
#ifndef MU_INSPECTOR_HAIRPINPLAYBACKMODEL_H
#define MU_INSPECTOR_HAIRPINPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class HairpinPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * scopeType READ scopeType CONSTANT)
    Q_PROPERTY(PropertyItem * velocityChange READ velocityChange CONSTANT)
    Q_PROPERTY(PropertyItem * useSingleNoteDynamics READ useSingleNoteDynamics CONSTANT)
    Q_PROPERTY(PropertyItem * velocityChangeType READ velocityChangeType CONSTANT)

public:
    explicit HairpinPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* scopeType() const;
    PropertyItem* velocityChange() const;
    PropertyItem* useSingleNoteDynamics() const;
    PropertyItem* velocityChangeType() const;

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* m_scopeType = nullptr;
    PropertyItem* m_velocityChange = nullptr;
    PropertyItem* m_useSingleNoteDynamics = nullptr;
    PropertyItem* m_velocityChangeType = nullptr;
};
}

#endif // MU_INSPECTOR_HAIRPINPLAYBACKMODEL_H
