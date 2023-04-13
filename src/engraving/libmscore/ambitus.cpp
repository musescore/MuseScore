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

#include "ambitus.h"

#include "translation.h"

#include "draw/types/pen.h"

#include "rw/xml.h"
#include "rw/206/read206.h"

#include "accidental.h"
#include "chord.h"
#include "factory.h"
#include "measure.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
const Spatium Ambitus::LINEWIDTH_DEFAULT = Spatium(0.12);
//---------------------------------------------------------
//   Ambitus
//---------------------------------------------------------

Ambitus::Ambitus(Segment* parent)
    : EngravingItem(ElementType::AMBITUS, parent, ElementFlag::ON_STAFF)
{
    _noteHeadGroup    = NOTEHEADGROUP_DEFAULT;
    _noteHeadType     = NOTEHEADTYPE_DEFAULT;
    _dir              = DIR_DEFAULT;
    _hasLine          = HASLINE_DEFAULT;
    _lineWidth        = LINEWIDTH_DEFAULT;
    _topPitch         = INVALID_PITCH;
    _bottomPitch      = INVALID_PITCH;
    _topTpc           = Tpc::TPC_INVALID;
    _bottomTpc        = Tpc::TPC_INVALID;

    _topAccid = Factory::createAccidental(this, false);
    _bottomAccid = Factory::createAccidental(this, false);
    _topAccid->setParent(this);
    _bottomAccid->setParent(this);
}

Ambitus::Ambitus(const Ambitus& a)
    : EngravingItem(a)
{
    _noteHeadGroup = a._noteHeadGroup;
    _noteHeadType = a._noteHeadType;
    _dir = a._dir;
    _hasLine = a._hasLine;
    _lineWidth = a._lineWidth;
    _topAccid = a._topAccid->clone();
    _bottomAccid = a._bottomAccid->clone();
    _topPitch = a._topPitch;
    _topTpc = a._topTpc;
    _bottomPitch = a._bottomPitch;
    _bottomTpc = a._bottomTpc;

    _topPos = a._topPos;
    _bottomPos = a._bottomPos;
    _line = a._line;
}

