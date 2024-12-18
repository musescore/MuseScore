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

#include "dom/bend.h"
#include "dom/guitarbend.h"

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
    setTitle(muse::qtrc("inspector", "Bend"));
    setIcon(muse::ui::IconCode::Code::GUITAR_BEND);

    createProperties();
}

void BendSettingsModel::createProperties()
{
    m_bendDirection = buildPropertyItem(mu::engraving::Pid::DIRECTION);
    m_showHoldLine = buildPropertyItem(mu::engraving::Pid::BEND_SHOW_HOLD_LINE);

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

    updateIsShowHoldLineAvailable();

    loadBendCurve();
}

void BendSettingsModel::resetProperties()
{
    m_bendDirection->resetToDefault();
    m_showHoldLine->resetToDefault();
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

    m_releaseBend = bend->isReleaseBend();
    bool isSlightBend = bend->type() == GuitarBendType::SLIGHT_BEND;

    QString startPointName = muse::qtrc("inspector", "Start point");
    QString endPointName = muse::qtrc("inspector", "End point");

    if (bend->type() == GuitarBendType::PRE_BEND) {
        m_bendCurve = { CurvePoint(0, 0, true),
                        CurvePoint(0, endPitch, true),
                        CurvePoint(endTime, endPitch, { CurvePoint::MoveDirection::Vertical }, true, endPointName) };
    } else if (m_releaseBend) {
        m_bendCurve = { CurvePoint(0, startPitch - endPitch, true),
                        CurvePoint(starTime, startPitch - endPitch, { CurvePoint::MoveDirection::Horizontal }, true, startPointName),
                        CurvePoint(endTime, 0, { CurvePoint::MoveDirection::Both }, false, endPointName, false),
                        CurvePoint(CurvePoint::MAX_TIME, 0, true, true) };
    } else {
        m_bendCurve = { CurvePoint(0, startPitch, true),
                        CurvePoint(starTime, startPitch, { CurvePoint::MoveDirection::Horizontal }, true, startPointName),
                        CurvePoint(endTime, endPitch,
                                   { isSlightBend ? CurvePoint::MoveDirection::Horizontal : CurvePoint::MoveDirection::Both },
                                   false, endPointName, startPitch == 0),
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
        int bendAmount = curvePitchToBendAmount(endTimePoint.pitch);
        int pitch = bendAmount / 2 + bend->startNoteOfChain()->pitch();
        QuarterOffset quarterOff = bendAmount % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
        if (pitch == bend->startNote()->pitch() && quarterOff == QuarterOffset::QUARTER_SHARP) {
            // Because a flat second is more readable than a sharp unison
            pitch += 1;
            quarterOff = QuarterOffset::QUARTER_FLAT;
        }

        if (!m_releaseBend) {
            bend->setEndNotePitch(pitch, quarterOff);
        } else {
            int oldBendAmount = curvePitchToBendAmount(m_bendCurve[START_POINT_INDEX].pitch);
            pitch = bend->startNote()->pitch() - ((oldBendAmount - bendAmount) / 2);

            bend->setEndNotePitch(pitch, quarterOff);
        }
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
