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

#include "stretchedbend.h"

#include "draw/fontmetrics.h"

#include "accidental.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "tie.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   textFlags
//---------------------------------------------------------

static const char* label[] = {
    "",     "\u00BC",   "\u00BD",   "\u00BE",  /// 0,   1/4, 1/2, 3/4
    "full", "1\u00BC", "1\u00BD", "1\u00BE",   /// 1, 1+1/4...
    "2",    "2\u00BC", "2\u00BD", "2\u00BE",   /// 2, ...
    "3"                                        /// 3
};

static const ElementStyle stretchedBendStyle {
    { Sid::bendFontFace,  Pid::FONT_FACE },
    { Sid::bendFontSize,  Pid::FONT_SIZE },
    { Sid::bendFontStyle, Pid::FONT_STYLE },
    { Sid::bendLineWidth, Pid::LINE_WIDTH },
};

//---------------------------------------------------------
//   forward declarations of static functions
//---------------------------------------------------------

static RectF textBoundingRect(const FontMetrics& fm, const PointF& pos, const String& text);
static int bendTone(int notePitch);
static std::pair<Note*, Note*> getLowestAndHighestNote(const std::vector<Note*>& notes);

//---------------------------------------------------------
//   static values
//---------------------------------------------------------

static constexpr double BEND_HEIGHT_MULTIPLIER = .2; /// how much height differs for bend pitches

//---------------------------------------------------------
//   StretchedBend
//---------------------------------------------------------

StretchedBend::StretchedBend(Chord* parent)
    : EngravingItem(ElementType::STRETCHED_BEND, parent, ElementFlag::MOVABLE)
{
    initElementStyle(&stretchedBendStyle);
}

//---------------------------------------------------------
//   font
//---------------------------------------------------------

Font StretchedBend::font(double sp) const
{
    Font f(_fontFace, Font::Type::Unknown);
    f.setBold(_fontStyle & FontStyle::Bold);
    f.setItalic(_fontStyle & FontStyle::Italic);
    f.setUnderline(_fontStyle & FontStyle::Underline);
    f.setStrike(_fontStyle & FontStyle::Strike);
    double m = _fontSize;
    m *= sp / SPATIUM20;

    f.setPointSizeF(m);
    return f;
}

int StretchedBend::textFlags()
{
    return muse::draw::AlignHCenter | muse::draw::AlignBottom | muse::draw::TextDontClip;
}

