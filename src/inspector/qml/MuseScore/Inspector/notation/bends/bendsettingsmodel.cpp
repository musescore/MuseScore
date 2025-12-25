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
#include "bendsettingsmodel.h"

#include "translation.h"
#include "types/bendtypes.h"

#include "engraving/dom/bend.h"
#include "engraving/dom/guitarbend.h"

using namespace mu::engraving;
using namespace mu::inspector;

static constexpr int START_POINT_INDEX = 1;
static constexpr int END_POINT_INDEX = 2;

static const std::set<ElementType> ELEMENTS_TYPES = {
    ElementType::GUITAR_BEND,
    ElementType::GUITAR_BEND_SEGMENT,
    ElementType::GUITAR_BEND_HOLD,
    ElementType::GUITAR_BEND_HOLD_SEGMENT
};

static int bendAmountToCurvePitch(int amount)
{
    int fulls = amount / 4;
    int quarts = amount % 4;
    return fulls * 100 + quarts * 25;
}

static int curvePitchToBendAmount(int pitch)
{
    int fulls = pitch / 100;
    int quarts = (pitch % 100) / 25;
    return fulls * 4 + quarts;
}

BendSettingsModel::BendSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BEND);
    setTitle(muse::qtrc("inspector", "Bend / Dive"));
    setIcon(muse::ui::IconCode::Code::GUITAR_BEND);

    createProperties();
}

void BendSettingsModel::createProperties()
{
    m_bendDirection = buildPropertyItem(mu::engraving::Pid::DIRECTION);
    m_showHoldLine = buildPropertyItem(mu::engraving::Pid::BEND_SHOW_HOLD_LINE);
    m_diveTabPos = buildPropertyItem(mu::engraving::Pid::GUITAR_DIVE_TAB_POS);
    m_dipVibratoType = buildPropertyItem(mu::engraving::Pid::VIBRATO_LINE_TYPE);
    m_lineStyle = buildPropertyItem(mu::engraving::Pid::LINE_STYLE);

    loadBendCurve();
}

void BendSettingsModel::requestElements()
{
    m_elementList.clear();

    for (ElementType type : ELEMENTS_TYPES) {
        m_elementList << m_repository->findElementsByType(type);
    }

    emit areSettingsAvailableChanged(areSettingsAvailable());
    emit isBendCurveEnabledChanged();
}

void BendSettingsModel::loadProperties()
{
    loadPropertyItem(m_bendDirection);
    loadPropertyItem(m_showHoldLine);
    loadPropertyItem(m_diveTabPos);
    loadPropertyItem(m_dipVibratoType);
    loadPropertyItem(m_lineStyle);

    updateIsHoldLine();
    updateIsShowHoldLineAvailable();
    updateIsDiveTabPosAvailable();
    updateIsTabStaff();
    updateIsDive();
    updateIsDip();

    loadBendCurve();
}

void BendSettingsModel::resetProperties()
{
    m_bendDirection->resetToDefault();
    m_showHoldLine->resetToDefault();
    m_diveTabPos->resetToDefault();
    m_dipVibratoType->resetToDefault();
}

bool BendSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // Bend inspector doesn't support multiple selection
}

void BendSettingsModel::updateIsShowHoldLineAvailable()
{
    bool available = true;

    if (m_isHoldLine) {
        available = false;
    } else {
        for (EngravingItem* item : m_elementList) {
            if (!item->isGuitarBendSegment()) {
                continue;
            }

            GuitarBendSegment* seg = toGuitarBendSegment(item);
            bool isAvail = seg->staffType() && seg->staffType()->isTabStaff() && seg->guitarBend()->bendType() != GuitarBendType::DIP;
            if (!isAvail) {
                available = false;
                break;
            }
        }
    }

    if (m_isShowHoldLineAvailable != available) {
        m_isShowHoldLineAvailable = available;
        emit isShowHoldLineAvailableChanged(m_isShowHoldLineAvailable);
    }
}

void BendSettingsModel::updateIsDiveTabPosAvailable()
{
    bool available = true;

    if (m_isHoldLine) {
        available = false;
    } else {
        for (EngravingItem* item : m_elementList) {
            if (!item->isGuitarBendSegment()) {
                continue;
            }

            GuitarBendSegment* seg = toGuitarBendSegment(item);
            GuitarBendType bendType = seg->guitarBend()->bendType();
            const StaffType* staffType = seg->staffType();
            bool isDiveOnTab = (bendType == GuitarBendType::DIVE || bendType == GuitarBendType::PRE_DIVE)
                               && staffType && staffType->isTabStaff();
            if (!isDiveOnTab) {
                available = false;
                break;
            }
        }
    }

    if (m_isDiveTabPosAvailable != available) {
        m_isDiveTabPosAvailable = available;
        emit isDiveTabPosAvailableChanged(m_isDiveTabPosAvailable);
    }
}

void BendSettingsModel::updateIsTabStaff()
{
    bool isTabStaff = true;
    for (EngravingItem* item : m_elementList) {
        const StaffType* staffType = item->staffType();
        if (staffType && !staffType->isTabStaff()) {
            isTabStaff = false;
            break;
        }
    }

    if (m_isTabStaff != isTabStaff) {
        m_isTabStaff = isTabStaff;
        emit isTabStaffChanged(m_isTabStaff);
    }
}