Ambitus::~Ambitus()
{
    delete _topAccid;
    delete _bottomAccid;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Ambitus::mag() const
{
    return staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   initFrom
//---------------------------------------------------------

void Ambitus::initFrom(Ambitus* a)
{
    _noteHeadGroup   = a->_noteHeadGroup;
    _noteHeadType    = a->_noteHeadType;
    _dir             = a->_dir;
    _hasLine         = a->_hasLine;
    _lineWidth       = a->_lineWidth;
    _topPitch        = a->_topPitch;
    _bottomPitch     = a->_bottomPitch;
    _topTpc          = a->_topTpc;
    _bottomTpc       = a->_bottomTpc;
}

//---------------------------------------------------------
//   setTrack
//
//    when the Ambitus element is assigned a track,
//    initialize top and bottom 'notes' to top and bottom staff lines
//---------------------------------------------------------

void Ambitus::setTrack(track_idx_t t)
{
    Segment* segm  = segment();
    Staff* stf   = score()->staff(track2staff(t));

    EngravingItem::setTrack(t);
    // if not initialized and there is a segment and a staff,
    // initialize pitches and tpc's to first and last staff line
    // (for use in palettes)
    if (_topPitch == INVALID_PITCH || _topTpc == Tpc::TPC_INVALID
        || _bottomPitch == INVALID_PITCH || _bottomTpc == Tpc::TPC_INVALID) {
        if (segm && stf) {
            Ambitus::Ranges ranges = estimateRanges();
            _topTpc = ranges.topTpc;
            _bottomTpc = ranges.bottomTpc;
            _topPitch = ranges.topPitch;
            _bottomPitch = ranges.bottomPitch;

            _topAccid->setTrack(t);
            _bottomAccid->setTrack(t);
        }
//            else {
//                  _topPitch = _bottomPitch = INVALID_PITCH;
//                  _topTpc   = _bottomTpc   = Tpc::TPC_INVALID;
    }
}

//---------------------------------------------------------
//   setTop/BottomPitch
//
//    setting either pitch requires to adjust the corresponding tpc
//---------------------------------------------------------

void Ambitus::setTopPitch(int val, bool applyLogic)
{
    if (!applyLogic) {
        _topPitch   = val;
        return;
    }

    int deltaPitch    = val - topPitch();
    // if deltaPitch is not an integer number of octaves, adjust tpc
    // (to avoid 'wild' tpc changes with octave changes)
    if (deltaPitch % PITCH_DELTA_OCTAVE != 0) {
        int newTpc        = topTpc() + deltaPitch * TPC_DELTA_SEMITONE;
        // reduce newTpc into acceptable range via enharmonic
        while (newTpc < Tpc::TPC_MIN) {
            newTpc += TPC_DELTA_ENHARMONIC;
        }
        while (newTpc > Tpc::TPC_MAX) {
            newTpc -= TPC_DELTA_ENHARMONIC;
        }
        _topTpc     = newTpc;
    }
    _topPitch   = val;
    normalize();
}

void Ambitus::setBottomPitch(int val, bool applyLogic)
{
    if (!applyLogic) {
        _bottomPitch = val;
        return;
    }

    int deltaPitch    = val - bottomPitch();
    // if deltaPitch is not an integer number of octaves, adjust tpc
    // (to avoid 'wild' tpc changes with octave changes)
    if (deltaPitch % PITCH_DELTA_OCTAVE != 0) {
        int newTpc        = bottomTpc() + deltaPitch * TPC_DELTA_SEMITONE;
        // reduce newTpc into acceptable range via enharmonic
        while (newTpc < Tpc::TPC_MIN) {
            newTpc += TPC_DELTA_ENHARMONIC;
        }
        while (newTpc > Tpc::TPC_MAX) {
            newTpc -= TPC_DELTA_ENHARMONIC;
        }
        _bottomTpc  = newTpc;
    }
    _bottomPitch= val;
    normalize();
}

//---------------------------------------------------------
//   setTop/BottomTpc
//
//    setting either tpc requires to adjust the corresponding pitch
//    (but remaining in the same octave)
//---------------------------------------------------------

void Ambitus::setTopTpc(int val, bool applyLogic)
{
    if (!applyLogic) {
        _topTpc = val;
        return;
    }

    int octave        = topPitch() / PITCH_DELTA_OCTAVE;
    int deltaTpc      = val - topTpc();
    // get new pitch according to tpc change
    int newPitch      = topPitch() + deltaTpc * TPC_DELTA_SEMITONE;
    // reduce pitch to the same octave as original pitch
    newPitch          = (octave * PITCH_DELTA_OCTAVE) + (newPitch % PITCH_DELTA_OCTAVE);
    _topPitch   = newPitch;
    _topTpc     = val;
    normalize();
}

void Ambitus::setBottomTpc(int val, bool applyLogic)
{
    if (!applyLogic) {
        _bottomTpc = val;
        return;
    }

    int octave        = bottomPitch() / PITCH_DELTA_OCTAVE;
    int deltaTpc      = val - bottomTpc();
    // get new pitch according to tpc change
    int newPitch      = bottomPitch() + deltaTpc * TPC_DELTA_SEMITONE;
    // reduce pitch to the same octave as original pitch
    newPitch          = (octave * PITCH_DELTA_OCTAVE) + (newPitch % PITCH_DELTA_OCTAVE);
    _bottomPitch= newPitch;
    _bottomTpc  = val;
    normalize();
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ambitus::write(XmlWriter& xml) const
{
    UNREACHABLE;
    xml.startElement(this);
    xml.tagProperty(Pid::HEAD_GROUP, int(_noteHeadGroup), int(NOTEHEADGROUP_DEFAULT));
    xml.tagProperty(Pid::HEAD_TYPE,  int(_noteHeadType),  int(NOTEHEADTYPE_DEFAULT));
    xml.tagProperty(Pid::MIRROR_HEAD, int(_dir),           int(DIR_DEFAULT));
    xml.tag("hasLine",    _hasLine, true);
    xml.tagProperty(Pid::LINE_WIDTH_SPATIUM, _lineWidth, LINEWIDTH_DEFAULT);
    xml.tag("topPitch",   _topPitch);
    xml.tag("topTpc",     _topTpc);
    xml.tag("bottomPitch", _bottomPitch);
    xml.tag("bottomTpc",  _bottomTpc);
    if (_topAccid->accidentalType() != AccidentalType::NONE) {
        xml.startElement("topAccidental");
        _topAccid->write(xml);
        xml.endElement();
    }
    if (_bottomAccid->accidentalType() != AccidentalType::NONE) {
        xml.startElement("bottomAccidental");
        _bottomAccid->write(xml);
        xml.endElement();
    }
    EngravingItem::writeProperties(xml);
    xml.endElement();
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ambitus::layout()
{
    int bottomLine, topLine;
    ClefType clf;
    double headWdt     = headWidth();
    Key key;
    double lineDist;
    int numOfLines;
    Segment* segm        = segment();
    double _spatium    = spatium();
    Staff* stf         = nullptr;
    if (segm && track() != mu::nidx) {
        Fraction tick    = segm->tick();
        stf         = score()->staff(staffIdx());
        lineDist    = stf->lineDistance(tick) * _spatium;
        numOfLines  = stf->lines(tick);
        clf         = stf->clef(tick);
    } else {                              // for use in palettes
        lineDist    = _spatium;
        numOfLines  = 3;
        clf         = ClefType::G;
    }

    //
    // NOTEHEADS Y POS
    //
    // if pitch == INVALID_PITCH or tpc == Tpc::TPC_INVALID, set to some default:
    // for use in palettes and when actual range cannot be calculated (new ambitus or no notes in staff)
    //
    double xAccidOffTop    = 0;
    double xAccidOffBottom = 0;
    if (stf) {
        key = stf->key(segm->tick());
    } else {
        key = Key::C;
    }

    // top notehead
    if (_topPitch == INVALID_PITCH || _topTpc == Tpc::TPC_INVALID) {
        _topPos.setY(0.0);    // if uninitialized, set to top staff line
    } else {
        topLine  = absStep(_topTpc, _topPitch);
        topLine  = relStep(topLine, clf);
        _topPos.setY(topLine * lineDist * 0.5);
        // compute accidental
        AccidentalType accidType;
        // if (13 <= (tpc - key) <= 19) there is no accidental)
        if (_topTpc - int(key) >= 13 && _topTpc - int(key) <= 19) {
            accidType = AccidentalType::NONE;
        } else {
            AccidentalVal accidVal = tpc2alter(_topTpc);
            accidType = Accidental::value2subtype(accidVal);
            if (accidType == AccidentalType::NONE) {
                accidType = AccidentalType::NATURAL;
            }
        }
        _topAccid->setAccidentalType(accidType);
        if (accidType != AccidentalType::NONE) {
            _topAccid->layout();
        } else {
            _topAccid->setbbox(RectF());
        }
        _topAccid->setPosY(_topPos.y());
    }

    // bottom notehead
    if (_bottomPitch == INVALID_PITCH || _bottomTpc == Tpc::TPC_INVALID) {
        _bottomPos.setY((numOfLines - 1) * lineDist);             // if uninitialized, set to last staff line
    } else {
        bottomLine  = absStep(_bottomTpc, _bottomPitch);
        bottomLine  = relStep(bottomLine, clf);
        _bottomPos.setY(bottomLine * lineDist * 0.5);
        // compute accidental
        AccidentalType accidType;
        if (_bottomTpc - int(key) >= 13 && _bottomTpc - int(key) <= 19) {
            accidType = AccidentalType::NONE;
        } else {
            AccidentalVal accidVal = tpc2alter(_bottomTpc);
            accidType = Accidental::value2subtype(accidVal);
            if (accidType == AccidentalType::NONE) {
                accidType = AccidentalType::NATURAL;
            }
        }
        _bottomAccid->setAccidentalType(accidType);
        if (accidType != AccidentalType::NONE) {
            _bottomAccid->layout();
        } else {
            _bottomAccid->setbbox(RectF());
        }
        _bottomAccid->setPosY(_bottomPos.y());
    }

    //
    // NOTEHEAD X POS
    //
    // Note: manages colliding accidentals
    //
    double accNoteDist = point(score()->styleS(Sid::accidentalNoteDistance));
    xAccidOffTop      = _topAccid->width() + accNoteDist;
    xAccidOffBottom   = _bottomAccid->width() + accNoteDist;

    // if top accidental extends down more than bottom accidental extends up,
    // AND ambitus is not leaning right, bottom accidental needs to be displaced
    bool collision
        =(_topAccid->ipos().y() + _topAccid->bbox().y() + _topAccid->height()
          > _bottomAccid->ipos().y() + _bottomAccid->bbox().y())
          && _dir != DirectionH::RIGHT;
    if (collision) {
        // displace bottom accidental (also attempting to 'undercut' flats)
        xAccidOffBottom = xAccidOffTop
                          + ((_bottomAccid->accidentalType() == AccidentalType::FLAT
                              || _bottomAccid->accidentalType() == AccidentalType::FLAT2
                              || _bottomAccid->accidentalType() == AccidentalType::NATURAL)
                             ? _bottomAccid->width() * 0.5 : _bottomAccid->width());
    }

    switch (_dir) {
    case DirectionH::AUTO:                       // noteheads one above the other
        // left align noteheads and right align accidentals 'hanging' on the left
        _topPos.setX(0.0);
        _bottomPos.setX(0.0);
        _topAccid->setPosX(-xAccidOffTop);
        _bottomAccid->setPosX(-xAccidOffBottom);
        break;
    case DirectionH::LEFT:                       // top notehead at the left of bottom notehead
        // place top notehead at left margin; bottom notehead at right of top head;
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        _topPos.setX(0.0);
        _bottomPos.setX(headWdt);
        _topAccid->setPosX(-xAccidOffTop);
        _bottomAccid->setPosX(collision ? -xAccidOffBottom : headWdt - xAccidOffBottom);
        break;
    case DirectionH::RIGHT:                      // top notehead at the right of bottom notehead
        // bottom notehead at left margin; top notehead at right of bottomnotehead
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        _bottomPos.setX(0.0);
        _topPos.setX(headWdt);
        _bottomAccid->setPosX(-xAccidOffBottom);
        _topAccid->setPosX(headWdt - xAccidOffTop);
        break;
    }

    // compute line from top note centre to bottom note centre
    mu::LineF fullLine(_topPos.x() + headWdt * 0.5, _topPos.y(), _bottomPos.x() + headWdt * 0.5, _bottomPos.y());
    // shorten line on each side by offsets
    double yDelta = _bottomPos.y() - _topPos.y();
    if (yDelta != 0.0) {
        double off = _spatium * LINEOFFSET_DEFAULT;
        mu::PointF p1 = fullLine.pointAt(off / yDelta);
        mu::PointF p2 = fullLine.pointAt(1 - (off / yDelta));
        _line = mu::LineF(p1, p2);
    } else {
        _line = fullLine;
    }

    mu::RectF headRect(0, -0.5 * _spatium, headWdt, 1 * _spatium);
    setbbox(headRect.translated(_topPos).united(headRect.translated(_bottomPos))
            .united(_topAccid->bbox().translated(_topAccid->ipos()))
            .united(_bottomAccid->bbox().translated(_bottomAccid->ipos()))
            );
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Ambitus::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    double _spatium = spatium();
    double lw = lineWidth().val() * _spatium;
    painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
    drawSymbol(noteHead(), painter, _topPos);
    drawSymbol(noteHead(), painter, _bottomPos);
    if (_hasLine) {
        painter->drawLine(_line);
    }

    // draw ledger lines (if not in a palette)
    if (segment() && track() != mu::nidx) {
        Fraction tick  = segment()->tick();
        Staff* staff   = score()->staff(staffIdx());
        double lineDist = staff->lineDistance(tick);
        int numOfLines = staff->lines(tick);
        double step     = lineDist * _spatium;
        double stepTolerance    = step * 0.1;
        double ledgerLineLength = score()->styleS(Sid::ledgerLineLength).val() * _spatium;
        double ledgerLineWidth  = score()->styleS(Sid::ledgerLineWidth).val() * _spatium;
        painter->setPen(Pen(curColor(), ledgerLineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));

        if (_topPos.y() - stepTolerance <= -step) {
            double xMin = _topPos.x() - ledgerLineLength;
            double xMax = _topPos.x() + headWidth() + ledgerLineLength;
            for (double y = -step; y >= _topPos.y() - stepTolerance; y -= step) {
                painter->drawLine(mu::PointF(xMin, y), mu::PointF(xMax, y));
            }
        }

        if (_bottomPos.y() + stepTolerance >= numOfLines * step) {
            double xMin = _bottomPos.x() - ledgerLineLength;
            double xMax = _bottomPos.x() + headWidth() + ledgerLineLength;
            for (double y = numOfLines * step; y <= _bottomPos.y() + stepTolerance; y += step) {
                painter->drawLine(mu::PointF(xMin, y), mu::PointF(xMax, y));
            }
        }
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Ambitus::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    UNUSED(all);
    func(data, this);
    if (_topAccid->accidentalType() != AccidentalType::NONE) {
        func(data, _topAccid);
    }

    if (_bottomAccid->accidentalType() != AccidentalType::NONE) {
        func(data, _bottomAccid);
    }
}

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Ambitus::noteHead() const
{
    int hg = 1;
    NoteHeadType ht  = NoteHeadType::HEAD_QUARTER;

    if (_noteHeadType != NoteHeadType::HEAD_AUTO) {
        ht = _noteHeadType;
    }

    SymId t = Note::noteHead(hg, _noteHeadGroup, ht);
    if (t == SymId::noSym) {
        LOGD("invalid notehead %d/%d", int(_noteHeadGroup), int(_noteHeadType));
        t = Note::noteHead(0, NoteHeadGroup::HEAD_NORMAL, ht);
    }
    return t;
}

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the notehead symbol
//---------------------------------------------------------

double Ambitus::headWidth() const
{
//      int head  = noteHead();
//      double val = symbols[score()->symIdx()][head].width(magS());
//      return val;
    return symWidth(noteHead());
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

mu::PointF Ambitus::pagePos() const
{
    if (explicitParent() == 0) {
        return pos();
    }
    System* system = segment()->measure()->system();
    double yp = y();
    if (system) {
        yp += system->staff(staffIdx())->y() + system->y();
    }
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   normalize
//
//    makes sure _topPitch is not < _bottomPitch
//---------------------------------------------------------

void Ambitus::normalize()
{
    if (_topPitch < _bottomPitch) {
        int temp    = _topPitch;
        _topPitch   = _bottomPitch;
        _bottomPitch= temp;
        temp        = _topTpc;
        _topTpc     = _bottomTpc;
        _bottomTpc  = temp;
    }
}

//---------------------------------------------------------
//   updateRange
//
//    scans the staff contents up to next section break to update the range pitches/tpc's
//---------------------------------------------------------

Ambitus::Ranges Ambitus::estimateRanges() const
{
    Ambitus::Ranges result;

    if (!segment()) {
        return result;
    }
    Chord* chord;
    track_idx_t firstTrack  = track();
    track_idx_t lastTrack   = firstTrack + VOICES - 1;
    int pitchTop    = -1000;
    int pitchBottom = 1000;
    int tpcTop      = 0;    // Initialized to prevent warning
    int tpcBottom   = 0;    // Initialized to prevent warning
    track_idx_t trk;
    Measure* meas     = segment()->measure();
    Segment* segm     = meas->findSegment(SegmentType::ChordRest, segment()->tick());
    bool stop     = meas->sectionBreak();
    while (segm) {
        // moved to another measure?
        if (segm->measure() != meas) {
            // if section break has been found, stop here
            if (stop) {
                break;
            }
            // update meas and stop condition
            meas = segm->measure();
            stop = meas->sectionBreak();
        }
        // scan all relevant tracks of this segment for chords
        for (trk = firstTrack; trk <= lastTrack; trk++) {
            EngravingItem* e = segm->element(trk);
            if (!e || !e->isChord()) {
                continue;
            }
            chord = toChord(e);
            // update pitch range (with associated tpc's)
            for (Note* n : chord->notes()) {
                if (!n->play()) {         // skip notes which are not to be played
                    continue;
                }
                int pitch = n->epitch();
                if (pitch > pitchTop) {
                    pitchTop = pitch;
                    tpcTop   = n->tpc();
                }
                if (pitch < pitchBottom) {
                    pitchBottom = pitch;
                    tpcBottom   = n->tpc();
                }
            }
        }
        segm = segm->nextCR();
    }

    if (pitchTop > -1000) {               // if something has been found, update this
        result.topPitch    = pitchTop;
        result.bottomPitch = pitchBottom;
        result.topTpc      = tpcTop;
        result.bottomTpc   = tpcBottom;
    }

    return result;
}

void Ambitus::remove(EngravingItem* e)
{
    if (e->type() == ElementType::ACCIDENTAL) {
        //! NOTE Do nothing (removing _topAccid or _bottomAccid)
        return;
    }

    EngravingItem::remove(e);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Ambitus::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::HEAD_GROUP:
        return int(noteHeadGroup());
    case Pid::HEAD_TYPE:
        return int(noteHeadType());
    case Pid::MIRROR_HEAD:
        return int(direction());
    case Pid::GHOST:                         // recycled property = _hasLine
        return hasLine();
    case Pid::LINE_WIDTH_SPATIUM:
        return lineWidth();
    case Pid::TPC1:
        return topTpc();
    case Pid::FBPARENTHESIS1:                // recycled property = _bottomTpc
        return bottomTpc();
    case Pid::PITCH:
        return topPitch();
    case Pid::FBPARENTHESIS2:                // recycled property = _bottomPitch
        return bottomPitch();
    case Pid::FBPARENTHESIS3:                // recycled property = octave of _topPitch
        return topOctave();
    case Pid::FBPARENTHESIS4:                // recycled property = octave of _bottomPitch
        return bottomOctave();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ambitus::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::HEAD_GROUP:
        setNoteHeadGroup(v.value<NoteHeadGroup>());
        break;
    case Pid::HEAD_TYPE:
        setNoteHeadType(v.value<NoteHeadType>());
        break;
    case Pid::MIRROR_HEAD:
        setDirection(v.value<DirectionH>());
        break;
    case Pid::GHOST:                         // recycled property = _hasLine
        setHasLine(v.toBool());
        break;
    case Pid::LINE_WIDTH_SPATIUM:
        setLineWidth(v.value<Spatium>());
        break;
    case Pid::TPC1:
        setTopTpc(v.toInt());
        break;
    case Pid::FBPARENTHESIS1:                // recycled property = _bottomTpc
        setBottomTpc(v.toInt());
        break;
    case Pid::PITCH:
        setTopPitch(v.toInt());
        break;
    case Pid::FBPARENTHESIS2:                // recycled property = _bottomPitch
        setBottomPitch(v.toInt());
        break;
    case Pid::FBPARENTHESIS3:                // recycled property = octave of _topPitch
        setTopPitch(topPitch() % 12 + (v.toInt() + 1) * 12);
        break;
    case Pid::FBPARENTHESIS4:                // recycled property = octave of _bottomPitch
        setBottomPitch(bottomPitch() % 12 + (v.toInt() + 1) * 12);
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Ambitus::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::HEAD_GROUP:
        return int(NOTEHEADGROUP_DEFAULT);
    case Pid::HEAD_TYPE:
        return int(NOTEHEADTYPE_DEFAULT);
    case Pid::MIRROR_HEAD:
        return int(DIR_DEFAULT);
    case Pid::GHOST:
        return HASLINE_DEFAULT;
    case Pid::LINE_WIDTH_SPATIUM:
        return Spatium(LINEWIDTH_DEFAULT);
    case Pid::TPC1:
        return estimateRanges().topTpc;
    case Pid::FBPARENTHESIS1:
        return estimateRanges().bottomTpc;
    case Pid::PITCH:
        return estimateRanges().topPitch;
    case Pid::FBPARENTHESIS2:
        return estimateRanges().bottomPitch;
    case Pid::FBPARENTHESIS3:
        return int(estimateRanges().topPitch / 12) - 1;
    case Pid::FBPARENTHESIS4:
        return int(estimateRanges().bottomPitch / 12) - 1;
    default:
        return EngravingItem::propertyDefault(id);
    }
    //return QVariant();
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Ambitus::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Ambitus::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Ambitus::accessibleInfo() const
{
    return EngravingItem::accessibleInfo() + u"; "
           + mtrc("engraving", "Top pitch: %1; Bottom pitch: %2")
           .arg(tpc2name(topTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, false) + String::number(topOctave()),
                tpc2name(bottomTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, false) + String::number(bottomOctave()));
}
}
