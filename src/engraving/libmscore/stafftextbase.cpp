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

#include "stafftextbase.h"
#include "rw/xml.h"
#include "types/typesconv.h"

#include "system.h"
#include "staff.h"
#include "score.h"
#include "measure.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   StaffTextBase
//---------------------------------------------------------

StaffTextBase::StaffTextBase(const ElementType& type, Segment* parent, TextStyleType tid, ElementFlags flags)
    : TextBase(type, parent, tid, flags)
{
    setSwingParameters(Constant::division / 2, 60);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTextBase::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.startObject(this);

    for (const ChannelActions& s : _channelActions) {
        int channel = s.channel;
        for (const QString& name : qAsConst(s.midiActionNames)) {
            xml.tagE(QString("MidiAction channel=\"%1\" name=\"%2\"").arg(channel).arg(name));
        }
    }
    for (int voice = 0; voice < VOICES; ++voice) {
        if (!_channelNames[voice].isEmpty()) {
            xml.tagE(QString("channelSwitch voice=\"%1\" name=\"%2\"").arg(voice).arg(_channelNames[voice]));
        }
    }
    if (_setAeolusStops) {
        for (int i = 0; i < 4; ++i) {
            xml.tag(QString("aeolus group=\"%1\"").arg(i), aeolusStops[i]);
        }
    }
    if (swing()) {
        DurationType swingUnit;
        if (swingParameters()->swingUnit == Constant::division / 2) {
            swingUnit = DurationType::V_EIGHTH;
        } else if (swingParameters()->swingUnit == Constant::division / 4) {
            swingUnit = DurationType::V_16TH;
        } else {
            swingUnit = DurationType::V_ZERO;
        }
        int swingRatio = swingParameters()->swingRatio;
        xml.tagE(QString("swing unit=\"%1\" ratio= \"%2\"").arg(TConv::toXml(swingUnit)).arg(swingRatio));
    }
    if (capo() != 0) {
        xml.tagE(QString("capo fretId=\"%1\"").arg(capo()));
    }
    TextBase::writeProperties(xml);

    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTextBase::read(XmlReader& e)
{
    for (int voice = 0; voice < VOICES; ++voice) {
        _channelNames[voice].clear();
    }
    clearAeolusStops();
    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool StaffTextBase::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());

    if (tag == "MidiAction") {
        int channel = e.intAttribute("channel", 0);
        QString name = e.attribute("name");
        bool found = false;
        int n = _channelActions.size();
        for (int i = 0; i < n; ++i) {
            ChannelActions* a = &_channelActions[i];
            if (a->channel == channel) {
                a->midiActionNames.append(name);
                found = true;
                break;
            }
        }
        if (!found) {
            ChannelActions a;
            a.channel = channel;
            a.midiActionNames.append(name);
            _channelActions.append(a);
        }
        e.readNext();
    } else if (tag == "channelSwitch" || tag == "articulationChange") {
        int voice = e.intAttribute("voice", -1);
        if (voice >= 0 && voice < VOICES) {
            _channelNames[voice] = e.attribute("name");
        } else if (voice == -1) {
            // no voice applies channel to all voices for
            // compatibility
            for (int i = 0; i < VOICES; ++i) {
                _channelNames[i] = e.attribute("name");
            }
        }
        e.readNext();
    } else if (tag == "aeolus") {
        int group = e.intAttribute("group", -1);
        if (group >= 0 && group < 4) {
            aeolusStops[group] = e.readInt();
        } else {
            e.readNext();
        }
        _setAeolusStops = true;
    } else if (tag == "swing") {
        DurationType swingUnit = TConv::fromXml(e.attribute("unit", ""), DurationType::V_INVALID);
        int unit = 0;
        if (swingUnit == DurationType::V_EIGHTH) {
            unit = Constant::division / 2;
        } else if (swingUnit == DurationType::V_16TH) {
            unit = Constant::division / 4;
        } else if (swingUnit == DurationType::V_ZERO) {
            unit = 0;
        }
        int ratio = e.intAttribute("ratio", 60);
        setSwing(true);
        setSwingParameters(unit, ratio);
        e.readNext();
    } else if (tag == "capo") {
        int fretId = e.intAttribute("fretId", 0);
        setCapo(fretId);
        e.readNext();
    } else if (!TextBase::readProperties(e)) {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   clearAeolusStops
//---------------------------------------------------------

void StaffTextBase::clearAeolusStops()
{
    for (int i = 0; i < 4; ++i) {
        aeolusStops[i] = 0;
    }
}

//---------------------------------------------------------
//   setAeolusStop
//---------------------------------------------------------

void StaffTextBase::setAeolusStop(int group, int idx, bool val)
{
    if (val) {
        aeolusStops[group] |= (1 << idx);
    } else {
        aeolusStops[group] &= ~(1 << idx);
    }
}

//---------------------------------------------------------
//   getAeolusStop
//---------------------------------------------------------

bool StaffTextBase::getAeolusStop(int group, int idx) const
{
    return aeolusStops[group] & (1 << idx);
}

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* StaffTextBase::segment() const
{
    if (!explicitParent()->isSegment()) {
        qDebug("parent %s", explicitParent()->typeName());
        return 0;
    }
    Segment* s = toSegment(explicitParent());
    return s;
}
}
