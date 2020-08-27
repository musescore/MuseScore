//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationaccessibility.h"

#include "translation.h"

#include "igetscore.h"

#include "libmscore/score.h"
#include "libmscore/spanner.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/sig.h"
#include "libmscore/measure.h"

using namespace mu::notation;
using namespace mu::async;
using namespace Ms;

NotationAccessibility::NotationAccessibility(const IGetScore* getScore, Notification selectionChangedNotification)
    : m_getScore(getScore)
{
    selectionChangedNotification.onNotify(this, [this]() {
        if (score()) {
            updateAccessibilityInfo();
        }
    });
}

const Score* NotationAccessibility::score() const
{
    return m_getScore->score();
}

const Selection* NotationAccessibility::selection() const
{
    return &score()->selection();
}

mu::ValCh<std::string> NotationAccessibility::accessibilityInfo() const
{
    return m_accessibilityInfo;
}

void NotationAccessibility::updateAccessibilityInfo()
{
    QString newAccessibilityInfo;

    if (selection()->isSingle()) {
        newAccessibilityInfo = singleElementAccessibilityInfo();
    } else if (selection()->isRange()) {
        newAccessibilityInfo = rangeAccessibilityInfo();
    } else if (selection()->isList()) {
        newAccessibilityInfo = qtrc("notation", "List selection");
    }

    setAccessibilityInfo(newAccessibilityInfo);
}

void NotationAccessibility::setAccessibilityInfo(const QString& info)
{
    std::string infoStd = info.toStdString();

    if (m_accessibilityInfo.val == infoStd) {
        return;
    }

    m_accessibilityInfo.set(infoStd);
}

QString NotationAccessibility::rangeAccessibilityInfo() const
{
    const Segment* endSegment = selection()->endSegment();

    if (!endSegment) {
        endSegment = score()->lastSegment();
    } else {
        endSegment = endSegment->prev1MM();
    }

    return qtrc("notation", "Range Selection %1 %2")
           .arg(formatStartBarsAndBeats(selection()->startSegment()))
           .arg(formatEndBarsAndBeats(endSegment));
}

QString NotationAccessibility::singleElementAccessibilityInfo() const
{
    const Element* element = selection()->element();
    if (!element) {
        return QString();
    }

    QString accessibilityInfo = element->accessibleInfo();
    QString barsAndBeats = formatSingleElementBarsAndBeats(element);

    if (!barsAndBeats.isEmpty()) {
        accessibilityInfo += "; " + barsAndBeats;
    }

    if (element->staffIdx() + 1) {
        QString staff = qtrc("notation", "Staff %1").arg(QString::number(element->staffIdx() + 1));

        QString staffName = element->staff()->part()->longName(element->tick());
        if (staffName.isEmpty()) {
            staffName = element->staff()->partName();
        }

        if (staffName.isEmpty()) {
            accessibilityInfo = QString("%1; %2").arg(accessibilityInfo).arg(staff);
        } else {
            accessibilityInfo = QString("%1; %2 (%3)").arg(accessibilityInfo).arg(staff).arg(staffName);
        }
    }

    return accessibilityInfo;
}

QString NotationAccessibility::formatSingleElementBarsAndBeats(const Element* element) const
{
    const Spanner* spanner = nullptr;
    if (element->isSpannerSegment()) {
        spanner = static_cast<const SpannerSegment*>(element)->spanner();
    }

    if (spanner) {
        const Segment* endSegment = spanner->endSegment();

        if (!endSegment) {
            endSegment = score()->lastSegment()->prev1MM(SegmentType::ChordRest);
        }

        if (endSegment->tick() != score()->lastSegment()->prev1MM(SegmentType::ChordRest)->tick()
            && spanner->type() != ElementType::SLUR
            && spanner->type() != ElementType::TIE) {
            endSegment = endSegment->prev1MM(SegmentType::ChordRest);
        }

        return formatStartBarsAndBeats(spanner->startSegment()) + " " + formatEndBarsAndBeats(endSegment);
    }

    QString result;
    std::pair<int, float> barbeat = this->barbeat(element);

    if (barbeat.first != 0) {
        result = qtrc("notation", "Measure: %1").arg(QString::number(barbeat.first));

        if (!qFuzzyIsNull(barbeat.second)) {
            result += qtrc("notation", "; Beat: %1").arg(QString::number(barbeat.second));
        }
    }

    return result;
}

QString NotationAccessibility::formatStartBarsAndBeats(const Element* element) const
{
    std::pair<int, float> barbeat = this->barbeat(element);

    return qtrc("notation", "Start Measure: %1; Start Beat: %2")
           .arg(QString::number(barbeat.first))
           .arg(QString::number(barbeat.second));
}

QString NotationAccessibility::formatEndBarsAndBeats(const Element* element) const
{
    std::pair<int, float> barbeat = this->barbeat(element);

    return qtrc("notation", "End Measure: %1; End Beat: %2")
           .arg(QString::number(barbeat.first))
           .arg(QString::number(barbeat.second));
}

std::pair<int, float> NotationAccessibility::barbeat(const Element* element) const
{
    if (!element) {
        return std::pair<int, float>(0, 0.0F);
    }

    const Element* parent = element;
    while (parent && parent->type() != ElementType::SEGMENT && parent->type() != ElementType::MEASURE) {
        parent = parent->parent();
    }

    if (!parent) {
        return std::pair<int, float>(0, 0.0F);
    }

    int bar = 0;
    int beat = 0;
    int ticks = 0;

    const TimeSigMap* timeSigMap = element->score()->sigmap();
    int ticksB = Ms::ticks_beat(timeSigMap->timesig(0).timesig().denominator());

    if (parent->type() == ElementType::SEGMENT) {
        const Segment* segment = static_cast<const Segment*>(parent);
        timeSigMap->tickValues(segment->tick().ticks(), &bar, &beat, &ticks);
        ticksB = Ms::ticks_beat(timeSigMap->timesig(segment->tick().ticks()).timesig().denominator());
    } else if (parent->type() == ElementType::MEASURE) {
        const Measure* measure = static_cast<const Measure*>(parent);
        bar = measure->no();
        beat = -1;
        ticks = 0;
    }

    return std::pair<int, float>(bar + 1, beat + 1 + ticks / static_cast<float>(ticksB));
}
