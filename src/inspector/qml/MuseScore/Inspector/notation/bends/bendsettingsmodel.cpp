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

static const ElementTypeSet ELEMENTS_TYPES {
    ElementType::GUITAR_BEND,
    ElementType::GUITAR_BEND_SEGMENT,
    ElementType::GUITAR_BEND_HOLD,
    ElementType::GUITAR_BEND_HOLD_SEGMENT,
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

BendSettingsModel::BendSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, iocCtx, repository)
{
    setModelType(InspectorModelType::TYPE_BEND);
    setTitle(muse::qtrc("inspector", "Bend/dive"));
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

    const int totBendAmount = bend->totBendAmountIncludingPrecedingBends();
    const int localBendAmount = bend->bendAmountInQuarterTones();
    const int pitchDiff = bendAmountToCurvePitch(localBendAmount);

    int endPitch = bendAmountToCurvePitch(totBendAmount);
    int startPitch = endPitch - pitchDiff;

    if (isHold(item)) {
        m_bendCurve = {
            CurvePoint(0, endPitch, true),
            CurvePoint(CurvePoint::MAX_TIME, endPitch, {}, true)
        };

        emit bendCurveChanged();

        return;
    }

    if (!bend->isDive()) {
        endPitch = std::max(endPitch, 0);
    } else if (bend->bendType() == GuitarBendType::SCOOP) {
        std::swap(startPitch, endPitch);
    }

    const int startTime = bend->startTimeFactor() * CurvePoint::MAX_TIME;
    const int endTime = bend->endTimeFactor() * CurvePoint::MAX_TIME;
    const int targetTime = bend->targetTimeFactor().has_value() ? bend->targetTimeFactor().value() * CurvePoint::MAX_TIME
                           : endTime;

    QString startPointName = muse::qtrc("inspector", "Start point");
    QString midPointName = muse::qtrc("inspector", "Midpoint");
    QString endPointName = muse::qtrc("inspector", "End point");

    constexpr bool GENERATED = true;
    constexpr bool END_DASHED = true;
    constexpr bool LIMIT_MOVE_VERTICALLY_BY_NEAREST_POINTS = true;

    using MoveDirection = CurvePoint::MoveDirection;

    switch (bend->bendType()) {
    case GuitarBendType::PRE_BEND:
    case GuitarBendType::PRE_DIVE:
        m_bendCurve = { CurvePoint(0, 0, GENERATED),
                        CurvePoint(0, endPitch, GENERATED),
                        CurvePoint(endTime, endPitch, { MoveDirection::Vertical }, END_DASHED, endPointName) };
        break;
    case GuitarBendType::DIP:
        m_bendCurve = { CurvePoint(0, startPitch, GENERATED),
                        CurvePoint(startTime, startPitch, { MoveDirection::Horizontal }, END_DASHED, startPointName),
                        CurvePoint(targetTime, endPitch, { MoveDirection::Both }, !END_DASHED, midPointName,
                                   !LIMIT_MOVE_VERTICALLY_BY_NEAREST_POINTS),
                        CurvePoint(endTime, startPitch, { CurvePoint::MoveDirection::Horizontal }, !END_DASHED, startPointName),
                        CurvePoint(CurvePoint::MAX_TIME, 0, GENERATED, END_DASHED) };
        break;
    case GuitarBendType::SLIGHT_BEND:
        m_bendCurve = { CurvePoint(0, startPitch, GENERATED),
                        CurvePoint(startTime, startPitch, { MoveDirection::Horizontal }, END_DASHED, startPointName),
                        CurvePoint(endTime, endPitch, { MoveDirection::Horizontal }, !END_DASHED, endPointName),
                        CurvePoint(CurvePoint::MAX_TIME, endPitch, GENERATED, END_DASHED) };
        break;
    default:
        m_bendCurve = { CurvePoint(0, startPitch, GENERATED),
                        CurvePoint(startTime, startPitch, { MoveDirection::Horizontal }, END_DASHED, startPointName),
                        CurvePoint(endTime, endPitch, { MoveDirection::Both },
                                   !END_DASHED, endPointName, startPitch == 0 && !bend->isDive()),
                        CurvePoint(CurvePoint::MAX_TIME, endPitch, GENERATED, END_DASHED) };
        break;
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
    const CurvePoints points = curvePointsFromQVariant(newBendCurve);
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

    const bool hasSeparateTargetPitchPoint = bend->bendType() == GuitarBendType::DIP;
    const int endPointIdx = hasSeparateTargetPitchPoint ? 3 : 2;
    const int targetPitchPointIdx = hasSeparateTargetPitchPoint ? 2 : endPointIdx;

    if (endPointIdx >= points.size()) {
        return;
    }

    const CurvePoint& startTimePoint = points.at(1);
    const CurvePoint& targetPitchPoint = points.at(targetPitchPointIdx);
    const CurvePoint& endTimePoint = points.at(endPointIdx);

    beginCommand(muse::TranslatableString("undoableAction", "Edit bend curve"));

    if (targetPitchPoint.pitch != m_bendCurve.at(targetPitchPointIdx).pitch) {
        bend->changeBendAmount(curvePitchToBendAmount(targetPitchPoint.pitch), curvePitchToBendAmount(startTimePoint.pitch));
    }

    const float startTimeFactor = static_cast<float>(startTimePoint.time) / CurvePoint::MAX_TIME;
    const float endTimeFactor = static_cast<float>(endTimePoint.time) / CurvePoint::MAX_TIME;
    bend->undoChangeProperty(Pid::BEND_START_TIME_FACTOR, startTimeFactor);
    bend->undoChangeProperty(Pid::BEND_END_TIME_FACTOR, endTimeFactor);

    if (hasSeparateTargetPitchPoint) {
        const float targetTimeFactor = static_cast<float>(targetPitchPoint.time) / CurvePoint::MAX_TIME;
        bend->undoChangeProperty(Pid::BEND_TARGET_TIME_FACTOR, targetTimeFactor);
    }

    endCommand();
    updateNotation();

    m_bendCurve = points;
    emit bendCurveChanged();
}

bool BendSettingsModel::isBendCurveEnabled() const
{
    return item() != nullptr;
}
