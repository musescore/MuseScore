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
//   add
//---------------------------------------------------------

void KeySig::addLayout(SymId sym, int line)
{
    double _spatium = spatium();
    double step = _spatium * (staff() ? staff()->staffTypeForElement(this)->lineDistance().val() * 0.5 : 0.5);
    KeySym ks;
    ks.sym = sym;
    double x = 0.0;
    if (_sig.keySymbols().size() > 0) {
        KeySym& previous = _sig.keySymbols().back();
        double accidentalGap = score()->styleS(Sid::keysigAccidentalDistance).val();
        if (previous.sym != sym) {
            accidentalGap *= 2;
        } else if (previous.sym == SymId::accidentalNatural && sym == SymId::accidentalNatural) {
            accidentalGap = score()->styleS(Sid::keysigNaturalDistance).val();
        }
        double previousWidth = symWidth(previous.sym) / _spatium;
        x = previous.xPos + previousWidth + accidentalGap;
        bool isAscending = line < previous.line;
        SmuflAnchorId currentCutout = isAscending ? SmuflAnchorId::cutOutSW : SmuflAnchorId::cutOutNW;
        SmuflAnchorId previousCutout = isAscending ? SmuflAnchorId::cutOutNE : SmuflAnchorId::cutOutSE;
        PointF cutout = symSmuflAnchor(sym, currentCutout);
        double currentCutoutY = line * step + cutout.y();
        double previousCutoutY = previous.line * step + symSmuflAnchor(previous.sym, previousCutout).y();
        if ((isAscending && currentCutoutY < previousCutoutY) || (!isAscending && currentCutoutY > previousCutoutY)) {
            x -= cutout.x() / _spatium;
        }
    }
    ks.xPos = x;
    ks.line = line;
    _sig.keySymbols().push_back(ks);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout()
{
    double _spatium = spatium();
    double step = _spatium * (staff() ? staff()->staffTypeForElement(this)->lineDistance().val() * 0.5 : 0.5);

    setbbox(RectF());

    _sig.keySymbols().clear();
    if (staff() && !staff()->staffType(tick())->genKeysig()) {
        return;
    }

    // determine current clef for this staff
    ClefType clef = ClefType::G;
    if (staff()) {
        // Look for a clef before the key signature at the same tick
        Clef* c = nullptr;
        if (segment()) {
            for (Segment* seg = segment()->prev1(); !c && seg && seg->tick() == tick(); seg = seg->prev1()) {
                if (seg->isClefType() || seg->isHeaderClefType()) {
                    c = toClef(seg->element(track()));
                }
            }
        }
        if (c) {
            clef = c->clefType();
        } else {
            // no clef found, so get the clef type from the clefs list, using the previous tick
            clef = staff()->clef(tick() - Fraction::fromTicks(1));
        }
    }

    int t1 = int(_sig.key());

    if (isCustom() && !isAtonal()) {
        double accidentalGap = score()->styleS(Sid::keysigAccidentalDistance).val();
        // add standard key accidentals first, if necessary
        for (int i = 1; i <= abs(t1) && abs(t1) <= 7; ++i) {
            bool drop = false;
            for (CustDef& cd: _sig.customKeyDefs()) {
                int degree = _sig.degInKey(cd.degree);
                // if custom keysig accidental takes place, don't create tonal accidental
                if ((degree * 2 + 2) % 7 == (t1 < 0 ? 8 - i : i) % 7) {
                    drop = true;
                    break;
                }
            }
            if (!drop) {
                KeySym ks;
                int lineIndexOffset = t1 > 0 ? -1 : 6;
                ks.sym = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                ks.line = ClefInfo::lines(clef)[lineIndexOffset + i];
                if (_sig.keySymbols().size() > 0) {
                    KeySym& previous = _sig.keySymbols().back();
                    double previousWidth = symWidth(previous.sym) / _spatium;
                    ks.xPos = previous.xPos + previousWidth + accidentalGap;
                } else {
                    ks.xPos = 0;
                }
                // TODO octave metters?
                _sig.keySymbols().push_back(ks);
            }
        }
        for (CustDef& cd: _sig.customKeyDefs()) {
            SymId sym = _sig.symInKey(cd.sym, cd.degree);
            int degree = _sig.degInKey(cd.degree);
            bool flat = std::string(SymNames::nameForSymId(sym).ascii()).find("Flat") != std::string::npos;
            int accIdx = (degree * 2 + 1) % 7; // C D E F ... index to F C G D index
            accIdx = flat ? 13 - accIdx : accIdx;
            int line = ClefInfo::lines(clef)[accIdx] + cd.octAlt * 7;
            double xpos = cd.xAlt;
            if (_sig.keySymbols().size() > 0) {
                KeySym& previous = _sig.keySymbols().back();
                double previousWidth = symWidth(previous.sym) / _spatium;
                xpos += previous.xPos + previousWidth + accidentalGap;
            }
            // if translated symbol if out of range, add key accidental followed by untranslated symbol
            if (sym == SymId::noSym) {
                KeySym ks;
                ks.line = line;
                ks.xPos = xpos;
                // for quadruple sharp use two double sharps
                if (cd.sym == SymId::accidentalTripleSharp) {
                    ks.sym = SymId::accidentalDoubleSharp;
                    sym = SymId::accidentalDoubleSharp;
                } else {
                    ks.sym = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                    sym = cd.sym;
                }
                _sig.keySymbols().push_back(ks);
                xpos += t1 < 0 ? 0.7 : 1; // flats closer
            }
            // create symbol; natural only if is user defined
            if (sym != SymId::accidentalNatural || sym == cd.sym) {
                KeySym ks;
                ks.sym = sym;
                ks.line = line;
                ks.xPos = xpos;
                _sig.keySymbols().push_back(ks);
            }
        }
    } else {
        int accidentals = 0, naturals = 0;
        switch (std::abs(t1)) {
        case 7: accidentals = 0x7f;
            break;
        case 6: accidentals = 0x3f;
            break;
        case 5: accidentals = 0x1f;
            break;
        case 4: accidentals = 0xf;
            break;
        case 3: accidentals = 0x7;
            break;
        case 2: accidentals = 0x3;
            break;
        case 1: accidentals = 0x1;
            break;
        case 0: accidentals = 0;
            break;
        default:
            LOGD("illegal t1 key %d", t1);
            break;
        }

        // manage display of naturals:
        // naturals are shown if there is some natural AND prev. measure has no section break
        // AND style says they are not off
        // OR key sig is CMaj/Amin (in which case they are always shown)

        bool naturalsOn = false;
        Measure* prevMeasure = measure() ? measure()->prevMeasure() : 0;

        // If we're not force hiding naturals (Continuous panel), use score style settings
        if (!_hideNaturals) {
            const bool newSection = (!segment()
                                     || (segment()->rtick().isZero() && (!prevMeasure || prevMeasure->sectionBreak()))
                                     );
            naturalsOn = !newSection && (score()->styleI(Sid::keySigNaturals) != int(KeySigNatural::NONE) || (t1 == 0));
        }

        // Don't repeat naturals if shown in courtesy
        if (measure() && measure()->system() && measure()->isFirstInSystem()
            && prevMeasure && prevMeasure->findSegment(SegmentType::KeySigAnnounce, tick())
            && !segment()->isKeySigAnnounceType()) {
            naturalsOn = false;
        }
        if (track() == mu::nidx) {
            naturalsOn = false;
        }

        int coffset = 0;
        Key t2      = Key::C;
        if (naturalsOn) {
            if (staff()) {
                t2 = staff()->key(tick() - Fraction(1, 480 * 4));
            }
            if (t2 == Key::C) {
                naturalsOn = false;
            } else {
                switch (std::abs(int(t2))) {
                case 7: naturals = 0x7f;
                    break;
                case 6: naturals = 0x3f;
                    break;
                case 5: naturals = 0x1f;
                    break;
                case 4: naturals = 0xf;
                    break;
                case 3: naturals = 0x7;
                    break;
                case 2: naturals = 0x3;
                    break;
                case 1: naturals = 0x1;
                    break;
                case 0: naturals = 0;
                    break;
                default:
                    LOGD("illegal t2 key %d", int(t2));
                    break;
                }
                // remove redundant naturals
                if (!((t1 > 0) ^ (t2 > 0))) {
                    naturals &= ~accidentals;
                }
                if (t2 < 0) {
                    coffset = 7;
                }
            }
        }

        // naturals should go BEFORE accidentals if style says so
        // OR going from sharps to flats or vice versa (i.e. t1 & t2 have opposite signs)

        bool prefixNaturals
            =naturalsOn
              && (score()->styleI(Sid::keySigNaturals) == int(KeySigNatural::BEFORE) || t1 * int(t2) < 0);

        // naturals should go AFTER accidentals if they should not go before!
        bool suffixNaturals = naturalsOn && !prefixNaturals;

        const signed char* lines = ClefInfo::lines(clef);

        if (prefixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    addLayout(SymId::accidentalNatural, lines[i + coffset]);
                }
            }
        }
        if (abs(t1) <= 7) {
            SymId symbol = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
            int lineIndexOffset = t1 > 0 ? 0 : 7;
            for (int i = 0; i < abs(t1); ++i) {
                addLayout(symbol, lines[lineIndexOffset + i]);
            }
        } else {
            LOGD("illegal t1 key %d", t1);
        }

        // add suffixed naturals, if any
        if (suffixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    addLayout(SymId::accidentalNatural, lines[i + coffset]);
                }
            }
        }

        // Follow stepOffset
        if (staffType()) {
            setPosY(staffType()->stepOffset() * 0.5 * _spatium);
        }
    }

    // compute bbox
    for (KeySym& ks : _sig.keySymbols()) {
        double x = ks.xPos * _spatium;
        double y = ks.line * step;
        addbbox(symBbox(ks.sym).translated(x, y));
    }
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
