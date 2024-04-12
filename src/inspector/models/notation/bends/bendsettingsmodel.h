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
#ifndef MU_INSPECTOR_BENDSETTINGSMODEL_H
#define MU_INSPECTOR_BENDSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include "types/bendtypes.h"

namespace mu::inspector {
class BendSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bendDirection READ bendDirection CONSTANT)
    Q_PROPERTY(PropertyItem * showHoldLine READ showHoldLine CONSTANT)
    Q_PROPERTY(bool isShowHoldLineAvailable READ isShowHoldLineAvailable NOTIFY isShowHoldLineAvailableChanged)

    Q_PROPERTY(bool isBendCurveEnabled READ isBendCurveEnabled NOTIFY isBendCurveEnabledChanged)
    Q_PROPERTY(QVariantList bendCurve READ bendCurve WRITE setBendCurve NOTIFY bendCurveChanged)

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

    QVariantList bendCurve() const;

    bool areSettingsAvailable() const;

    void setBendCurve(const QVariantList& newBendCurve);

    bool isBendCurveEnabled() const;

signals:
    void areSettingsAvailableChanged(bool areSettingsAvailable);
    void isShowHoldLineAvailableChanged(bool isAvailable);

    void isBendCurveEnabledChanged();
    void bendCurveChanged();

private:
    void updateIsShowHoldLineAvailable();

    void loadBendCurve();

    mu::engraving::EngravingItem* item() const;

    bool isHold(const mu::engraving::EngravingItem* item) const;
    mu::engraving::GuitarBend* guitarBend(mu::engraving::EngravingItem* item) const;

    PropertyItem* m_bendDirection = nullptr;
    PropertyItem* m_showHoldLine = nullptr;
    bool m_isShowHoldLineAvailable = false;

    CurvePoints m_bendCurve;
    bool m_releaseBend = false;
};
}

#endif // MU_INSPECTOR_BENDSETTINGSMODEL_H
