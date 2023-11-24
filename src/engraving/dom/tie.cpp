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
#include "tie.h"

#include <cmath>

#include "draw/types/transform.h"

#include "accidental.h"
#include "chord.h"
#include "hook.h"
#include "ledgerline.h"
#include "measure.h"
#include "mscoreview.h"
#include "note.h"
#include "notedot.h"
#include "score.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;

namespace mu::engraving {
Note* Tie::editStartNote;
Note* Tie::editEndNote;

TieSegment::TieSegment(System* parent)
    : SlurTieSegment(ElementType::TIE_SEGMENT, parent)
{
    m_autoAdjustOffset = mu::PointF();
}

TieSegment::TieSegment(const TieSegment& s)
    : SlurTieSegment(s)
{
    m_autoAdjustOffset = mu::PointF();
}

bool TieSegment::isEditAllowed(EditData& ed) const
{
    if (ed.key == Key_Home && !ed.modifiers) {
        return true;
    }

    return false;
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool TieSegment::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    SlurTie* sl = tie();

    if (ed.key == Key_Home && !ed.modifiers) {
        if (ed.hasCurrentGrip()) {
            ups(ed.curGrip).off = PointF();
            renderer()->layoutItem(sl);
            triggerLayout();
        }
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void TieSegment::changeAnchor(EditData& ed, EngravingItem* element)
{
    if (ed.curGrip == Grip::START) {
        spanner()->setStartElement(element);
        Note* note = toNote(element);
        if (note->chord()->tick() <= tie()->endNote()->chord()->tick()) {
            tie()->startNote()->setTieFor(0);
            tie()->setStartNote(note);
            note->setTieFor(tie());
        }
    } else {
        spanner()->setEndElement(element);
        Note* note = toNote(element);
        // do not allow backward ties
        if (note->chord()->tick() >= tie()->startNote()->chord()->tick()) {
            tie()->endNote()->setTieBack(0);
            tie()->setEndNote(note);
            note->setTieBack(tie());
        }
    }

    const size_t segments  = spanner()->spannerSegments().size();
    ups(ed.curGrip).off = PointF();
    renderer()->layoutItem(spanner());
    if (spanner()->spannerSegments().size() != segments) {
        const std::vector<SpannerSegment*>& ss = spanner()->spannerSegments();

        TieSegment* newSegment = toTieSegment(ed.curGrip == Grip::END ? ss.back() : ss.front());
        score()->endCmd();
        score()->startCmd();
        ed.view()->changeEditElement(newSegment);
        triggerLayout();
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void TieSegment::editDrag(EditData& ed)
{
    consolidateAdjustmentOffsetIntoUserOffset();
    Grip g = ed.curGrip;
    ups(g).off += ed.delta;

    if (g == Grip::START || g == Grip::END) {
        computeBezier();
        //
        // move anchor for slurs/ties
        //
        if ((g == Grip::START && isSingleBeginType()) || (g == Grip::END && isSingleEndType())) {
            Spanner* spanner = tie();
            EngravingItem* e = ed.view()->elementNear(ed.pos);
            Note* note = (e && e->isNote()) ? toNote(e) : nullptr;
            if (note && ((g == Grip::END && note->tick() > tie()->tick()) || (g == Grip::START && note->tick() < tie()->tick2()))) {
                if (g == Grip::END) {
                    Tie* tie = toTie(spanner);
                    if (tie->startNote()->pitch() == note->pitch()
                        && tie->startNote()->chord()->tick() < note->chord()->tick()) {
                        ed.view()->setDropTarget(note);
                        if (note != tie->endNote()) {
                            changeAnchor(ed, note);
                            return;
                        }
                    }
                }
            } else {
                ed.view()->setDropTarget(0);
            }
        }
    } else if (g == Grip::BEZIER1 || g == Grip::BEZIER2) {
        computeBezier();
    } else if (g == Grip::SHOULDER) {
        ups(g).off = PointF();
        ups(Grip::BEZIER1).off += ed.delta;
        ups(Grip::BEZIER2).off += ed.delta;
        computeBezier();
    } else if (g == Grip::DRAG) {
        ups(Grip::DRAG).off = PointF();
        roffset() += ed.delta;
    }
}

void TieSegment::computeMidThickness(double tieLengthInSp)
{
    const double mag = staff() ? staff()->staffMag(tie()->tick()) : 1.0;
    const double minTieLength = mag * style().styleS(Sid::MinTieLength).val();
    const double shortTieLimit = mag * 4.0;
    const double minTieThickness = mag * (0.15 * spatium() - style().styleMM(Sid::SlurEndWidth));
    const double normalThickness = mag * (style().styleMM(Sid::SlurMidWidth) - style().styleMM(Sid::SlurEndWidth));

    bool invalid = RealIsEqualOrMore(minTieLength, shortTieLimit);

    if (tieLengthInSp > shortTieLimit || invalid) {
        m_midThickness = normalThickness;
    } else {
        const double A = 1 / (shortTieLimit - minTieLength);
        const double B = normalThickness - minTieThickness;
        const double C = shortTieLimit * minTieThickness - minTieLength * normalThickness;
        m_midThickness = A * (B * tieLengthInSp + C);
    }
}

//---------------------------------------------------------
//   computeBezier
//    compute help points of slur bezier segment
//---------------------------------------------------------

void TieSegment::computeBezier(PointF shoulderOffset)
{
    const PointF tieStart = ups(Grip::START).p + ups(Grip::START).off;
    const PointF tieEnd = ups(Grip::END).p + ups(Grip::END).off;

    PointF tieEndNormalized = tieEnd - tieStart;  // normalize to zero
    if (RealIsNull(tieEndNormalized.x())) {
        return;
    }

    const double tieAngle = atan(tieEndNormalized.y() / tieEndNormalized.x()); // angle required from tie start to tie end--zero if horizontal
    Transform t;
    t.rotateRadians(-tieAngle);  // rotate so that we are working with horizontal ties regardless of endpoint height difference
    tieEndNormalized = t.map(tieEndNormalized);  // apply that rotation
    shoulderOffset = t.map(shoulderOffset);  // also apply to shoulderOffset

    const double _spatium = spatium();
    double tieLengthInSp = tieEndNormalized.x() / _spatium;

    const double minShoulderHeight = style().styleMM(Sid::tieMinShoulderHeight);
    const double maxShoulderHeight = style().styleMM(Sid::tieMaxShoulderHeight);
    double shoulderH = minShoulderHeight + _spatium * 0.3 * sqrt(abs(tieLengthInSp - 1));
    shoulderH = std::clamp(shoulderH, minShoulderHeight, maxShoulderHeight);

    shoulderH -= shoulderOffset.y();

    PointF shoulderAdjustOffset = tie()->up() ? PointF(0.0, shoulderOffset.y()) : PointF(0.0, -shoulderOffset.y());
    addAdjustmentOffset(shoulderAdjustOffset, Grip::BEZIER1);
    addAdjustmentOffset(shoulderAdjustOffset, Grip::BEZIER2);

    if (!tie()->up()) {
        shoulderH = -shoulderH;
    }

    double shoulderW = 0.6; // TODO: style

    const double tieWidth = tieEndNormalized.x();
    const double bezier1X = (tieWidth - tieWidth * shoulderW) * .5 + shoulderOffset.x();
    const double bezier2X = bezier1X + tieWidth * shoulderW + shoulderOffset.x();

    const PointF tieDrag = PointF(tieWidth * .5, 0.0);

    const PointF bezier1(bezier1X, -shoulderH);
    const PointF bezier2(bezier2X, -shoulderH);

    computeMidThickness(tieLengthInSp);

    PointF tieThickness(0.0, m_midThickness);

    const PointF bezier1Offset = t.map(ups(Grip::BEZIER1).off);
    const PointF bezier2Offset = t.map(ups(Grip::BEZIER2).off);

    //-----------------------------------calculate p6
    const PointF bezier1Final = bezier1 + bezier1Offset;
    const PointF bezier2Final = bezier2 + bezier2Offset;

    const PointF tieShoulder = 0.5 * (bezier1Final + bezier2Final);
    //-----------------------------------

    m_path = PainterPath();
    m_path.moveTo(PointF());
    m_path.cubicTo(bezier1 + bezier1Offset - tieThickness, bezier2 + bezier2Offset - tieThickness, tieEndNormalized);
    if (tie()->styleType() == SlurStyleType::Solid) {
        m_path.cubicTo(bezier2 + bezier2Offset + tieThickness, bezier1 + bezier1Offset + tieThickness, PointF());
    }

    tieThickness = PointF(0.0, 3.0 * m_midThickness);
    m_shapePath = PainterPath();
    m_shapePath.moveTo(PointF());
    m_shapePath.cubicTo(bezier1 + bezier1Offset - tieThickness, bezier2 + bezier2Offset - tieThickness, tieEndNormalized);
    m_shapePath.cubicTo(bezier2 + bezier2Offset + tieThickness, bezier1 + bezier1Offset + tieThickness, PointF());

    // translate back
    t.reset();
    t.translate(tieStart.x(), tieStart.y());
    t.rotateRadians(tieAngle);
    m_path = t.map(m_path);
    m_shapePath = t.map(m_shapePath);
    ups(Grip::BEZIER1).p = t.map(bezier1);
    ups(Grip::BEZIER2).p = t.map(bezier2);
    ups(Grip::END).p = t.map(tieEndNormalized) - ups(Grip::END).off;
    ups(Grip::DRAG).p = t.map(tieDrag);
    ups(Grip::SHOULDER).p = t.map(tieShoulder);

    Shape shape(Shape::Type::Composite);
    PointF start;
    start = t.map(start);

    double minH = std::abs(2 * m_midThickness);
    int nbShapes = 15;
    const CubicBezier b(tieStart, ups(Grip::BEZIER1).pos(), ups(Grip::BEZIER2).pos(), ups(Grip::END).pos());
    for (int i = 1; i <= nbShapes; i++) {
        const PointF point = b.pointAtPercent(i / float(nbShapes));
        RectF re = RectF(start, point).normalized();
        if (re.height() < minH) {
            tieLengthInSp = (minH - re.height()) * .5;
            re.adjust(0.0, -tieLengthInSp, 0.0, tieLengthInSp);
        }
        shape.add(re, this);
        start = point;
    }

    mutldata()->setShape(shape);
}

//---------------------------------------------------------
//   adjustY
//    adjust the y-position of the tie. this is called before adjustX()
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void TieSegment::adjustY(const PointF& p1, const PointF& p2)
{
    /*****************************************
     *            DEPRECATED
     * use SlurTieLayout::adjustY() instead
     ****************************************/
    m_autoAdjustOffset = PointF();
    const StaffType* staffType = this->staffType();
    bool useTablature = staffType->isTabStaff();
    Tie* t = toTie(slurTie());
    Chord* sc = t->startNote() ? t->startNote()->chord() : 0;

    if (!sc) {
        return; // don't adjust these ties vertically
    }
    double sp = spatium();
    const double ld = staff()->lineDistance(sc->tick()) * sp;
    const double lines = staff()->lines(sc->tick());
    const int line = t->startNote()->line();
    double tieAdjustSp = 0;

    const double staffLineOffset = 0.110 + (styleP(Sid::staffLineWidth) / 2 / ld); // sp
    const double noteHeadOffset = 0.185; // sp
    bool isUp = t->up();

    setPos(PointF());

    //Adjust Y pos to staff type offset before other calculations
    if (staffType) {
        mutldata()->moveY(staffType->yoffset().val() * spatium());
    }

    if (isNudged() || isEdited()) {
        return;
    }
    if (!t->isInside()) {
        setAutoAdjust(PointF(0, noteHeadOffset * spatium() * (slurTie()->up() ? -1 : 1)));
    }
    RectF bbox;
    if (p1.y() == p2.y()) {
#if 0
        // for horizontal ties we can estimate the bbox using simple math instead of having to call
        // computeBezier() which uses a whole lot of trigonometry to draw the entire tie
        bbox.setX(p1.x());
        bbox.setWidth(p2.x() - p1.x());

        // The following is ripped from computeBezier()
        // TODO: refactor this into its own method
        double shoulderHeight = bbox.width() * 0.4 * 0.38;
        shoulderHeight = qBound(shoulderHeightMin * spatium(), shoulderHeight, shoulderHeightMax * spatium());
        //////////
        double actualHeight = 3 * (shoulderHeight + styleP(Sid::SlurMidWidth)) / 4;

        bbox.setY(p1.y() - (slurTie()->up() ? actualHeight : 0));
        bbox.setHeight(actualHeight);
#else
        // more correct, less efficient
        computeBezier();
        bbox = m_path.boundingRect();
#endif
    } else {
        // don't adjust ties that aren't horizontal, just add offset
        return;
    }

    auto spansBarline = [staffLineOffset, lines](double a, double b) {
        if (b < a) {
            std::swap(a, b);
        }
        if (b < -staffLineOffset || a > (lines - 1) + staffLineOffset) {
            return false;
        }
        if (a < -staffLineOffset && b > staffLineOffset) {
            // a and b straddle line zero
            return true;
        }
        if (floor(a - staffLineOffset) != floor(b + staffLineOffset)) {
            return true;
        }
        return false;
    };
    Chord* ec = t->endNote() ? t->endNote()->chord() : 0;
    double staffDistance = 0.;
    if (sc && sc->staffMove() != 0 && ec->vStaffIdx() == sc->vStaffIdx()) {
        staffDistance = system()->staff(sc->vStaffIdx())->y() - system()->staff(staffIdx())->y();
    }

    double endpointYsp = (bbox.y() + (isUp ? bbox.height() : 0) - staffDistance) / ld;
    double tieHeightSp = bbox.height() / ld;
    double tieThicknessSp = (styleP(Sid::SlurMidWidth) + ((styleP(Sid::SlurMidWidth) - styleP(Sid::SlurEndWidth)) / 2)) / ld;
    double tieMidOutsideSp = endpointYsp + (isUp ? -tieHeightSp : tieHeightSp);
    double tieMidInsideSp = tieMidOutsideSp + (isUp ? (tieThicknessSp) : -(tieThicknessSp));
    if (useTablature && t->isInside()) {
        const double tieEndpointOffsetSp = 0.2;
        Note* sn = tie()->startNote();
        int string = sn->string();
        m_shoulderHeightMax = 4 / 3; // at max ties will be 1sp tall
        double newAnchor = (double)string;
        newAnchor += tieEndpointOffsetSp * (isUp ? -1 : 1);
        setAutoAdjust(PointF(0, (newAnchor - endpointYsp) * ld));
    } else if (!t->isInside()) {
        // OUTSIDE TIES

        double endpointYLineDist = endpointYsp - floor(endpointYsp);

        // ENDPOINTS ////////////////////////////////
        // If the endpoints are less than staffLineOffset from a line, they need to be adjusted
        // in the direction of the tie.
        double newAnchor = endpointYsp;
        bool farAdjust = false;
        if ((isUp && endpointYsp > -staffLineOffset) || (!isUp && endpointYsp < (lines - 1) + staffLineOffset)) {
            if (isUp) {
                if (endpointYLineDist < staffLineOffset) {
                    newAnchor = floor(endpointYsp) - staffLineOffset;
                    farAdjust = true;
                } else if (endpointYLineDist > (1 - staffLineOffset)) {
                    newAnchor = ceil(endpointYsp) - staffLineOffset;
                }
            } else { // down
                if (endpointYLineDist < staffLineOffset) {
                    newAnchor = floor(endpointYsp) + staffLineOffset;
                } else if (endpointYLineDist > (1 - staffLineOffset)) {
                    newAnchor = ceil(endpointYsp) + staffLineOffset;
                    farAdjust = true;
                }
            }
            tieAdjustSp += newAnchor - endpointYsp;
            tieMidOutsideSp += tieAdjustSp;
            tieMidInsideSp += tieAdjustSp;

            // TIE APOGEE ///////////////////////////////
            // If the middle of the tie conflicts with a staff line, the tie must be adjusted to resolve
            // that collision.
            if (farAdjust) {
                // we've already adjusted the tie pretty far from the notehead, so let's just
                // constrain the tie height to fit within a single space
                if (endpointYsp + tieAdjustSp > 0 && endpointYsp + tieAdjustSp < lines - 1) {
                    m_shoulderHeightMax = 4 * (1 - ((staffLineOffset * 2) + (tieThicknessSp / 2))) / 3;
                }
            } else {
                if (spansBarline(tieMidOutsideSp, tieMidInsideSp)) {
                    newAnchor = tieMidInsideSp;
                    if (isUp) {
                        newAnchor = floor(tieMidInsideSp + staffLineOffset) - staffLineOffset;
                    } else { // down
                        newAnchor = ceil(tieMidInsideSp - staffLineOffset) + staffLineOffset;
                    }
                    tieAdjustSp += newAnchor - tieMidInsideSp;
                    // we've adjusted the midpoint, but maybe the endpoint is too close to a barline now
                    double newEndpoint = endpointYsp + tieAdjustSp;
                    newAnchor = newEndpoint;
                    if (isUp && newEndpoint - floor(newEndpoint + staffLineOffset) < staffLineOffset) {
                        // clamp endpoint and adjust tie height
                        newAnchor = floor(newEndpoint + staffLineOffset) + staffLineOffset;
                        m_shoulderHeightMin = 4 * (staffLineOffset * 2 + (tieThicknessSp / 2)) / 3;
                        m_shoulderHeightMin *= (ld / spatium()); // shoulderHeightMin and Max are in spatium units, not line distance
                        m_shoulderHeightMax = m_shoulderHeightMin;
                    } else if (!isUp && ceil(newEndpoint - staffLineOffset) - newEndpoint < staffLineOffset) {
                        // clamp endpoint and adjust tie height
                        newAnchor = ceil(newEndpoint - staffLineOffset) - staffLineOffset;
                        m_shoulderHeightMin = 4 * (staffLineOffset * 2 + (tieThicknessSp / 2)) / 3;
                        m_shoulderHeightMin *= (ld / spatium());
                        m_shoulderHeightMax = m_shoulderHeightMin;
                    }
                    tieAdjustSp += newAnchor - newEndpoint;
                }
            }
        }
        setAutoAdjust(PointF(0, (tieAdjustSp * ld) - (p1.y() - (endpointYsp * ld + staffDistance))));
    } else {
        // INSIDE TIES (non-tab)
        bool collideAbove = false;
        bool collideBelow = false;
        Note* sn = tie()->startNote();
        Chord* sc2 = sn->chord();

        // figure out if there are situations where a tie collides with a tie above or below it
        // in a chord
        for (Note* note : sc2->notes()) {
            if (note == sn || !note->tieFor()) {
                continue;
            }
            if (note->line() == sn->line() - 1 && t->up() == note->tieFor()->up()) {
                collideAbove = true;
            }
            if (note->line() == sn->line() + 1 && t->up() == note->tieFor()->up()) {
                collideBelow = true;
            }
        }
        m_shoulderHeightMax = 4 / 3; // at max ties will be 1sp tall

        // ENDPOINTS ////////////////////////////////
        // Each line position in the staff has a set endpoint Y location
        double newAnchor;
        if (isUp) {
            newAnchor = floor(line / 2.0) + (line & 1 ? staffLineOffset : -staffLineOffset);
        } else {
            newAnchor = floor((line + 1) / 2.0) + (line & 1 ? -staffLineOffset : staffLineOffset);
        }

        // are the endpoints within the staff?
        bool endpointsInStaff = endpointYsp >= -staffLineOffset && endpointYsp <= (lines - 1) + staffLineOffset;
        // are the endpoints outside, but still close enough to need height adjustment?
        int lastLine = (lines - 1) * 2;
        bool downTieAbove = !isUp && line >= -2 && line < 0;
        bool upTieBelow = isUp && line > lastLine && line <= lastLine + 2;

        if (endpointsInStaff || downTieAbove || upTieBelow) {
            // TIE APOGEE ///////////////////////////////
            // Constrain tie height to avoid staff line collisions
            if (line & 1) {
                // tie endpoint is right below the line, so let's adjust the height so that the top clears the line
                m_shoulderHeightMin = 4 * ((staffLineOffset * 2) + (tieThicknessSp / 2)) / 3;
                m_shoulderHeightMin *= (ld / spatium());
            } else {
                // avoid collisions with the next line up by constraining maximum
                m_shoulderHeightMax = 4 * (1 - ((staffLineOffset * 2) + tieThicknessSp / 2)) / 3;
                m_shoulderHeightMax *= (ld / spatium());
            }
            if ((isUp && collideBelow) || (!isUp && collideAbove)) {
                m_shoulderHeightMin = 4 * ((staffLineOffset * 2) + (tieThicknessSp / 2)) / 3;
                m_shoulderHeightMin *= (ld / spatium());
            }
            if ((isUp && collideAbove && newAnchor > staffLineOffset)
                || (!isUp && collideBelow && newAnchor < (lines - 1))) {
                m_shoulderHeightMax = 4 * (1 - (staffLineOffset * 2) - (tieThicknessSp / 2)) / 3;
                m_shoulderHeightMax *= (ld / spatium());
            }
        }
        setAutoAdjust(PointF(0, (newAnchor - endpointYsp) * ld));
    }
}

//---------------------------------------------------------
//   finalizeSegment
//    compute the bezier and adjust the bbox for the curve
//---------------------------------------------------------

void TieSegment::finalizeSegment()
{
    /*****************************************
     *            DEPRECATED
     ****************************************/
    computeBezier();
    setbbox(m_path.boundingRect());
}

//---------------------------------------------------------
//   adjustX
//    adjust the tie endpoints to avoid staff lines. call adjustY() first!
//---------------------------------------------------------

void TieSegment::adjustX()
{
    /*****************************************
     *            DEPRECATED
     * use SlurTieLayout::adjustX() instead
     ****************************************/
    const bool adjustForHooks = false;
    double offsetMargin = spatium() * 0.25;
    double collisionYMargin = spatium() * 0.25;
    Note* sn = tie()->startNote();
    Note* en = tie()->endNote();
    Chord* sc = sn ? sn->chord() : nullptr;
    Chord* ec = en ? en->chord() : nullptr;

    double xo = 0;

    if (isNudged() || isEdited()) {
        return;
    }

    // ADJUST LEFT GRIP -----------
    if (sc && (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::BEGIN)) {
        // grips are in system coordinates, normalize to note position
        // (now generalized for the case of non-defined system (M.S.))
        PointF p1 = system() ? ups(Grip::START).p + PointF(system()->pos().x() - sn->canvasX() + sn->headWidth(), 0)
                    : ups(Grip::START).p + sn->posInStaffCoordinates();
        xo = 0;
        if (tie()->isInside()) {  // only adjust for inside-style ties
            // for cross-voice collisions, we need a list of all chords at this tick
            std::vector<Chord*> chords;
            track_idx_t strack = sc->staffIdx() * VOICES;
            track_idx_t etrack = sc->staffIdx() * VOICES + VOICES;
            chords.push_back(sc);
            for (track_idx_t track = strack; track < etrack; ++track) {
                if (Chord* ch = sc->measure()->findChord(sc->tick(), track)) {
                    const std::vector<Chord*>& graceNotes = ch->graceNotes();
                    if (ch != sc && std::find(graceNotes.begin(), graceNotes.end(), sc) == graceNotes.end()) {
                        chords.push_back(ch);
                    }
                }
            }

            for (Chord* chord : chords) {
                double chordOffset = chord->x() - sc->x() - sn->x() - sn->width(); // sn for right-offset notes, width() to normalize to zero
                // adjust for hooks
                if (chord->hook() && chord->hook()->visible() && adjustForHooks) {
                    double hookHeight = chord->hook()->ldata()->bbox().height();
                    // turn the hook upside down for downstems
                    double hookY = chord->hook()->pos().y() - (chord->up() ? 0 : hookHeight);
                    if (p1.y() > hookY - collisionYMargin && p1.y() < hookY + hookHeight + collisionYMargin) {
                        xo = std::max(xo, chord->hook()->x() + chord->hook()->width() + chordOffset);
                    }
                }

                // adjust for stems
                if (chord->stem() && chord->stem()->visible()) {
                    double stemLen = chord->stem()->ldata()->bbox().height();
                    double stemY = chord->stem()->pos().y() - (chord->up() ? stemLen : 0);
                    if (p1.y() > stemY - collisionYMargin && p1.y() < stemY + stemLen + collisionYMargin) {
                        xo = std::max(xo, chord->stem()->x() + chord->stem()->width() + chordOffset);
                    }
                }

                // adjust for ledger lines
                for (LedgerLine* currLedger = chord->ledgerLines(); currLedger; currLedger = currLedger->next()) {
                    // search through ledger lines and see if any are within .5sp of tie start
                    if (std::abs(p1.y() - currLedger->y()) < spatium() * 0.5) {
                        xo = std::max(xo, (currLedger->x() + currLedger->len() + chordOffset));
                        break;
                    }
                }

                for (auto note : chord->notes()) {
                    // adjust for dots
                    if (note->dots().size() > 0) {
                        double dotY = note->pos().y() + note->dots().back()->y();
                        if (std::abs(p1.y() - dotY) < spatium() * 0.5) {
                            xo = std::max(xo, note->x() + note->dots().back()->x() + note->dots().back()->width() + chordOffset);
                        }
                    }

                    // adjust for note collisions
                    if (note == sn) {
                        continue;
                    }
                    double noteTop = note->y() + note->ldata()->bbox().top();
                    double noteHeight = note->height();
                    if (p1.y() > noteTop - collisionYMargin && p1.y() < noteTop + noteHeight + collisionYMargin) {
                        xo = std::max(xo, note->x() + note->width() + chordOffset);
                    }
                }
            }
            xo += offsetMargin;
        } else { // tie is outside
            if ((slurTie()->up() && sc->up()) || (!slurTie()->up() && !sc->up())) {
                // outside ties may still require adjustment for hooks
                if (sc->hook() && sc->hook()->visible() && adjustForHooks) {
                    double hookHeight = sc->hook()->ldata()->bbox().height();
                    // turn the hook upside down for downstems
                    double hookY = sc->hook()->pos().y() - (sc->up() ? 0 : hookHeight);
                    if (p1.y() > hookY - collisionYMargin && p1.y() < hookY + hookHeight + collisionYMargin) {
                        double tieAttach = sn->outsideTieAttachX(slurTie()->up());
                        double hookOffsetX = sc->hook()->width() - (slurTie()->up() ? 0 : tieAttach);
                        xo = hookOffsetX + offsetMargin;
                    }
                } else if (sc->stem()) {
                    xo = offsetMargin;
                }
            } else if (sn->tieBack()) {
                xo += spatium() / 6; // 1/3 spatium in either direction, so .33/2
            }
        }
        xo *= sc->mag();
        ups(Grip::START).p += PointF(xo, 0);
    }

    // ADJUST RIGHT GRIP ----------
    if (ec && (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::END)) {
        // grips are in system coordinates, normalize to note position
        // (now generalized for the case of non-defined system (M.S.))
        PointF p2 = system() ? ups(Grip::END).p + PointF(system()->pos().x() - en->canvasX(), 0)
                    : ups(Grip::END).p + en->posInStaffCoordinates();
        xo = 0;
        if (tie()->isInside()) {
            // for inter-voice collisions, we need a list of all notes from all voices
            std::vector<Chord*> chords;
            track_idx_t strack = ec->staffIdx() * VOICES;
            track_idx_t etrack = ec->staffIdx() * VOICES + VOICES;
            chords.push_back(ec);
            if (!ec->isGraceAfter()) {
                for (track_idx_t track = strack; track < etrack; ++track) {
                    if (Chord* ch = ec->measure()->findChord(ec->tick(), track)) {
                        if (ch != ec) {
                            chords.push_back(ch);
                        }
                    }
                }
            }

            for (Chord* chord : chords) {
                double chordOffset = (ec->x() + en->x()) - chord->x(); // en->x() for right-offset notes
                for (LedgerLine* currLedger = chord->ledgerLines(); currLedger; currLedger = currLedger->next()) {
                    // search through ledger lines and see if any are within .5sp of tie end
                    if (std::abs(p2.y() - currLedger->y()) < spatium() * 0.5) {
                        xo = std::min(xo, currLedger->x() - chordOffset);
                    }
                }

                if (chord->stem() && chord->stem()->visible()) {
                    // adjust for stems
                    double stemLen = chord->stem()->ldata()->bbox().height();
                    double stemY = chord->stem()->pos().y() - (chord->up() ? stemLen : 0);
                    if (p2.y() > stemY - offsetMargin && p2.y() < stemY + stemLen + offsetMargin) {
                        xo = std::min(xo, chord->stem()->x() - chordOffset);
                    }
                }

                for (Note* note : chord->notes()) {
                    // adjust for accidentals
                    Accidental* acc = note->accidental();
                    if (acc && acc->visible()) {
                        double accTop = (note->y() + acc->y()) + acc->ldata()->bbox().top();
                        double accHeight = acc->height();
                        if (p2.y() >= accTop && p2.y() <= accTop + accHeight) {
                            xo = std::min(xo, note->x() + acc->x() - chordOffset);
                        }
                    }

                    if (note == en) {
                        continue;
                    }
                    // adjust for shifted notes (such as intervals of unison or second)
                    double noteTop = note->y() + note->ldata()->bbox().top();
                    double noteHeight = note->headHeight();
                    if (p2.y() >= noteTop - collisionYMargin && p2.y() <= noteTop + noteHeight + collisionYMargin) {
                        xo = std::min(xo, note->x() - chordOffset);
                    }
                }
            }
            xo -= offsetMargin;
        } else {
            // tie is outside
            if (!tie()->up() && !ec->up() && ec->stem() && ec->stem()->visible()) {
                xo -= offsetMargin;
            } else if (en && en->tieFor()) {
                xo -= spatium() / 6;
            }
        }
        xo *= ec->mag();
        ups(Grip::END).p += PointF(xo, 0);
    }
}

void TieSegment::consolidateAdjustmentOffsetIntoUserOffset()
{
    for (size_t i = 0; i < m_adjustmentOffsets.size(); ++i) {
        Grip grip = static_cast<Grip>(i);
        PointF adjustOffset = m_adjustmentOffsets[i];
        if (!adjustOffset.isNull()) {
            ups(grip).p -= adjustOffset;
            ups(grip).off = adjustOffset;
        }
    }
    resetAdjustmentOffset();
}

//---------------------------------------------------------
//   setAutoAdjust
//---------------------------------------------------------

void TieSegment::setAutoAdjust(const PointF& offset)
{
    PointF diff = offset - m_autoAdjustOffset;
    if (!diff.isNull()) {
        m_path.translate(diff);
        m_shapePath.translate(diff);

        Shape sh = ldata()->shape();
        sh.translate(diff);
        mutldata()->setShape(sh);

        for (int i = 0; i < int(Grip::GRIPS); ++i) {
            m_ups[i].p += diff;
        }
        m_autoAdjustOffset = offset;
    }
}

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool TieSegment::isEdited() const
{
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        if (!m_ups[i].off.isNull()) {
            return true;
        }
    }
    return false;
}

void TieSegment::addLineAttachPoints()
{
    // Add tie attach point to start and end note
    Note* startNote = tie()->startNote();
    Note* endNote = tie()->endNote();
    if (startNote) {
        startNote->addLineAttachPoint(ups(Grip::START).pos(), tie());
    }
    if (endNote) {
        endNote->addLineAttachPoint(ups(Grip::END).pos(), tie());
    }
}

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(EngravingItem* parent)
    : SlurTie(ElementType::TIE, parent)
{
    setAnchor(Anchor::NOTE);
}

//---------------------------------------------------------
//   calculateDirection
//---------------------------------------------------------

static int compareNotesPos(const Note* n1, const Note* n2)
{
    if (n1->line() != n2->line() && !(n1->staffType()->isTabStaff())) {
        return n2->line() - n1->line();
    } else if (n1->string() != n2->string()) {
        return n2->string() - n1->string();
    } else {
        return n1->pitch() - n2->pitch();
    }
}

void Tie::calculateDirection()
{
    Chord* c1   = startNote()->chord();
    Chord* c2   = endNote()->chord();
    Measure* m1 = c1->measure();
    Measure* m2 = c2->measure();

    if (m_slurDirection == DirectionV::AUTO) {
        std::vector<Note*> notes = c1->notes();
        size_t n = notes.size();
        StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
        bool simpleException = st && st->isSimpleTabStaff();
        // if there are multiple voices, the tie direction goes on stem side
        if (m1->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks())) {
            m_up = simpleException ? isUpVoice(c1->voice()) : c1->up();
        } else if (m2->hasVoices(c2->staffIdx(), c2->tick(), c2->actualTicks())) {
            m_up = simpleException ? isUpVoice(c2->voice()) : c2->up();
        } else if (n == 1) {
            //
            // single note
            //
            if (c1->up() != c2->up()) {
                // if stem direction is mixed, always up
                m_up = true;
            } else {
                m_up = !c1->up();
            }
        } else {
            //
            // chords
            //
            // first, find pivot point in chord (below which all ties curve down and above which all ties curve up)
            Note* pivotPoint = nullptr;
            bool multiplePivots = false;
            for (size_t i = 0; i < n - 1; ++i) {
                if (!notes[i]->tieFor()) {
                    continue; // don't include notes that don't have ties
                }
                for (size_t j = i + 1; j < n; ++j) {
                    if (!notes[j]->tieFor()) {
                        continue;
                    }
                    int noteDiff = compareNotesPos(notes[i], notes[j]);
                    if (!multiplePivots && std::abs(noteDiff) <= 1) {
                        // TODO: Fix unison ties somehow--if noteDiff == 0 then we need to determine which of the unison is 'lower'
                        if (pivotPoint) {
                            multiplePivots = true;
                            pivotPoint = nullptr;
                        } else {
                            pivotPoint = noteDiff < 0 ? notes[i] : notes[j];
                        }
                    }
                }
            }
            if (!pivotPoint) {
                // if the pivot point was not found (either there are no unisons/seconds or there are more than one),
                // determine if this note is in the lower or upper half of this chord
                int notesAbove = 0, tiesAbove = 0;
                int notesBelow = 0, tiesBelow = 0;
                int unisonTies = 0;
                for (size_t i = 0; i < n; ++i) {
                    if (notes[i] == startNote()) {
                        // skip counting if this note is the current note or if this note doesn't have a tie
                        continue;
                    }
                    int noteDiff = compareNotesPos(startNote(), notes[i]);
                    if (noteDiff == 0) {  // unison
                        if (notes[i]->tieFor()) {
                            unisonTies++;
                        }
                    }
                    if (noteDiff < 0) { // the note is above startNote
                        notesAbove++;
                        if (notes[i]->tieFor()) {
                            tiesAbove++;
                        }
                    }
                    if (noteDiff > 0) { // the note is below startNote
                        notesBelow++;
                        if (notes[i]->tieFor()) {
                            tiesBelow++;
                        }
                    }
                }

                if (tiesAbove == 0 && tiesBelow == 0 && unisonTies == 0) {
                    // this is the only tie in the chord.
                    if (notesAbove == notesBelow) {
                        m_up = !c1->up();
                    } else {
                        m_up = (notesAbove < notesBelow);
                    }
                } else if (tiesAbove == tiesBelow) {
                    // this note is dead center, so its tie should go counter to the stem direction
                    m_up = !c1->up();
                } else {
                    m_up = (tiesAbove < tiesBelow);
                }
            } else if (pivotPoint == startNote()) {
                // the current note is the lower of the only second or unison in the chord; tie goes down.
                m_up = false;
            } else {
                // if lower than the pivot, tie goes down, otherwise up
                int noteDiff = compareNotesPos(startNote(), pivotPoint);
                m_up = (noteDiff >= 0);
            }
        }
    } else {
        m_up = m_slurDirection == DirectionV::UP ? true : false;
    }
}

void Tie::calculateIsInside()
{
    if (_tiePlacement != TiePlacement::AUTO) {
        setIsInside(_tiePlacement == TiePlacement::INSIDE);
        return;
    }

    const Note* startN = startNote();
    const Chord* startChord = startN ? startN->chord() : nullptr;
    const Note* endN = endNote();
    const Chord* endChord = endN ? endN->chord() : nullptr;

    if (!startChord || !endChord) {
        setIsInside(false);
        return;
    }

    const bool startIsSingleNote = startChord->notes().size() <= 1;
    const bool endIsSingleNote = endChord->notes().size() <= 1;

    if (startIsSingleNote && endIsSingleNote) {
        setIsInside(style().styleV(Sid::tiePlacementSingleNote).value<TiePlacement>() == TiePlacement::INSIDE);
    } else {
        setIsInside(style().styleV(Sid::tiePlacementChord).value<TiePlacement>() == TiePlacement::INSIDE);
    }
}

PropertyValue Tie::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TIE_PLACEMENT:
        return tiePlacement();
    default:
        return SlurTie::getProperty(propertyId);
    }
}

PropertyValue Tie::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TIE_PLACEMENT:
        return TiePlacement::AUTO;
    default:
        return SlurTie::propertyDefault(id);
    }
}