void BendSettingsModel::updateIsDive()
{
    bool isDive = true;
    for (EngravingItem* item : m_elementList) {
        if (!(item->isGuitarBendSegment() && toGuitarBendSegment(item)->guitarBend()->isDive())) {
            isDive = false;
            break;
        }
    }

    if (m_isDive != isDive) {
        m_isDive = isDive;
        emit isDiveChanged(m_isDive);
    }
}

void BendSettingsModel::updateIsDip()
{
    bool isDip = true;
    for (EngravingItem* item : m_elementList) {
        bool dip = item->isGuitarBendSegment() && toGuitarBendSegment(item)->guitarBend()->bendType() == GuitarBendType::DIP;
        if (!dip) {
            isDip = false;
            break;
        }
    }

    if (m_isDip != isDip) {
        m_isDip = isDip;
        emit isDipChanged(m_isDip);
    }
}

void BendSettingsModel::updateIsHoldLine()
{
    bool isHoldLine = true;
    for (EngravingItem* item : m_elementList) {
        if (!item->isGuitarBendHoldSegment()
            || toGuitarBendHoldSegment(item)->guitarBendHold()->guitarBend()->bendType() == GuitarBendType::DIP) {
            isHoldLine = false;
            break;
        }
    }

    if (m_isHoldLine != isHoldLine) {
        m_isHoldLine = isHoldLine;
        emit isHoldLineChanged(m_isHoldLine);
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

    int totBendAmount = bend->totBendAmountIncludingPrecedingBends();
    int endPitch = bendAmountToCurvePitch(totBendAmount);

    int localBendAmount = bend->bendAmountInQuarterTones();
    int pitchDiff = bendAmountToCurvePitch(localBendAmount);

    int startPitch = endPitch - pitchDiff;

    int starTime = bend->startTimeFactor() * CurvePoint::MAX_TIME;
    int endTime = bend->endTimeFactor() * CurvePoint::MAX_TIME;

    bool isHold = this->isHold(item);
    if (isHold) {
        m_bendCurve = {
            CurvePoint(0, endPitch, true),
            CurvePoint(CurvePoint::MAX_TIME, endPitch, {}, true)
        };

        emit bendCurveChanged();

        return;
    }

    bool isSlightBend = bend->bendType() == GuitarBendType::SLIGHT_BEND;

    QString startPointName = muse::qtrc("inspector", "Start point");
    QString endPointName = muse::qtrc("inspector", "End point");

    if (bend->bendType() == GuitarBendType::PRE_BEND || bend->bendType() == GuitarBendType::PRE_DIVE) {
        m_bendCurve = { CurvePoint(0, 0, true),
                        CurvePoint(0, endPitch, true),
                        CurvePoint(endTime, endPitch, { CurvePoint::MoveDirection::Vertical }, true, endPointName) };
    } else {
        m_bendCurve = { CurvePoint(0, startPitch, true),
                        CurvePoint(starTime, startPitch, { CurvePoint::MoveDirection::Horizontal }, true, startPointName),
                        CurvePoint(endTime, endPitch,
                                   { isSlightBend ? CurvePoint::MoveDirection::Horizontal : CurvePoint::MoveDirection::Both },
                                   false, endPointName, startPitch == 0 && !bend->isDive()),
                        CurvePoint(CurvePoint::MAX_TIME, endPitch, true, true) };
    }

    emit bendCurveChanged();
}

EngravingItem* BendSettingsModel::item() const
{
    for (EngravingItem* item : m_elementList) {
        if (muse::contains(ELEMENTS_TYPES, item->type())) {
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

PropertyItem* BendSettingsModel::diveTabPos() const
{
    return m_diveTabPos;
}

PropertyItem* BendSettingsModel::dipVibratoType() const
{
    return m_dipVibratoType;
}

PropertyItem* BendSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

bool BendSettingsModel::isShowHoldLineAvailable() const
{
    return m_isShowHoldLineAvailable;
}

bool BendSettingsModel::isDiveTabPosAvailable() const
{
    return m_isDiveTabPosAvailable;
}

bool BendSettingsModel::isTabStaff() const
{
    return m_isTabStaff;
}

bool BendSettingsModel::isDive() const
{
    return m_isDive;
}

bool BendSettingsModel::isDip() const
{
    return m_isDip;
}

bool BendSettingsModel::isHoldLine() const
{
    return m_isHoldLine;
}

void BendSettingsModel::setBendCurve(const QVariantList& newBendCurve)
{
    CurvePoints points = curvePointsFromQVariant(newBendCurve);

    if (m_bendCurve == points) {
        return;
    }

    if (END_POINT_INDEX >= points.size()) {
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

    const CurvePoint& endTimePoint = points.at(END_POINT_INDEX);

    bool pitchChanged = endTimePoint.pitch != m_bendCurve.at(END_POINT_INDEX).pitch;

    beginCommand(muse::TranslatableString("undoableAction", "Edit bend curve"));

    if (pitchChanged) {
        bend->changeBendAmount(curvePitchToBendAmount(endTimePoint.pitch));
    }

    float starTimeFactor = static_cast<float>(points.at(START_POINT_INDEX).time) / CurvePoint::MAX_TIME;
    float endTimeFactor = static_cast<float>(endTimePoint.time) / CurvePoint::MAX_TIME;
    bend->undoChangeProperty(Pid::BEND_START_TIME_FACTOR, starTimeFactor);
    bend->undoChangeProperty(Pid::BEND_END_TIME_FACTOR, endTimeFactor);

    endCommand();

    updateNotation();

    m_bendCurve = points;
    emit bendCurveChanged();
}

bool BendSettingsModel::isBendCurveEnabled() const
{
    return item() != nullptr;
}
