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
#pragma once

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class SlurAndTieSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * lineStyle READ lineStyle CONSTANT)
    Q_PROPERTY(PropertyItem * direction READ direction CONSTANT)
    Q_PROPERTY(PropertyItem * tiePlacement READ tiePlacement CONSTANT)
    Q_PROPERTY(PropertyItem * minLength READ minLength CONSTANT)
    Q_PROPERTY(bool isTiePlacementAvailable READ isTiePlacementAvailable NOTIFY isTiePlacementAvailableChanged)
    Q_PROPERTY(bool isMinLengthAvailable READ isMinLengthAvailable NOTIFY isMinLengthAvailableChanged)
    Q_PROPERTY(bool isLineStyleAvailable READ isLineStyleAvailable NOTIFY isLineStyleAvailableChanged)
    Q_PROPERTY(bool isLaissezVib READ isLaissezVib CONSTANT)

public:
    enum ElementType {
        Slur,
        Tie,
        LaissezVib,
        PartialTie
    };

    explicit SlurAndTieSettingsModel(QObject* parent, IElementRepositoryService* repository, ElementType elementType);

    PropertyItem* lineStyle() const;
    PropertyItem* direction() const;
    PropertyItem* tiePlacement() const;
    PropertyItem* minLength() const;

    bool isLaissezVib() const;

    bool isTiePlacementAvailable() const;
    bool isMinLengthAvailable() const;
    bool isLineStyleAvailable() const;

    Q_INVOKABLE QVariantList possibleLineStyles() const;

signals:
    void isTiePlacementAvailableChanged(bool available);
    void isMinLengthAvailableChanged(bool available);
    void isLineStyleAvailableChanged(bool available);

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

    void updateIsTiePlacementAvailable();
    void updateIsMinLengthAvailable();
    void updateisLineStyleAvailable();

    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_direction = nullptr;
    PropertyItem* m_tiePlacement = nullptr;
    PropertyItem* m_minLength = nullptr;

    bool m_isTiePlacementAvailable = false;
    bool m_isMinLengthAvailable = false;
    bool m_isLineStyleAvailable = false;

    bool m_isLaissezVib = false;
};
}
