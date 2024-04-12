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
#ifndef MU_INSPECTOR_AMBITUSSETTINGSMODEL_H
#define MU_INSPECTOR_AMBITUSSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class AmbitusSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * noteheadGroup READ noteheadGroup CONSTANT)
    Q_PROPERTY(PropertyItem * noteheadType READ noteheadType CONSTANT)

    Q_PROPERTY(PropertyItem * topTpc READ topTpc CONSTANT)
    Q_PROPERTY(PropertyItem * bottomTpc READ bottomTpc CONSTANT)
    Q_PROPERTY(PropertyItem * topOctave READ topOctave CONSTANT)
    Q_PROPERTY(PropertyItem * bottomOctave READ bottomOctave CONSTANT)
    Q_PROPERTY(PropertyItem * topPitch READ topPitch CONSTANT)
    Q_PROPERTY(PropertyItem * bottomPitch READ bottomPitch CONSTANT)

    Q_PROPERTY(PropertyItem * direction READ direction CONSTANT)
    Q_PROPERTY(PropertyItem * lineThickness READ lineThickness CONSTANT)

public:
    explicit AmbitusSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void matchRangesToStaff();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* noteheadGroup() const;
    PropertyItem* noteheadType() const;

    PropertyItem* topTpc() const;
    PropertyItem* bottomTpc() const;
    PropertyItem* topOctave() const;
    PropertyItem* bottomOctave() const;
    PropertyItem* topPitch() const;
    PropertyItem* bottomPitch() const;

    PropertyItem* direction() const;
    PropertyItem* lineThickness() const;

private:
    PropertyItem* m_noteheadGroup = nullptr;
    PropertyItem* m_noteheadType = nullptr;

    PropertyItem* m_topTpc = nullptr;
    PropertyItem* m_bottomTpc = nullptr;
    PropertyItem* m_topOctave = nullptr;
    PropertyItem* m_bottomOctave = nullptr;
    PropertyItem* m_topPitch = nullptr;
    PropertyItem* m_bottomPitch = nullptr;

    PropertyItem* m_direction = nullptr;
    PropertyItem* m_lineThickness = nullptr;
};
}

#endif // MU_INSPECTOR_AMBITUSSETTINGSMODEL_H
