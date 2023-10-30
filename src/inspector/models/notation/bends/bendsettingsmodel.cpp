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
#include "bendsettingsmodel.h"
#include "dom/guitarbend.h"

#include "translation.h"
#include "dataformatter.h"
#include "types/bendtypes.h"

#include "dom/bend.h"

using namespace mu::inspector;

static constexpr int MAX_TIME = 60;

static QList<mu::engraving::ElementType> ELEMENTS_TYPES = {
    engraving::ElementType::GUITAR_BEND,
    engraving::ElementType::GUITAR_BEND_SEGMENT,
    engraving::ElementType::GUITAR_BEND_HOLD,
    engraving::ElementType::GUITAR_BEND_HOLD_SEGMENT
};

BendSettingsModel::BendSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BEND);
    setTitle(qtrc("inspector", "Bend"));
    setIcon(ui::IconCode::Code::GUITAR_BEND);

    createProperties();
}

void BendSettingsModel::createProperties()
{
    m_lineThickness = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);

    m_bendDirection = buildPropertyItem(mu::engraving::Pid::DIRECTION);
    m_showHoldLine = buildPropertyItem(mu::engraving::Pid::BEND_SHOW_HOLD_LINE);

    loadBendCurve();
}

void BendSettingsModel::requestElements()
{
    m_elementList.clear();

    for (engraving::ElementType type : ELEMENTS_TYPES) {
        m_elementList << m_repository->findElementsByType(type);
    }

    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void BendSettingsModel::loadProperties()
{
    loadPropertyItem(m_lineThickness, formatDoubleFunc);
    loadPropertyItem(m_bendDirection);
    loadPropertyItem(m_showHoldLine);

    updateIsShowHoldLineAvailable();

    loadBendCurve();
}

void BendSettingsModel::resetProperties()
{
    m_lineThickness->resetToDefault();
    m_bendDirection->resetToDefault();
    m_showHoldLine->resetToDefault();
}

PropertyItem* BendSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

bool BendSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // Bend inspector doesn't support multiple selection
}

void BendSettingsModel::updateIsShowHoldLineAvailable()
{
    bool available = true;
    for (EngravingItem* item : m_elementList) {
        if (!item->isGuitarBendSegment()) {
            continue;
        }

        GuitarBendSegment* seg = toGuitarBendSegment(item);
        if (seg->staffType() && !seg->staffType()->isTabStaff()) {
            available = false;
            break;
        }
    }

    if (m_isShowHoldLineAvailable != available) {
        m_isShowHoldLineAvailable = available;
        emit isShowHoldLineAvailableChanged(m_isShowHoldLineAvailable);
    }
}

void BendSettingsModel::loadBendCurve()
{
    EngravingItem* item = this->item();
    if (!item) {
        return;
    }

    GuitarBend* bend = guitarBend(item);
    if (!bend) {
        return;
    }

    int bendAmount = bend->bendAmountInQuarterTones();

    int fulls = bendAmount / 4;
    int quarts = bendAmount % 4;
    int pitch = std::abs(fulls * 100 + quarts * 25);

    bool isHold = this->isHold(item);
    if (isHold) {
        m_bendCurve = {
            CurvePoint(0, pitch, true),
            CurvePoint(MAX_TIME, pitch, {}, true)
        };

        emit bendCurveChanged();

        return;
    }

    if (bend->type() == engraving::GuitarBendType::PRE_BEND) {
        m_bendCurve = { CurvePoint(0, 0, true),
                        CurvePoint(0, pitch, true),
                        CurvePoint(MAX_TIME, pitch, { CurvePoint::MoveDirection::Vertical }, true) };
    } else if (bend->isReleaseBend()) {
        m_bendCurve = { CurvePoint(0, pitch, true),
                        CurvePoint(0, pitch, { CurvePoint::MoveDirection::Horizontal }, true),
                        CurvePoint(15, 0, { CurvePoint::MoveDirection::Both }),
                        CurvePoint(MAX_TIME, 0, {}, true, true) };
    } else {
        m_bendCurve = { CurvePoint(0, 0, true),
                        CurvePoint(0, 0, { CurvePoint::MoveDirection::Horizontal }, true),
                        CurvePoint(15, pitch, { CurvePoint::MoveDirection::Both }),
                        CurvePoint(MAX_TIME, pitch, {}, true, true) };
    }

    emit bendCurveChanged();
}

EngravingItem* BendSettingsModel::item() const
{
    for (EngravingItem* item : m_elementList) {
        if (ELEMENTS_TYPES.contains(item->type())) {
            return item;
        }
    }

    return nullptr;
}

bool BendSettingsModel::isHold(const EngravingItem* item) const
{
    if (!item) {
        return false;
    }

    return item->isGuitarBendHold() || item->isGuitarBendHoldSegment();
}

GuitarBend* BendSettingsModel::guitarBend(EngravingItem* item) const
{
    if (item->isGuitarBend()) {
        return toGuitarBend(item);
    } else if (item->isGuitarBendHold()) {
        return toGuitarBendHold(item)->guitarBend();
    } else if (item->isGuitarBendHoldSegment()) {
        return toGuitarBendHoldSegment(item)->guitarBendHold()->guitarBend();
    } else if (item->isGuitarBendSegment()) {
        return toGuitarBendSegment(item)->guitarBend();
    }

    return nullptr;
}

QVariantList BendSettingsModel::bendCurve() const
{
    QVariantList result;
    for (const CurvePoint& point : m_bendCurve) {
        result << point.toMap();
    }

    return result;
}

PropertyItem* BendSettingsModel::bendDirection() const
{
    return m_bendDirection;
}

PropertyItem* BendSettingsModel::showHoldLine() const
{
    return m_showHoldLine;
}

bool BendSettingsModel::isShowHoldLineAvailable() const
{
    return m_isShowHoldLineAvailable;
}

void BendSettingsModel::setBendCurve(const QVariantList& newBendCurve)
{
    CurvePoints points = curvePointsFromQVariant(newBendCurve);

    if (m_bendCurve == points) {
        return;
    }

    EngravingItem* item = this->item();
    if (!item) {
        return;
    }

    GuitarBend* bend = guitarBend(item);
    if (!bend) {
        return;
    }

    int newPitch = points[2].pitch;
    int fulls = newPitch / 100;
    int quarts = (newPitch % 100) / 25;
    int bendAmount = fulls * 4 + quarts;

    int pitch = 0;
    if (bend->isReleaseBend()) {
        pitch = bend->startNote()->pitch() - bendAmount / 2;
    } else {
        pitch = bendAmount / 2 + bend->startNote()->pitch();
    }

    beginCommand();
    bend->setEndNotePitch(pitch);
    // todo set time
    endCommand();

    updateNotation();

    m_bendCurve = points;
    emit bendCurveChanged();
}