bool Tie::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::TIE_PLACEMENT:
        setTiePlacement(v.value<TiePlacement>());
        break;
    default:
        return SlurTie::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Tie::setStartNote(Note* note)
{
    setStartElement(note);
    setParent(note);
}

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

Note* Tie::startNote() const
{
    assert(!startElement() || startElement()->type() == ElementType::NOTE);
    return toNote(startElement());
}

//---------------------------------------------------------
//   endNote
//---------------------------------------------------------

Note* Tie::endNote() const
{
    return toNote(endElement());
}

bool Tie::isOuterTieOfChord(Grip startOrEnd) const
{
    if (m_isInside) {
        return false;
    }

    const bool start = startOrEnd == Grip::START;
    const Note* note = start ? startNote() : endNote();
    if (!note) {
        return false;
    }

    const Chord* chord = note->chord();

    return (note == chord->upNote() && up()) || (note == chord->downNote() && !up());
}

bool Tie::hasTiedSecondInside() const
{
    const Note* note = startNote();
    if (!note) {
        return false;
    }

    const Chord* chord = note->chord();
    const int line = note->line();
    const int secondInsideLine = up() ? line + 1 : line - 1;

    for (const Note* otherNote : chord->notes()) {
        if (otherNote->line() == secondInsideLine && otherNote->tieFor() && otherNote->tieFor()->up() == up()) {
            return true;
        }
    }

    return false;
}

bool Tie::isCrossStaff() const
{
    const Note* startN = startNote();
    const Note* endN = endNote();

    return (startN && startN->chord()->staffMove() != 0) || (endN && endN->chord()->staffMove() != 0);
}
}