String StretchedBend::toneToLabel(int tone)
{
    return String::fromUtf8(label[tone]);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue StretchedBend::getProperty(Pid id) const
{
    switch (id) {
    case Pid::FONT_FACE:
        return _fontFace;
    case Pid::FONT_SIZE:
        return _fontSize;
    case Pid::FONT_STYLE:
        return int(_fontStyle);
    case Pid::LINE_WIDTH:
        return _lineWidth;
    default:
        return EngravingItem::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool StretchedBend::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::FONT_FACE:
        _fontFace = v.value<String>();
        break;
    case Pid::FONT_SIZE:
        _fontSize = v.toReal();
        break;
    case Pid::FONT_STYLE:
        _fontStyle = FontStyle(v.toInt());
        break;
    case Pid::LINE_WIDTH:
        _lineWidth = v.value<Spatium>();
        break;
    default:
        return EngravingItem::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

StretchedBend::BendSegment::BendSegment()
{
}

StretchedBend::BendSegment::BendSegment(BendSegmentType bendType, int bendTone)
    : type(bendType), tone(bendTone)
{
}

void StretchedBend::BendSegment::setupCoords(PointF s, PointF d)
{
    src = s;
    dest = d;
}

void StretchedBend::addSegment(std::vector<StretchedBend::BendSegment>& bendSegments, StretchedBend::BendSegmentType type, int tone) const
{
    bendSegments.emplace_back(type, tone);
}

//---------------------------------------------------------
//   createBendSegments
//---------------------------------------------------------

void StretchedBend::createBendSegments()
{
    m_bendSegments.clear();

    if (m_pitchValues.size() < 2) {
        return;
    }

    IF_ASSERT_FAILED(m_note) {
        LOGE() << "note is not set";
        return;
    }

    bool isPrevBendUp = false;
    bool skipFirstPoint = firstPointShouldBeSkipped();

    BendSegmentType prevLineType = BendSegmentType::NO_TYPE;

    for (size_t pt = 0; pt < m_pitchValues.size() - 1; pt++) {
        int pitch = m_pitchValues.at(pt).pitch;
        int nextPitch = m_pitchValues.at(pt + 1).pitch;

        BendSegmentType type = BendSegmentType::NO_TYPE;

        /// PRE-BEND (+BEND, +RELEASE)
        if (pt == 0 && pitch != 0) {
            if (!skipFirstPoint) {
                addSegment(m_bendSegments, BendSegmentType::LINE_UP, bendTone(pitch));
            }
        }

        /// PRE-BEND - - -
        if (pitch == nextPitch) {
            if (pt == 0) {
                type = BendSegmentType::LINE_STROKED;
            }
        } else {
            bool bendUp = pitch < nextPitch;
            if (bendUp && isPrevBendUp && pt > 0) {
                // prevent double bendUp rendering
                int prevBendPitch = m_pitchValues.at(pt - 1).pitch;
                // remove prev segment if there are two bendup in a row
                if (!m_bendSegments.empty()) {
                    m_bendSegments.pop_back();
                    if (prevBendPitch > 0 && prevBendPitch < pitch && !m_bendSegments.empty()) {
                        m_bendSegments.back().tone = bendTone(0);
                    }
                }
            }

            isPrevBendUp = bendUp;
            type = (bendUp ? BendSegmentType::CURVE_UP : BendSegmentType::CURVE_DOWN);
        }

        // prevent double curve down rendering
        if (prevLineType == BendSegmentType::CURVE_DOWN && type == BendSegmentType::CURVE_DOWN) {
            continue;
        }

        if (type != BendSegmentType::NO_TYPE) {
            addSegment(m_bendSegments, type, bendTone(nextPitch));
        }

        prevLineType = type;
    }
}

//---------------------------------------------------------
//   fillSegments
//---------------------------------------------------------

void StretchedBend::fillSegments()
{
    IF_ASSERT_FAILED(!m_bendSegments.empty()) {
        LOGE() << "bend segments are empty";
        return;
    }

    double noteWidth = m_note->width();
    double noteHeight = m_note->height();
    PointF notePos = m_note->pos();
    double sp = spatium();
    Staff* st = staff();
    PointF upBendDefaultOffset = PointF(noteWidth + sp * .8, 0);
    if (!m_note->dots().empty() && st && !st->isTabStaff(tick())) {
        /// moving bend to the right from dot
        size_t dotsSize = m_note->dots().size();
        upBendDefaultOffset
            += PointF(style().styleS(Sid::dotDotDistance).val() * sp + symWidth(SymId::augmentationDot) + style().styleS(
                          Sid::dotDotDistance).val() * sp * (dotsSize - 1), 0);
    }

    PointF downBendDefaultOffset = PointF(noteWidth * .5, -noteHeight * .5 - sp * .2);
    PointF src;

    if (StretchedBend* tiedBend = backTiedStretchedBend()) {
        src = PointF(.0, tiedBend->m_bendSegments.back().dest.y());
    } else {
        PointF upBendDefaultSrc = upBendDefaultOffset + notePos;
        PointF downBendDefaultSrc = downBendDefaultOffset + notePos;
        src = m_pitchValues.at(0).pitch == 0 ? upBendDefaultSrc : downBendDefaultSrc;
    }

    PointF dest(src);
    bool releasedToInitial = (0 == m_pitchValues.back().pitch);
    double baseBendHeight = sp * 1.5;

    for (size_t i = 0; i < m_bendSegments.size(); i++) {
        BendSegment& bendSegment = m_bendSegments.at(i);

        if (bendSegment.type == BendSegmentType::LINE_UP) {
            /// rewriting the source position for prebend, should go from highest note
            Chord* chord = toChord(parent());
            auto [lowestNote, highestNote] = getLowestAndHighestNote(chord->notes());
            src = highestNote->pos() + PointF(highestNote->width() / 2, -highestNote->height());

            double minY = std::min(.0, src.y());
            dest = PointF(src.x(), minY - bendHeight(bendSegment.tone) - baseBendHeight);

            /// shortening length of prebend on standard staff if chord has staccato
            Staff* st2 = staff();
            if (st2 && !st2->isTabStaff(tick())) {
                for (Articulation* art : chord->articulations()) {
                    if (art->isStaccato()) {
                        src.setY(src.y() - symWidth(art->symId()) - style().styleS(Sid::articulationMinDistance).val() * sp);
                        break;
                    }
                }
            }

            bendSegment.setupCoords(src, dest);
            src.setY(dest.y());
            continue;
        }

        if (bendSegment.type == BendSegmentType::CURVE_UP) {
            double minY = std::min(.0, src.y());
            dest.setY(minY - bendHeight(bendSegment.tone) - baseBendHeight);
        } else if (bendSegment.type == BendSegmentType::CURVE_DOWN) {
            if (releasedToInitial) {
                dest.setY(notePos.y());
            } else {
                int prevTone = ((i == 0) ? 0 : m_bendSegments.at(i - 1).tone);
                dest.setY(src.y() + bendHeight(prevTone) + baseBendHeight);
            }
        }

        bendSegment.setupCoords(src, dest);

        src = dest;
    }
}

//---------------------------------------------------------
//   getLowestAndHighestNote
//---------------------------------------------------------

std::pair<Note*, Note*> getLowestAndHighestNote(const std::vector<Note*>& notes)
{
    IF_ASSERT_FAILED(!notes.empty()) {
        return { nullptr, nullptr };
    }

    Note* note = notes.at(0);
    bool isTab = note->staff()->isTabStaff(note->tick());
    const auto [min, max] = std::minmax_element(notes.begin(), notes.end(), [isTab](Note* l, Note* r) {
        if (isTab) {
            return l->string() > r->string();
        }

        return l->pitch() < r->pitch();
    });

    return { *min, *max };
}

//---------------------------------------------------------
//   adjustBendInChord
//---------------------------------------------------------

void StretchedBend::adjustBendInChord()
{
    IF_ASSERT_FAILED(!m_bendSegments.empty()) {
        LOGE() << "invalid bend data";
        return;
    }

    if (!m_needsHeightAdjust) {
        return;
    }

    std::vector<Note*> bendNotes = notesWithStretchedBend(toChord(parent()));
    if (bendNotes.size() < 2) {
        return;
    }

    auto [lowestNote, highestNote] = getLowestAndHighestNote(bendNotes);
    m_noteToAdjust = highestNote;

    StretchedBend* upStretchedBend = m_noteToAdjust->stretchedBend();
    IF_ASSERT_FAILED(upStretchedBend) {
        LOGE() << "unable to adjust stretched bends heights: note to adjust is not found";
        return;
    }

    const std::vector<BendSegment>& upBendSegments = upStretchedBend->m_bendSegments;
    IF_ASSERT_FAILED(m_bendSegments.size() == upBendSegments.size()) {
        LOGE() << "unable to adjust stretched bends heights: bends of the chord are not of same type";
        return;
    }

    for (size_t i = 0; i < m_bendSegments.size(); i++) {
        BendSegment& bendSegment = m_bendSegments.at(i);
        const BendSegment& upBendSegment = upBendSegments.at(i);
        switch (bendSegment.type) {
        case BendSegmentType::LINE_UP:
            bendSegment.src = upBendSegment.src;
            bendSegment.dest = upBendSegment.dest;
            break;
        case BendSegmentType::CURVE_UP:
            if (m_pitchValues.at(0).pitch > 0) {
                bendSegment.src = upBendSegment.src;
            }

            bendSegment.dest = upBendSegment.dest;
            break;
        case BendSegmentType::CURVE_DOWN:
            bendSegment.src = upBendSegment.src;
            bendSegment.visible = lowestNote == m_note;
            break;
        case BendSegmentType::LINE_STROKED:
            bendSegment.src = upBendSegment.src;
            bendSegment.dest = upBendSegment.dest;
            break;
        case BendSegmentType::NO_TYPE:
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   fillStretchedSegments
//---------------------------------------------------------

void StretchedBend::fillStretchedSegments(bool untilNextSegment)
{
    if (m_bendSegments.empty()) {
        return;
    }

    double sp = spatium();

    StretchedBend* beginCoordBend = this;
    if (m_noteToAdjust) {
        beginCoordBend = m_noteToAdjust->stretchedBend();
    }

    double bendStart = beginCoordBend->m_bendSegments.at(0).src.x();
    double bendEnd = untilNextSegment ? nextSegmentX() : sp * 6;

    if (bendStart > bendEnd) {
        return;
    }

    m_bendSegmentsStretched = m_bendSegments;
    size_t segsSize = m_bendSegmentsStretched.size();

    m_bendSegmentsStretched.at(segsSize - 1).dest.setX(bendEnd);

    double step = (bendEnd - bendStart) / segsSize;

    for (auto i = segsSize - 1; i > 0; --i) {
        auto& lastSeg = m_bendSegmentsStretched.at(i);
        auto& prevSeg = m_bendSegmentsStretched.at(i - 1);
        if (lastSeg.type != BendSegmentType::LINE_UP && prevSeg.type != BendSegmentType::LINE_UP
            && lastSeg.type != BendSegmentType::LINE_STROKED) {
            bendEnd -= step;
            lastSeg.src.setX(bendEnd);
            prevSeg.dest.setX(bendEnd);
        }
    }

    if (!untilNextSegment) {
        return;
    }

    /// adjust coordinate to bend of tied back note
    StretchedBend* backTiedBend = backTiedStretchedBend();
    if (backTiedBend) {
        auto getSystem = [](Chord* ch) { return ch->measure()->system(); };
        auto getPageX = [](Chord* ch) { return ch->segment()->pageX(); };
        Chord* currentChord = toChord(parent());
        Chord* tiedBackChord = toChord(backTiedBend->parent());
        if (getSystem(currentChord) == getSystem(tiedBackChord)) {
            double newX = getPageX(currentChord) - getPageX(tiedBackChord);
            for (EngravingItem* item : tiedBackChord->el()) {
                if (item->isStretchedBend()) {
                    StretchedBend* bendToAdjust = toStretchedBend(item);
                    IF_ASSERT_FAILED(!bendToAdjust->m_bendSegmentsStretched.empty()) {
                        LOGE() << "wrong bend data while adjusting coordinates";
                        return;
                    }

                    PointF& tiedBendEndPoint = bendToAdjust->m_bendSegmentsStretched.back().dest;
                    tiedBendEndPoint.setX(newX);
                }
            }
        }
    }
}

RectF StretchedBend::calculateBoundingRect() const
{
    RectF bRect;
    double sp = spatium();
    bool isTextDrawn = false;

    for (const BendSegment& bendSegment : m_bendSegmentsStretched) {
        const PointF& src = bendSegment.src;
        const PointF& dest = bendSegment.dest;
        const String& text = String::fromUtf8(label[bendSegment.tone]);

        switch (bendSegment.type) {
        case BendSegmentType::LINE_UP:
        {
            bRect.unite(RectF(src.x(), src.y(), dest.x() - src.x(), dest.y() - src.y()));
            bRect.unite(m_arrows.up.translated(dest).boundingRect());

            FontMetrics fm(font(sp));
            bRect.unite(textBoundingRect(fm, dest, text));
            /// TODO: remove after fixing bRect
            bRect.setHeight(bRect.height() + sp);
            bRect.setY(bRect.y() - sp);
            break;
        }

        case BendSegmentType::CURVE_UP:
        case BendSegmentType::CURVE_DOWN:
        {
            bool bendUp = (bendSegment.type == BendSegmentType::CURVE_UP);
            double endY = dest.y() + m_arrows.width * (bendUp ? 1 : -1);

            PainterPath path = bendCurveFromPoints(src, PointF(dest.x(), endY));
            const auto& arrowPath = (bendUp ? m_arrows.up : m_arrows.down);

            bRect.unite(path.boundingRect());
            bRect.unite(arrowPath.translated(dest).boundingRect());

            if (bendUp && !isTextDrawn) {
                FontMetrics fm(font(sp));
                bRect.unite(textBoundingRect(fm, dest - PointF(sp, 0), text));
                /// TODO: remove after fixing bRect
                bRect.setHeight(bRect.height() + sp);
                bRect.setY(bRect.y() - sp);
                isTextDrawn = true;
            }

            break;
        }

        case BendSegmentType::LINE_STROKED:
        {
            bRect.unite(RectF(src.x(), src.y(), dest.x() - src.x(), dest.y() - src.y()));
            break;
        }

        default:
            break;
        }
    }

    double lw = absoluteFromSpatium(lineWidth());
    bRect.adjust(-lw, -lw, lw, lw);

    return bRect;
}

//---------------------------------------------------------
//   fillArrows
//---------------------------------------------------------

void StretchedBend::fillArrows(double width)
{
    if (m_arrows.width == width) {
        return;
    }

    double aw = 0;
    m_arrows.width = aw = width;

    m_arrows.up.clear();
    m_arrows.down.clear();

    m_arrows.up << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    m_arrows.down << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);
}

//---------------------------------------------------------
//   textBoundingRect
//---------------------------------------------------------

RectF textBoundingRect(const FontMetrics& fm, const PointF& pos, const String& text)
{
    return fm.boundingRect(RectF(pos.x(), pos.y(), 0, 0), StretchedBend::textFlags(), text);
}

//---------------------------------------------------------
//   bendCurveFromPoints
//---------------------------------------------------------

PainterPath StretchedBend::bendCurveFromPoints(const PointF& p1, const PointF& p2)
{
    PainterPath path;

    path.moveTo(p1.x(), p1.y());
    path.cubicTo(p1.x() + (p2.x() - p1.x()) / 2, p1.y(), p2.x(), p1.y() + (p2.y() - p1.y()) / 4, p2.x(), p2.y());

    return path;
}

//---------------------------------------------------------
//   nextSegmentX
//---------------------------------------------------------

double StretchedBend::nextSegmentX() const
{
    Segment* nextSeg = toChord(parent())->segment()->nextInStaff(
        staffIdx(), SegmentType::ChordRest | SegmentType::BarLine | SegmentType::EndBarLine);
    if (!nextSeg) {
        return 0;
    }

    double resultPosX = nextSeg->pagePos().x() - pagePos().x() - spatium();

    /// adjusting to accidentals
    if (nextSeg->segmentType() == SegmentType::ChordRest) {
        EngravingItem* item = nextSeg->element(track());
        if (item && item->isChord()) {
            Chord* chord = toChord(item);
            for (Note* note : chord->notes()) {
                if (Accidental* ac = note->accidental()) {
                    resultPosX -= (ac->width() + spatium());
                    break;
                }
            }
        }
    }

    return resultPosX;
}

//---------------------------------------------------------
//   bendTone
//---------------------------------------------------------

int bendTone(int notePitch)
{
    return (notePitch + 12) / 25;
}

//---------------------------------------------------------
//   bendHeight
//---------------------------------------------------------

double StretchedBend::bendHeight(int bendIdx) const
{
    return spatium() * (bendIdx + 1) * BEND_HEIGHT_MULTIPLIER;
}

//---------------------------------------------------------
//   notesWithStretchedBend
//---------------------------------------------------------

std::vector<Note*> StretchedBend::notesWithStretchedBend(Chord* chord)
{
    std::vector<Note*> bendNotes;

    const std::vector<Note*>& allNotes = chord->notes();
    std::copy_if(allNotes.begin(), allNotes.end(), std::back_inserter(bendNotes),
                 [](Note* note) { return note->stretchedBend(); });

    return bendNotes;
}

bool StretchedBend::equalBendTypes(const StretchedBend* bend1, const StretchedBend* bend2)
{
    const std::vector<BendSegment>& bendSegments1 = bend1->m_bendSegments;
    const std::vector<BendSegment>& bendSegments2 = bend2->m_bendSegments;

    if (bendSegments1.size() != bendSegments2.size()) {
        return false;
    }

    for (size_t i = 0; i < bendSegments1.size(); i++) {
        if (bendSegments1.at(i).type != bendSegments2.at(i).type) {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
//   prepareBends
//---------------------------------------------------------

void StretchedBend::prepareBends(std::vector<StretchedBend*>& strechedBends)
{
    std::unordered_map<Chord*, std::vector<StretchedBend*> > bendsInChords;
    for (StretchedBend* bend : strechedBends) {
        // filling segments after all ties are connected
        bend->createBendSegments();
        bendsInChords[toChord(bend->parent())].push_back(bend);
    }

    /// checking if height adjust is needed for bends in chords
    for (auto&[chord, bends] : bendsInChords) {
        std::vector<Note*> bendNotes = notesWithStretchedBend(chord);
        if (bendNotes.size() < 2) {
            continue;
        }

        Note* compareNote = bendNotes.at(0);

        bool updateHeightsNeeded = true;

        for (StretchedBend* bend : bends) {
            if (!equalBendTypes(compareNote->stretchedBend(), bend)) {
                updateHeightsNeeded = false;
                break;
            }
        }

        if (updateHeightsNeeded) {
            for (StretchedBend* bend : bends) {
                bend->m_needsHeightAdjust = true;
            }

            /// here to adjust the tone of particular segment
            for (size_t segIdx = 0; segIdx < bends.at(0)->m_bendSegments.size(); segIdx++) {
                int maxTone = BendSegment::NO_TONE;
                for (size_t bendIdx = 0; bendIdx < bends.size(); bendIdx++) {
                    const BendSegment& bendSegment = bends.at(bendIdx)->m_bendSegments.at(segIdx);
                    maxTone = std::max(bendSegment.tone, maxTone);
                }

                for (size_t bendIdx = 0; bendIdx < bends.size(); bendIdx++) {
                    BendSegment& bendSegment = bends.at(bendIdx)->m_bendSegments.at(segIdx);
                    bendSegment.tone = maxTone;
                }
            }
        }
    }
}

//---------------------------------------------------------
//   backTiedStretchedBend
//---------------------------------------------------------

StretchedBend* StretchedBend::backTiedStretchedBend() const
{
    if (Tie* tie = m_note->tieBack()) {
        Note* backTied = tie->startNote();
        return backTied ? backTied->stretchedBend() : nullptr;
    }

    return nullptr;
}

//---------------------------------------------------------
//   firstPointShouldBeSkipped
//---------------------------------------------------------

bool StretchedBend::firstPointShouldBeSkipped() const
{
    StretchedBend* lastStretchedBend = backTiedStretchedBend();
    if (lastStretchedBend) {
        const auto& lastBendPoints = lastStretchedBend->m_pitchValues;
        if (!lastBendPoints.empty() && lastBendPoints.back().pitch == m_pitchValues.at(0).pitch) {
            return true;
        }
    }

    return false;
}
}
