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

#include <qqmlintegration.h>

#include "abstractinspectormodel.h"

#include "types/bendtypes.h"

namespace mu::inspector {
class BendSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * bendDirection READ bendDirection CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * showHoldLine READ showHoldLine CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * diveTabPos READ diveTabPos CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * dipVibratoType READ dipVibratoType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * lineStyle READ lineStyle CONSTANT)

    Q_PROPERTY(bool isShowHoldLineAvailable READ isShowHoldLineAvailable NOTIFY isShowHoldLineAvailableChanged)
    Q_PROPERTY(bool isDiveTabPosAvailable READ isDiveTabPosAvailable NOTIFY isDiveTabPosAvailableChanged)
    Q_PROPERTY(bool isTabStaff READ isTabStaff NOTIFY isTabStaffChanged)
    Q_PROPERTY(bool isDive READ isDive NOTIFY isDiveChanged)
    Q_PROPERTY(bool isDip READ isDip NOTIFY isDipChanged)
    Q_PROPERTY(bool isHoldLine READ isHoldLine NOTIFY isHoldLineChanged)

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
    PropertyItem* diveTabPos() const;
    PropertyItem* dipVibratoType() const;
    PropertyItem* lineStyle() const;

    bool isShowHoldLineAvailable() const;
    bool isDiveTabPosAvailable() const;
    bool isTabStaff() const;
    bool isDive() const;
    bool isDip() const;
    bool isHoldLine() const;

    QVariantList bendCurve() const;

    bool areSettingsAvailable() const;

    void setBendCurve(const QVariantList& newBendCurve);

    bool isBendCurveEnabled() const;

signals:
    void areSettingsAvailableChanged(bool areSettingsAvailable);
    void isShowHoldLineAvailableChanged(bool isAvailable);
    void isDiveTabPosAvailableChanged(bool diveTabPosAvailable);
    void isTabStaffChanged(bool isTabStaff);
    void isDiveChanged(bool isDive);
    void isDipChanged(bool isDip);
    void isHoldLineChanged(bool isHoldLine);

    void isBendCurveEnabledChanged();
    void bendCurveChanged();

private:
    void updateIsShowHoldLineAvailable();
    void updateIsDiveTabPosAvailable();
    void updateIsTabStaff();
    void updateIsDive();
    void updateIsDip();
    void updateIsHoldLine();

    void loadBendCurve();

    mu::engraving::EngravingItem* item() const;

    bool isHold(const mu::engraving::EngravingItem* item) const;
    mu::engraving::GuitarBend* guitarBend(mu::engraving::EngravingItem* item) const;

    PropertyItem* m_bendDirection = nullptr;
    PropertyItem* m_showHoldLine = nullptr;
    PropertyItem* m_diveTabPos = nullptr;
    PropertyItem* m_dipVibratoType = nullptr;
    PropertyItem* m_lineStyle = nullptr;

    bool m_isShowHoldLineAvailable = false;
    bool m_isDiveTabPosAvailable = false;
    bool m_isTabStaff = false;
    bool m_isDive = false;
    bool m_isDip = false;
    bool m_isHoldLine = false;

    CurvePoints m_bendCurve;
};
}
