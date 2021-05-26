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

#include "measure.h"
#include "score.h"
#include "system.h"
#include "undo.h"
#include "chord.h"
#include "stem.h"
#include "navigate.h"
#include "slurando.h"

namespace Ms {
// sync with SlurandoType in types.h
static QString SlurLabels[] = {
    "S", "S", "H", "P", "B", "BR", "T"
};

static const ElementStyle slurandoElementStyle {
    { Sid::slurandoFontFace,    Pid::FONT_FACE },
    { Sid::slurandoFontSize,    Pid::FONT_SIZE },
    { Sid::slurandoFontStyle,   Pid::FONT_STYLE }
};

//---------------------------------------------------------
//   Slurando
//---------------------------------------------------------

Slurando::Slurando(Score* s)
    : SlurTie(s),
    _slurType(SlurandoType::HAMMER_ON),
    _textPlace(SlurandoTextPlacement::NONE),
    _fontSize(6.0),
    _fontStyle(FontStyle::Normal)
{
    setAnchor(Anchor::NOTE);
    initElementStyle(&slurandoElementStyle);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Slurando::write(XmlWriter& xml) const
{
    xml.stag(this);
    writeProperty(xml, Pid::SLURANDO_TYPE);
    writeProperty(xml, Pid::SLURANDO_PLACE);

    if (_textPlace != SlurandoTextPlacement::NONE && !_text.isEmpty()) {
        xml.tag("text", _text);
    }
    for (const StyledProperty& spp : *styledProperties()) {
        writeProperty(xml, spp.pid);
    }
    SlurTie::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slurando::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        if (tag == "subtype") {
            readProperty(e, Pid::SLURANDO_TYPE);
        } else if (tag == "place") {
            readProperty(e, Pid::SLURANDO_PLACE);
        } else if (tag == "text") {
            readProperty(e, Pid::SLURANDO_TEXT);
        } else if (readStyledProperty(e, tag)) {
            // placement and font stuff
        } else if (!SlurTie::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Slurando::getProperty(Pid id) const
{
    switch (id) {
    case Pid::SLURANDO_TYPE:
        return int(_slurType);
    case Pid::SLURANDO_PLACE:
        return int(_textPlace);
    case Pid::SLURANDO_TEXT:
        return text();
    case Pid::FONT_FACE:
        return _fontFace;
    case Pid::FONT_SIZE:
        return _fontSize;
    case Pid::FONT_STYLE:
        return int(_fontStyle);
    default:
        break;
    }
    return SlurTie::getProperty(id);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Slurando::setProperty(Pid id, const QVariant& v)
{
    switch (id) {
    case Pid::SLURANDO_TYPE:
        _slurType = SlurandoType(v.toInt());
        break;
    case Pid::SLURANDO_PLACE:
        _textPlace = SlurandoTextPlacement(v.toInt());
        break;
    case Pid::SLURANDO_TEXT:
        setText(v.toString());
        break;
    case Pid::FONT_FACE:
        setFontFace(v.toString());
        break;
    case Pid::FONT_SIZE:
        setFontSize(v.toReal());
        break;
    case Pid::FONT_STYLE:
        setFontStyle(FontStyle(v.toInt()));
        break;
    default:
        if (!SlurTie::setProperty(id, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
// propertyDefault
//---------------------------------------------------------

QVariant Slurando::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SLURANDO_TYPE:
        return int(SlurandoType::HAMMER_ON);
    case Pid::SLURANDO_PLACE:
        return int(SlurandoTextPlacement::NONE);
    case Pid::FONT_FACE:
        return score()->styleV(Sid::slurandoFontFace);
    case Pid::FONT_SIZE:
        return score()->styleV(Sid::slurandoFontSize);
    case Pid::FONT_STYLE:
        return score()->styleV(Sid::slurandoFontStyle);
    default:
        break;
    }
    return SlurTie::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   getFont
//---------------------------------------------------------

QFont Slurando::getFont() const
{
    QFont f(_fontFace);
    f.setBold(_fontStyle & FontStyle::Bold);
    f.setItalic(_fontStyle & FontStyle::Italic);
    f.setUnderline(_fontStyle & FontStyle::Underline);

    qreal m = _fontSize;
    m *= ((spatium() * MScore::pixelRatio) / SPATIUM20);
    f.setPointSizeF(m);
    return f;
}

//---------------------------------------------------------
// getLabel
//---------------------------------------------------------

const QString& Slurando::getLabel() const
{
    if (!_text.isEmpty()) {
        return _text;
    }
    return SlurLabels[int(_slurType)];
}

//---------------------------------------------------------
// showLabel
//---------------------------------------------------------

bool Slurando::showLabel() const
{
    if (_textPlace == SlurandoTextPlacement::NONE || !startNote() || !staff()) {
        return false;
    }
    // TAB staff only
    StaffType* st = staff()->staffType(startNote()->tick());
    if (!st->isTabStaff()) {
        return false;
    }
    // TAB FULL - only ON_SLUR is safe (TODO)
    if (st->stemThrough()) {
        return _textPlace == SlurandoTextPlacement::ON_SLUR;
    }
    return true;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slurando::layout()
{
    if (staff()) {
        setMag(staff()->staffMag(tick()));
    }
    if (!startNote() || !endNote()) {
        return;
    }
    System* startSystem = startNote()->chord()->measure()->system();
    System* endSystem = endNote()->chord()->measure()->system();
    if (!startSystem || !endSystem) {
        return;
    }
    SysStaff* staff = startSystem->staff(startNote()->chord()->staffIdx());
    if (!staff || !staff->show()) {
        return;
    }
    SlurandoSegment* ts = layoutFor(startSystem);
    if (ts && ts->addToSkyline()) {
        staff->skyline().add(ts->shape().translated(ts->pos()));
    }
    if (startSystem != endSystem) {
        ts = layoutBack(endSystem);
        if (ts && ts->addToSkyline()) {
            staff->skyline().add(ts->shape().translated(ts->pos()));
        }
    }
    if (showLabel()) {
        layoutLabel();
        staff->skyline().add(_labelBox);
    }
}

//---------------------------------------------------------
//   layoutFor
//    - from tie.cpp
//    layout the first SpannerSegment of a slur
//---------------------------------------------------------

SlurandoSegment* Slurando::layoutFor(System* system)
{
    // do not layout slurs in tablature if not showing back-tied fret marks
    StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
    if (st && st->isTabStaff() && !st->showBackTied()) {
        if (!segmentsEmpty()) {
            eraseSpannerSegments();
        }
        return nullptr;
    }
    //
    //    show short bow
    //
    if (startNote() == 0 || endNote() == 0) {
        if (startNote() == 0) {
            qDebug("no start note");
            return 0;
        }
        Chord* c1 = startNote()->chord();
        if (_slurDirection == Direction::AUTO) {
            if (c1->measure()->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks())) {
                // in polyphonic passage, slurs go on the stem side
                _up = c1->up();
            } else {
                _up = !c1->up();
            }
        } else {
            _up = _slurDirection == Direction::UP ? true : false;
        }
        fixupSegments(1);
        SlurandoSegment* segment = segmentAt(0);
        segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
        segment->setSystem(startNote()->chord()->segment()->measure()->system());
        SlurPos sPos;
        slurPos(&sPos);
        segment->layoutSegment(sPos.p1, sPos.p2);

        if (_slurType == SlurandoType::SHIFT_SLIDE) {
            segment->setVisible(false);
        }

        return segment;
    }
    calculateDirection();

    SlurPos sPos;
    slurPos(&sPos);

    setPos(0, 0);

    int n;
    if (sPos.system1 != sPos.system2) {
        n = 2;
        sPos.p2 = QPointF(system->width(), sPos.p1.y());
    } else {
        n = 1;
    }

    fixupSegments(n);
    SlurandoSegment* segment = segmentAt(0);
    segment->setSystem(system); // Needed to populate System.spannerSegments
    segment->layoutSegment(sPos.p1, sPos.p2);
    segment->setSpannerSegmentType(sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);

    if (_slurType == SlurandoType::SHIFT_SLIDE) {
        segment->setVisible(false);
    }

    return segment;
}

//---------------------------------------------------------
//   layoutBack
//    - from tie.cpp
//    layout the second SpannerSegment of a split slur
//---------------------------------------------------------

SlurandoSegment* Slurando::layoutBack(System* system)
{
    // do not layout slurs in tablature if not showing back-tied fret marks
    StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
    if (st->isTabStaff() && !st->showBackTied()) {
        if (!segmentsEmpty()) {
            eraseSpannerSegments();
        }
        return nullptr;
    }

    SlurPos sPos;
    slurPos(&sPos);

    fixupSegments(2);
    SlurandoSegment* segment = segmentAt(1);
    segment->setSystem(system);

    qreal x = system->firstNoteRestSegmentX(true);

    // bug fix - should go into tie.cpp some day
    x = x <= spatium() ? 0 : x;

    segment->layoutSegment(QPointF(x, sPos.p2.y()), sPos.p2);
    segment->setSpannerSegmentType(SpannerSegmentType::END);

    if (_slurType == SlurandoType::SHIFT_SLIDE) {
        segment->setVisible(false);
    }

    return segment;
}

//---------------------------------------------------------
// layoutLabel
//---------------------------------------------------------

void Slurando::layoutLabel()
{
    Note* nt1 = startNote();
    Note* nt2 = endNote();
    Chord* c1 = nt1->chord();
    Chord* c2 = nt2->chord();
    Stem* st1 = c1->stem();
    Segment* s1 = c1->segment();
    Segment* s2 = c2->segment();
    Measure* m1 = s1->measure();
    Measure* m2 = s2->measure();
    qreal _spatium = spatium();
    const QString& label = getLabel();

    QRectF r1 = bbox().translated(m1->pos() + s1->pos() + c1->pos() + nt1->pos() + pos());
    QRectF r2 = bbox().translated(m2->pos() + s2->pos() + c2->pos() + nt2->pos() + pos());

    qreal x1 = r1.x() + nt1->bbox().width();
    qreal x2 = r2.x() > r1.x() ? r2.x() : m1->system()->width();

    qreal x = x1 + (x2 - x1) / 2.0;
    qreal y = 0.0;
    qreal gap = _spatium * 0.33;

    const QFont f = getFont(); //_label->font();
    QFontMetricsF fm(f, MScore::paintDevice());
    qreal h = fm.height() / MScore::pixelRatio;
    qreal w = fm.width(label) / MScore::pixelRatio;
    qreal fontAscent = fm.ascent() / MScore::pixelRatio;

    switch (_textPlace) {
    case SlurandoTextPlacement::ON_STEM:     // TAB COMMON ONLY (TODO)
        y = st1->pos().y() + h;
        break;
    case SlurandoTextPlacement::ON_SLUR:
        if (up()) {  // label above slur
            y = r1.top() - h - gap;
        } else {    // label below slur
            y = r1.bottom() + h + gap + fontAscent;
        }
        break;
    case SlurandoTextPlacement::ABOVE_STAFF:      // TODO
    case SlurandoTextPlacement::BELOW_STAFF:      // TODO
    default:
        _labelBox = QRectF();
        return;
    }

    x -= (w / 2.0);
    _labelBox.setCoords(x, y - h, x + w, y);
}

//---------------------------------------------------------
// draw
//  - assumes Slurando::layout has been called
//---------------------------------------------------------

void SlurandoSegment::draw(mu::draw::Painter* painter) const
{
    TieSegment::draw(painter);

    Slurando* slur = toSlurando(slurTie());
    if (slur && slur->showLabel()) {
        QString label = slur->getLabel();
        if (!label.isEmpty()) {
            painter->setPen(curColor(true));
            painter->setFont(slur->getFont());
            const QRectF& box = slur->labelBox();
            painter->drawText(QRectF(box.left(), box.top(), .0, .0),
                              Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip, label);
        }
    }
}

//---------------------------------------------------------
//   noteHasTieOrSlurando
//---------------------------------------------------------

bool Slurando::noteHasTieOrSlurando(const Note* note) const
{
    if (note->tieFor()) {
        return true;
    }
    for (const Spanner* sp : note->spannerFor()) {
        if (sp->isSlurando()) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   calculateDirection
//---------------------------------------------------------

static int compareNotesPos(const Note* n1, const Note* n2)
{
    if (n1->line() != n2->line()) {
        return n2->line() - n1->line();
    } else if (n1->string() != n2->string()) {
        return n2->string() - n1->string();
    } else {
        return n1->pitch() - n2->pitch();
    }
}

void Slurando::calculateDirection()
{
    Chord* c1 = startNote()->chord();
    Chord* c2 = endNote()->chord();
    Measure* m1 = c1->measure();
    Measure* m2 = c2->measure();

    if (_slurDirection == Direction::AUTO) {
        std::vector<Note*> notes = c1->notes();
        size_t n = notes.size();
        if (m1->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks()) || m2->hasVoices(c2->staffIdx(), c2->tick(), c2->actualTicks())) {
            // in polyphonic passage, ties go on the stem side
            _up = c1->up();
        } else if (n == 1) {
            //
            // single note
            //
            if (c1->up() != c2->up()) {
                // if stem direction is mixed, always up
                _up = true;
            } else {
                _up = !c1->up();
            }
        } else {
            //
            // chords
            //
            int tiesCount = 0;
            Note* tieNote = startNote();
            Note* tieAbove = nullptr;
            Note* tieBelow = nullptr;

            for (size_t i = 0; i < n; ++i) {
                if (noteHasTieOrSlurando(notes[i])) {
                    tiesCount++;
                    int noteDiff = compareNotesPos(notes[i], tieNote);

                    if (noteDiff > 0) {
                        if (!tieAbove) {
                            tieAbove = notes[i];
                        } else if (compareNotesPos(notes[i], tieAbove) < 0) {
                            tieAbove = notes[i];
                        }
                    } else if (noteDiff < 0) {
                        if (!tieBelow) {
                            tieBelow = notes[i];
                        } else if (compareNotesPos(notes[i], tieBelow) > 0) {
                            tieBelow = notes[i];
                        }
                    }
                }
            }
            if (!tieBelow) {
                // bottom tie is up if it is the only tie and not the bottom note of the chord
                _up = tiesCount == 1 && tieNote != c1->downNote();
            } else if (!tieAbove) {
                // top tie always up
                _up = true;
            } else {
                bool tabStaff = onTabStaff();
                int tieLine = tabStaff ? tieNote->string() : tieNote->line();
                int belowLine = tabStaff ? tieBelow->string() : tieBelow->line();
                int aboveLine = tabStaff ? tieAbove->string() : tieAbove->line();

                if (tieLine <= (tabStaff ? 2 : 4)) {
                    _up = ((belowLine - tieLine) <= 1) || ((tieLine - aboveLine) > 1);
                } else {
                    _up = ((belowLine - tieLine) <= 1) && ((tieLine - aboveLine) > 1);
                }
            }
        }
    } else {
        _up = _slurDirection == Direction::UP ? true : false;
    }
}

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Slurando::slurPos(SlurPos* sp)
{
    bool useTablature = staff() && staff()->isTabStaff(tick());
    const StaffType* stt = useTablature ? staff()->staffType(tick()) : 0;
    qreal _spatium = spatium();
    qreal hw = startNote()->tabHeadWidth(stt);     // if stt == 0, defaults to headWidth()
    qreal __up = _up ? -1.0 : 1.0;
    // y offset for ties inside chord margins (typically multi-note chords): lined up with note top or bottom margin
    //    or outside (typically single-note chord): overlaps note and is above/below it
    // Outside: Tab: uses font size and may be asymmetric placed above/below line (frets ON or ABOVE line)
    //          Std: assumes notehead is 1 sp high, 1/2 sp above and 1/2 below line; add 1/4 sp to it
    // Inside:  Tab: 1/2 of Outside offset
    //          Std: use a fixed percentage of note width
    qreal yOffOutside = useTablature
                        ? (_up ? stt->fretBoxY() : stt->fretBoxY() + stt->fretBoxH()) * magS()
                        : 0.75 * _spatium * __up;
    qreal yOffInside = useTablature ? yOffOutside * 0.5 : hw * .3 * __up;

    Chord* sc = startNote()->chord();
    sp->system1 = sc->measure()->system();
    if (!sp->system1) {
        Measure* m = sc->measure();
        qDebug("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
    }

    qreal xo;
    qreal yo;
    bool shortStart = false;

    // determine attachment points
    // similar code is used in Chord::layoutPitched()
    // to allocate extra space to enforce minTieLength
    // so keep these in sync

    sp->p1 = sc->pos() + sc->segment()->pos() + sc->measure()->pos();

    //------p1
    if ((sc->notes().size() > 1) || (sc->stem() && (sc->up() == _up))) {
        xo = startNote()->x() + hw * 1.12;
        yo = startNote()->pos().y() + yOffInside;
        shortStart = true;
    } else {
        xo = startNote()->x() + hw * 0.65;
        yo = startNote()->pos().y() + yOffOutside;
    }
    sp->p1 += QPointF(xo, yo);

    //------p2
    if (endNote() == 0) {
        sp->p2 = sp->p1 + QPointF(_spatium * 3, 0.0);
        sp->system2 = sp->system1;
        return;
    }
    Chord* ec = endNote()->chord();
    sp->p2 = ec->pos() + ec->segment()->pos() + ec->measure()->pos();
    sp->system2 = ec->measure()->system();

    // force tie to be horizontal except for cross-staff or if there is a difference of line (tpc, clef, tpc)
    bool horizontal = startNote()->line() == endNote()->line() && sc->vStaffIdx() == ec->vStaffIdx();

    hw = endNote()->tabHeadWidth(stt);
    if ((ec->notes().size() > 1) || (ec->stem() && !ec->up() && !_up)) {
        xo = endNote()->x() - hw * 0.12;
        if (!horizontal) {
            yo = endNote()->pos().y() + yOffInside;
        }
    } else if (shortStart) {
        xo = endNote()->x() + hw * 0.15;
        if (!horizontal) {
            yo = endNote()->pos().y() + yOffOutside;
        }
    } else {
        xo = endNote()->x() + hw * 0.35;
        if (!horizontal) {
            yo = endNote()->pos().y() + yOffOutside;
        }
    }
    sp->p2 += QPointF(xo, yo);

    // adjust for cross-staff
    if (sc->vStaffIdx() != vStaffIdx() && sp->system1) {
        qreal diff = sp->system1->staff(sc->vStaffIdx())->y() - sp->system1->staff(vStaffIdx())->y();
        sp->p1.ry() += diff;
    }
    if (ec->vStaffIdx() != vStaffIdx() && sp->system2) {
        qreal diff = sp->system2->staff(ec->vStaffIdx())->y() - sp->system2->staff(vStaffIdx())->y();
        sp->p2.ry() += diff;
    }
}

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Slurando::setStartNote(Note* note)
{
    setStartElement(note);
    setParent(note);
}

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

Note* Slurando::startNote() const
{
    Q_ASSERT(!startElement() || startElement()->type() == ElementType::NOTE);
    return toNote(startElement());
}

//---------------------------------------------------------
//   endNote
//---------------------------------------------------------

Note* Slurando::endNote() const
{
    return toNote(endElement());
}

//---------------------------------------------------------
//   layoutWidth
//---------------------------------------------------------

qreal Slurando::layoutWidth(const Note* note, Chord* ch)
{
    calculateDirection();
    qreal mag_ = staff() ? staff()->staffMag(this) : 1.0;    // palette elements do not have a staff
    qreal minTieLength = score()->styleP(Sid::MinTieLength) + mag_;
    qreal overlap = 0.0;
    bool shortStart = false;
    Note* sn = startNote();
    Chord* sc = sn->chord();
    if (sc && sc->measure() == ch->measure() && sc == prevChordRest(ch)) {
        if (sc->notes().size() > 1 || (sc->stem() && sc->up() == up())) {
            shortStart = true;
            if (sc->width() > sn->width()) {
                // chord with second?
                // account for noteheads further to right
                qreal snEnd = sn->x() + sn->bboxRightPos();
                qreal scEnd = snEnd;
                for (unsigned i = 0; i < sc->notes().size(); ++i) {
                    scEnd = qMax(scEnd, sc->notes().at(i)->x() + sc->notes().at(i)->bboxRightPos());
                }
                overlap += scEnd - snEnd;
            } else {
                overlap -= sn->headWidth() * 0.12;
            }
        } else {
            overlap += sn->headWidth() * 0.35;
        }

        if (ch->notes().size() > 1 || (ch->stem() && !ch->up() && !up())) {
            // for positive offset:
            //    use available space
            // for negative x offset:
            //    space is allocated elsewhere, so don't re-allocate here
            if (note->ipos().x() != 0.0) {
                overlap += qAbs(note->ipos().x());
            } else {
                overlap -= note->headWidth() * 0.12;
            }
        } else {
            if (shortStart) {
                overlap += note->headWidth() * 0.15;
            } else {
                overlap += note->headWidth() * 0.35;
            }
        }
    }
    return qMax(minTieLength - overlap, 0.0);
}

//---------------------------------------------------------
//   layoutWidthTab
//---------------------------------------------------------

qreal Slurando::layoutWidthTab(const Note* note, Chord* ch)
{
    calculateDirection();
    qreal mag_ = staff() ? staff()->staffMag(this) : 1.0;    // palette elements do not have a staff
    qreal minTieLength = score()->styleP(Sid::MinTieLength) * mag_;
    qreal fretWidth = note->bbox().width();
    qreal overlap = 0.0;          // how much tie can overlap start and end notes
    bool shortStart = false;      // whether tie should clear start note or not

    Note* sNote = startNote();
    Chord* startChord = sNote->chord();

    if (startChord && startChord->measure() == ch->measure() && startChord == prevChordRest(ch)) {
        qreal startNoteWidth = sNote->width();
        // overlap into start chord?
        // if in start chord, there are several notes or stem and tie in same direction
        if (startChord->notes().size() > 1 || (startChord->stem() && startChord->up() == up())) {
            // clear start note (1/8 of fret mark width)
            shortStart = true;
            overlap -= startNoteWidth * 0.125;
        } else {    // overlap start note (by ca. 1/3 of fret mark width)
            overlap += startNoteWidth * 0.35;
        }
        // overlap into end chord (this)?
        // if several notes or neither stem or tie are up
        if (ch->notes().size() > 1 || (ch->stem() && !ch->up() && !up())) {
            // for positive offset:
            //    use available space
            // for negative x offset:
            //    space is allocated elsewhere, so don't re-allocate here
            if (note->ipos().x() != 0.0) {            // this probably does not work for TAB, as
                overlap += qAbs(note->ipos().x());  // _pos is used to centre the fret on the stem
            } else {
                overlap -= fretWidth * 0.125;
            }
        } else {
            if (shortStart) {
                overlap += fretWidth * 0.15;
            } else {
                overlap += fretWidth * 0.35;
            }
        }
    }
    return qMax(minTieLength - overlap, 0.0);
}
} // namespace
