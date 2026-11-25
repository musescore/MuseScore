/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "notationautomationaccessibility.h"

#include <QStringList>

#include "translation.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"

#include "notationautomationdisplaymapping.h"

using namespace mu::notation;
using namespace mu::engraving;

QString NotationAutomationAccessibility::laneTitle(AutomationLane lane)
{
    switch (lane) {
    case AutomationLane::Dynamics: return muse::qtrc("notation/automation", "Dynamics");
    case AutomationLane::Tempo: return muse::qtrc("notation/automation", "Tempo");
    case AutomationLane::Volume: return muse::qtrc("notation/automation", "Volume");
    case AutomationLane::Pan: return muse::qtrc("notation/automation", "Pan");
    case AutomationLane::MasterVolume: return muse::qtrc("notation/automation", "Master volume");
    case AutomationLane::MasterPan: return muse::qtrc("notation/automation", "Master pan");
    }

    return QString();
}

QString NotationAutomationAccessibility::scopeText(const PointInfo& info)
{
    if (info.curveKey.isGlobal() || !info.staff) {
        return QString();
    }

    QString partName;
    if (const Part* part = info.staff->part()) {
        partName = part->longName(Fraction::fromTicks(info.tick)).toQString();
        if (partName.isEmpty()) {
            partName = part->partName().toQString();
        }
    }

    if (info.curveKey.trackId()) {
        return partName;
    }

    const QString staff = muse::qtrc("engraving", "Staff %1").arg(QString::number(info.staff->idx() + 1));
    return partName.isEmpty() ? staff : QString("%1 (%2)").arg(staff, partName);
}

QString NotationAutomationAccessibility::barsAndBeatsText(const Score* score, int tick)
{
    const TimeSigMap* sigmap = score->sigmap();

    int bar = 0;
    int beat = 0;
    int ticks = 0;
    sigmap->tickValues(tick, &bar, &beat, &ticks);

    const int ticksPerBeat = ticks_beat(sigmap->timesig(tick).timesig().denominator());
    const double beatNumber = beat + 1 + ticks / static_cast<double>(ticksPerBeat);

    const Measure* measure = score->tick2measure(Fraction::fromTicks(tick));
    const int displayedBar = measure ? measure->measureNumber() + 1 : bar + 1;

    QStringList parts;
    parts << muse::qtrc("notation/automation", "Measure %1").arg(bar + 1);
    if (displayedBar != bar + 1) {
        parts << muse::qtrc("notation/automation", "Displayed measure %1").arg(displayedBar);
    }
    parts << muse::qtrc("notation/automation", "Beat %1").arg(beatNumber, 0, 'g', 3);

    return parts.join(", ");
}

QString NotationAutomationAccessibility::pointDescription(const PointInfo& info, bool includeScope)
{
    QStringList parts;

    if (includeScope) {
        const QString scope = scopeText(info);
        if (!scope.isEmpty()) {
            parts << scope;
        }
    }

    parts << muse::qtrc("notation/automation", "%1 node").arg(laneTitle(info.lane));

    if (info.value) {
        const QString value = NotationAutomationDisplayMapping::valueText(automationTypeOf(info.lane), *info.value);
        switch (info.pointType) {
        case PointData::PointType::IN:
            parts << muse::qtrc("notation/automation", "Incoming %1").arg(value);
            break;
        case PointData::PointType::OUT:
            parts << muse::qtrc("notation/automation", "Outgoing %1").arg(value);
            break;
        case PointData::PointType::BOTH:
        case PointData::PointType::UNKNOWN:
            parts << value;
            break;
        }
    }

    if (info.staff && info.staff->score()) {
        parts << barsAndBeatsText(info.staff->score(), info.tick);
    }

    return parts.join(", ");
}

QString NotationAutomationAccessibility::noTargetDescription(MoveDirection direction, bool hadSelection)
{
    if (!hadSelection) {
        return muse::qtrc("notation/automation", "No nodes");
    }

    switch (direction) {
    case MoveDirection::Left: return muse::qtrc("notation/automation", "No previous node");
    case MoveDirection::Right: return muse::qtrc("notation/automation", "No next node");
    case MoveDirection::Up: return muse::qtrc("notation/automation", "No node above");
    case MoveDirection::Down: return muse::qtrc("notation/automation", "No node below");
    case MoveDirection::Undefined: break;
    }

    return QString();
}
