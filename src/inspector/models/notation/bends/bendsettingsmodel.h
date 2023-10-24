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
#ifndef MU_INSPECTOR_BENDSETTINGSMODEL_H
#define MU_INSPECTOR_BENDSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class BendSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bendDirection READ bendDirection CONSTANT)
    Q_PROPERTY(PropertyItem * showHoldLine READ showHoldLine CONSTANT)
    Q_PROPERTY(bool isShowHoldLineAvailable READ isShowHoldLineAvailable NOTIFY isShowHoldLineAvailableChanged)

    Q_PROPERTY(PropertyItem * bendType READ bendType CONSTANT)
    Q_PROPERTY(PropertyItem * bendCurve READ bendCurve CONSTANT)
    Q_PROPERTY(PropertyItem * lineThickness READ lineThickness CONSTANT)

    Q_PROPERTY(bool areSettingsAvailable READ areSettingsAvailable NOTIFY areSettingsAvailableChanged)

public:
    explicit BendSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* bendDirection() const;
    PropertyItem* showHoldLine() const;
    bool isShowHoldLineAvailable() const;

    PropertyItem* bendType() const;
    PropertyItem* bendCurve() const;
    PropertyItem* lineThickness() const;

    bool areSettingsAvailable() const;

signals:
    void areSettingsAvailableChanged(bool areSettingsAvailable);
    void isShowHoldLineAvailableChanged(bool isAvailable);

private:
    void updateIsShowHoldLineAvailable();

    PropertyItem* m_bendDirection = nullptr;
    PropertyItem* m_showHoldLine = nullptr;
    bool m_isShowHoldLineAvailable = false;

    PropertyItem* m_bendType = nullptr;
    PropertyItem* m_bendCurve = nullptr;
    PropertyItem* m_lineThickness = nullptr;
};
}

#endif // MU_INSPECTOR_BENDSETTINGSMODEL_H
