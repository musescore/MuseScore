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
#ifndef MU_INSPECTOR_ACCIDENTALSETTINGSMODEL_H
#define MU_INSPECTOR_ACCIDENTALSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class AccidentalSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bracketType READ bracketType CONSTANT)
    Q_PROPERTY(PropertyItem * isSmall READ isSmall CONSTANT)
    Q_PROPERTY(bool isSmallAvailable READ isSmallAvailable NOTIFY isSmallAvailableChanged)

    Q_PROPERTY(PropertyItem * stackingOrderOffset READ stackingOrderOffset CONSTANT)
    Q_PROPERTY(bool isStackingOrderAvailable READ isStackingOrderAvailable NOTIFY isStackingOrderAvailableChanged)
    Q_PROPERTY(bool isStackingOrderEnabled READ isStackingOrderEnabled NOTIFY isStackingOrderEnabledChanged)

public:
    explicit AccidentalSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* bracketType() const;
    PropertyItem* isSmall() const;
    PropertyItem* stackingOrderOffset() const;
    bool isStackingOrderAvailable() const;
    bool isStackingOrderEnabled() const;

    bool isSmallAvailable() const;

signals:
    void isSmallAvailableChanged(bool available);
    void isStackingOrderAvailableChanged(bool available);
    void isStackingOrderEnabledChanged(bool enabled);

private:
    void updateIsSmallAvailable();
    void setIsSmallAvailable(bool available);

    void updateIsStackingOrderAvailableAndEnabled();
    void setIsStackingOrderAvailable(bool available);
    void setIsStackingOrderEnabled(bool enabled);

private:
    PropertyItem* m_bracketType = nullptr;
    PropertyItem* m_isSmall = nullptr;
    bool m_isSmallAvailable = true;

    PropertyItem* m_stackingOrderOffset = nullptr;
    bool m_isStackinOrderAvailable = false;
    bool m_isStackingOrderEnabled = false;
};
}

#endif // MU_INSPECTOR_ACCIDENTALSETTINGSMODEL_H
