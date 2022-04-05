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
#ifndef MU_INSPECTOR_APPEARANCESETTINGSMODEL_H
#define MU_INSPECTOR_APPEARANCESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include "notation/inotationconfiguration.h"

namespace mu::inspector {
class AppearanceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    INJECT(inspector, notation::INotationConfiguration, notationConfiguration)

    Q_PROPERTY(PropertyItem * leadingSpace READ leadingSpace CONSTANT)
    Q_PROPERTY(PropertyItem * barWidth READ barWidth CONSTANT)
    Q_PROPERTY(PropertyItem * minimumDistance READ minimumDistance CONSTANT)
    Q_PROPERTY(PropertyItem * color READ color CONSTANT)
    Q_PROPERTY(PropertyItem * arrangeOrder READ arrangeOrder CONSTANT)
    Q_PROPERTY(PropertyItem * horizontalOffset READ horizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * verticalOffset READ verticalOffset CONSTANT)
    Q_PROPERTY(bool isSnappedToGrid READ isSnappedToGrid WRITE setIsSnappedToGrid NOTIFY isSnappedToGridChanged)

public:
    explicit AppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void pushBackInOrder();
    Q_INVOKABLE void pushFrontInOrder();

    Q_INVOKABLE void configureGrid();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* leadingSpace() const;
    PropertyItem* barWidth() const;
    PropertyItem* minimumDistance() const;
    PropertyItem* color() const;
    PropertyItem* arrangeOrder() const;
    PropertyItem* horizontalOffset() const;
    PropertyItem* verticalOffset() const;

    bool isSnappedToGrid() const;

public slots:
    void setIsSnappedToGrid(bool isSnapped);

signals:
    void isSnappedToGridChanged(bool isSnappedToGrid);

private:
    void updatePropertiesOnNotationChanged() override;

    void loadOffsets();

    PropertyItem* m_leadingSpace = nullptr;
    PropertyItem* m_barWidth = nullptr;
    PropertyItem* m_minimumDistance = nullptr;
    PropertyItem* m_color = nullptr;
    PropertyItem* m_arrangeOrder = nullptr;
    PropertyItem* m_horizontalOffset = nullptr;
    PropertyItem* m_verticalOffset = nullptr;
};
}

#endif // MU_INSPECTOR_APPEARANCESETTINGSMODEL_H
