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

#include "keysig.h"

#include "types/typesconv.h"

#include "masterscore.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "part.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Segment* s)
    : EngravingItem(ElementType::KEYSIG, s, ElementFlag::ON_STAFF)
{
    m_showCourtesy = true;
    m_hideNaturals = false;
}

KeySig::KeySig(const KeySig& k)
    : EngravingItem(k)
{
    m_showCourtesy = k.m_showCourtesy;
    m_sig          = k.m_sig;
    m_hideNaturals = false;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double KeySig::mag() const
{
    return staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::KEYSIG;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* KeySig::drop(EditData& data)
{
    KeySig* ks = toKeySig(data.dropElement);
    if (ks->type() != ElementType::KEYSIG) {
        delete ks;
        return 0;
    }
    KeySigEvent k = ks->keySigEvent();
    delete ks;
    if (data.modifiers & ControlModifier) {
        // apply only to this stave
        if (!(k == keySigEvent())) {
            score()->undoChangeKeySig(staff(), tick(), k);
        }
    } else {
        // apply to all staves:
        for (Staff* s : score()->masterScore()->staves()) {
            score()->undoChangeKeySig(s, tick(), k);
        }
    }
    return this;
}

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void KeySig::setKey(Key cKey)
{
    KeySigEvent e;
    e.setConcertKey(cKey);
    if (staff() && !style().styleB(Sid::concertPitch)) {
        Interval v = staff()->part()->instrument(tick())->transpose();
        if (!v.isZero()) {
            v.flip();
            Key tKey = transposeKey(cKey, v, staff()->part()->preferSharpFlat());
            e.setKey(tKey);
        }
    }
    setKeySigEvent(e);
}

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void KeySig::setKey(Key cKey, Key tKey)
{
    KeySigEvent e;
    e.setConcertKey(cKey);
    e.setKey(tKey);
    setKeySigEvent(e);
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool KeySig::operator==(const KeySig& k) const
{
    return m_sig == k.m_sig;
}

//---------------------------------------------------------
//   isChange
//---------------------------------------------------------

bool KeySig::isChange() const
{
    if (!staff()) {
        return false;
    }
    if (!segment() || segment()->segmentType() != SegmentType::KeySig) {
        return false;
    }
    Fraction keyTick = tick();
    return staff()->currentKeyTick(keyTick) == keyTick;
}

//---------------------------------------------------------
//   changeKeySigEvent
//---------------------------------------------------------

void KeySig::changeKeySigEvent(const KeySigEvent& t)
{
    if (m_sig == t) {
        return;
    }
    setKeySigEvent(t);
}

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void KeySig::undoSetShowCourtesy(bool v)
{
    undoChangeProperty(Pid::SHOW_COURTESY, v);
}

//---------------------------------------------------------
//   undoSetMode
//---------------------------------------------------------

void KeySig::undoSetMode(KeyMode v)
{
    undoChangeProperty(Pid::KEYSIG_MODE, int(v));
}

PointF KeySig::staffOffset() const
{
    const Segment* seg = segment();
    const Measure* meas = seg ? seg->measure() : nullptr;
    if (meas && meas->endTick() == tick()) {
        // Courtesy key sig should be adjusted by the following staffType's offset
        return EngravingItem::staffOffset();
    }
    return PointF(0.0, 0.0);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue KeySig::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::KEY:
        return int(key());
    case Pid::KEY_CONCERT:
        return int(concertKey());
    case Pid::SHOW_COURTESY:
        return int(showCourtesy());
    case Pid::KEYSIG_MODE:
        return int(mode());
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool KeySig::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::KEY:
        if (generated()) {
            return false;
        }
        setKey(m_sig.concertKey(), Key(v.toInt()));
        break;
    case Pid::KEY_CONCERT:
        if (generated()) {
            return false;
        }
        setKey(Key(v.toInt()));
        break;
    case Pid::SHOW_COURTESY:
        if (generated()) {
            return false;
        }
        setShowCourtesy(v.toBool());
        break;
    case Pid::KEYSIG_MODE:
        if (generated()) {
            return false;
        }
        setMode(KeyMode(v.toInt()));
        staff()->setKey(tick(), keySigEvent());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayoutAll();
    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue KeySig::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::KEY:
        return int(Key::INVALID);
    case Pid::KEY_CONCERT:
        return int(Key::INVALID);
    case Pid::SHOW_COURTESY:
        return true;
    case Pid::KEYSIG_MODE:
        return int(KeyMode::UNKNOWN);
    default:
        return EngravingItem::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* KeySig::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* KeySig::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String KeySig::accessibleInfo() const
{
    String keySigType = TConv::translatedUserName(key(), isAtonal(), isCustom());
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), keySigType);
}

//---------------------------------------------------------
//   translatedSubtypeUserName
//---------------------------------------------------------

muse::TranslatableString KeySig::subtypeUserName() const
{
    return TConv::userName(key(), isAtonal(), isCustom());
}
}
