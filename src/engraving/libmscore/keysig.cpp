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

#include "keysig.h"

#include "draw/types/pen.h"

#include "types/symnames.h"
#include "types/typesconv.h"
#include "layout/tlayout.h"

#include "clef.h"
#include "masterscore.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"

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
    _showCourtesy = true;
    _hideNaturals = false;
}

KeySig::KeySig(const KeySig& k)
    : EngravingItem(k)
{
    _showCourtesy = k._showCourtesy;
    _sig          = k._sig;
    _hideNaturals = false;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double KeySig::mag() const
{
    return staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void KeySig::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    painter->setPen(curColor());
    double _spatium = spatium();
    double step = _spatium * (staff() ? staff()->staffTypeForElement(this)->lineDistance().val() * 0.5 : 0.5);
    int lines = staff() ? staff()->staffTypeForElement(this)->lines() : 5;
    double ledgerLineWidth = score()->styleMM(Sid::ledgerLineWidth) * mag();
    double ledgerExtraLen = score()->styleS(Sid::ledgerLineLength).val() * _spatium;
    for (const KeySym& ks: _sig.keySymbols()) {
        double x = ks.xPos * _spatium;
        double y = ks.line * step;
        drawSymbol(ks.sym, painter, PointF(x, y));
        // ledger lines
        double _symWidth = symWidth(ks.sym);
        double x1 = x - ledgerExtraLen;
        double x2 = x + _symWidth + ledgerExtraLen;
        painter->setPen(Pen(curColor(), ledgerLineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
        for (int i = -2; i >= ks.line; i -= 2) { // above
            y = i * step;
            painter->drawLine(LineF(x1, y, x2, y));
        }
        for (int i = lines * 2; i <= ks.line; i += 2) { // below
            y = i * step;
            painter->drawLine(LineF(x1, y, x2, y));
        }
    }
    if (!explicitParent() && (isAtonal() || isCustom()) && _sig.keySymbols().empty()) {
        // empty custom or atonal key signature - draw something for palette
        painter->setPen(engravingConfiguration()->formattingMarksColor());
        drawSymbol(SymId::timeSigX, painter, PointF(symWidth(SymId::timeSigX) * -0.5, 2.0 * spatium()));
    }
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

void KeySig::setKey(Key key)
{
    KeySigEvent e;
    e.setKey(key);
    setKeySigEvent(e);
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool KeySig::operator==(const KeySig& k) const
{
    return _sig == k._sig;
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
    if (_sig == t) {
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

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue KeySig::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::KEY:
        return int(key());
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
}
