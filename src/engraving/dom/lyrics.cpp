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

#include "lyrics.h"

#include "types/translatablestring.h"

#include "measure.h"
#include "navigate.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "text.h"
#include "textedit.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   lyricsElementStyle
//---------------------------------------------------------

static const ElementStyle lyricsElementStyle {
    { Sid::lyricsPlacement, Pid::PLACEMENT },
    { Sid::lyricsAvoidBarlines, Pid::AVOID_BARLINES },
};

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(ChordRest* parent)
    : TextBase(ElementType::LYRICS, parent, TextStyleType::LYRICS_ODD)
{
    m_separator  = 0;
    initElementStyle(&lyricsElementStyle);
    m_no         = 0;
    m_ticks      = Fraction(0, 1);
    m_syllabic   = LyricsSyllabic::SINGLE;
}

Lyrics::Lyrics(const Lyrics& l)
    : TextBase(l)
{
    m_no        = l.m_no;
    m_ticks     = l.m_ticks;
    m_syllabic  = l.m_syllabic;
    m_separator = 0;
}

Lyrics::~Lyrics()
{
    if (m_separator) {
        remove(m_separator);
    }
}

TranslatableString Lyrics::subtypeUserName() const
{
    return TranslatableString("engraving", "Verse %1").arg(m_no + 1);
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(EngravingItem* el)
{
    LOGD("Lyrics::add: unknown element %s", el->typeName());
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(EngravingItem* el)
{
    if (el->isLyricsLine()) {
        // only if separator still exists and is the right one
        if (m_separator && el == m_separator) {
            // Lyrics::remove() and LyricsLine::removeUnmanaged() call each other;
            // be sure each finds a clean context
            LyricsLine* separ = m_separator;
            m_separator = 0;
            separ->resetExplicitParent();
            separ->removeUnmanaged();
        }
    } else {
        LOGD("Lyrics::remove: unknown element %s", el->typeName());
    }
}

//---------------------------------------------------------
//   isMelisma
//---------------------------------------------------------

bool Lyrics::isMelisma() const
{
    // entered as melisma using underscore?
    if (m_ticks > Fraction(0, 1)) {
        return true;
    }

    // hyphenated?
    // if so, it is a melisma only if there is no lyric in same verse on next CR
    if (m_separator && (m_syllabic == LyricsSyllabic::BEGIN || m_syllabic == LyricsSyllabic::MIDDLE)) {
        // find next CR and check for existence of lyric in same verse and placement (in any voice)
        const ChordRest* cr = chordRest();
        if (cr) {
            const Segment* s = cr->segment()->next1();
            const track_idx_t strack = staffIdx() * VOICES;
            const track_idx_t etrack = strack + VOICES;
            const track_idx_t lyrTrack = track();
            const ChordRest* lyrVoiceNextCR = s ? s->nextChordRest(lyrTrack) : nullptr;
            for (track_idx_t track = strack; track < etrack; ++track) {
                const ChordRest* trackNextCR = s ? s->nextChordRest(track) : nullptr;
                if (trackNextCR) {
                    if (lyrTrack != track && lyrVoiceNextCR
                        && !lyrVoiceNextCR->lyrics(m_no, placement()) && lyrVoiceNextCR->tick() < trackNextCR->tick()) {
                        // There is an intermediary note in a different voice, this is a melisma
                        return true;
                    }
                    if (trackNextCR->lyrics(m_no, placement())) {
                        // Next note has lyrics, not a melisma just a dash
                        return false;
                    }
                }
            }
            return true;
        }
    }

    // default - not a melisma
    return false;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Lyrics::scanElements(void* data, void (* func)(void*, EngravingItem*), bool /*all*/)
{
    func(data, this);
    /* DO NOT ADD EITHER THE LYRICSLINE OR THE SEGMENTS: segments are added through the system each belongs to;
      LyricsLine is not needed, as it is internally managed.
      if (_separator)
            _separator->scanElements(data, func, all); */
}

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void Lyrics::paste(EditData& ed, const String& txt)
{
    if (txt.startsWith('<') && txt.contains('>')) {
        TextBase::paste(ed, txt);
        return;
    }

    String regex = String(u"[^\\S") + Char(0xa0) + Char(0x202F) + u"]+";
    StringList sl = txt.split(std::regex(regex.toStdString()), muse::SkipEmptyParts);
    if (sl.empty()) {
        return;
    }

    StringList hyph = sl.at(0).split(u'-');
    score()->startCmd(TranslatableString("undoableAction", "Paste lyrics"));

    deleteSelectedText(ed);

    if (hyph.size() > 1) {
        score()->undo(new InsertText(cursorFromEditData(ed), hyph[0]), &ed);
        hyph.removeAt(0);
        sl[0] =  hyph.join(u"-");
    } else if (sl.size() > 1 && sl[1] == u"-") {
        score()->undo(new InsertText(cursorFromEditData(ed), sl[0]), &ed);
        sl.removeAt(0);
        sl.removeAt(0);
    } else if (sl[0].startsWith(u"_")) {
        sl[0].remove(0, 1);
        if (sl[0].isEmpty()) {
            sl.removeAt(0);
        }
    } else if (sl[0].contains(u"_")) {
        size_t p = sl[0].indexOf(u'_');
        score()->undo(new InsertText(cursorFromEditData(ed), sl[0]), &ed);
        sl[0] = sl[0].mid(p + 1);
        if (sl[0].isEmpty()) {
            sl.removeAt(0);
        }
    } else if (sl.size() > 1 && sl[1] == "_") {
        score()->undo(new InsertText(cursorFromEditData(ed), sl[0]), &ed);
        sl.removeAt(0);
        sl.removeAt(0);
    } else {
        score()->undo(new InsertText(cursorFromEditData(ed), sl[0]), &ed);
        sl.removeAt(0);
    }

    score()->endCmd();
}

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction Lyrics::endTick() const
{
    return segment()->tick() + ticks();
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Lyrics::acceptDrop(EditData& data) const
{
    return data.dropElement->isText() || TextBase::acceptDrop(data);
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Lyrics::drop(EditData& data)
{
    ElementType type = data.dropElement->type();
    if (type == ElementType::SYMBOL || type == ElementType::FSYMBOL) {
        TextBase::drop(data);
        return 0;
    }
    if (!data.dropElement->isText()) {
        delete data.dropElement;
        data.dropElement = 0;
        return 0;
    }
    Text* e = toText(data.dropElement);
    e->setParent(this);
    score()->undoAddElement(e);
    return e;
}

bool Lyrics::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    static const std::set<KeyboardModifiers> navigationModifiers {
        NoModifier,
        KeypadModifier,
        ShiftModifier
    };

    if (navigationModifiers.find(ed.modifiers) != navigationModifiers.end()) {
        static const std::set<int> navigationKeys {
            Key_Underscore,
            Key_Minus,
            Key_Enter,
            Key_Return,
            Key_Up,
            Key_Down
        };

        if (navigationKeys.find(ed.key) != navigationKeys.end()) {
            return false;
        }
    }

    if (ed.key == Key_Left) {
        return cursor()->column() != 0 || cursor()->hasSelection();
    }

    if (ed.key == Key_Right) {
        bool cursorInLastColumn = cursor()->column() == cursor()->curLine().columns();
        return !cursorInLastColumn || cursor()->hasSelection();
    }

    return TextBase::isEditAllowed(ed);
}

void Lyrics::adjustPrevious()
{
    Lyrics* prev = prevLyrics(toLyrics(this));
    if (prev) {
        // search for lyric spanners to split at this point if necessary
        if (prev->tick() + prev->ticks() >= tick()) {
            // the previous lyric has a spanner attached that goes through this one
            // we need to shorten it
            Segment* s = score()->tick2segment(tick());
            if (s) {
                s = s->prev1(SegmentType::ChordRest);
                if (s->tick() > prev->tick()) {
                    prev->undoChangeProperty(Pid::LYRIC_TICKS, s->tick() - prev->tick());
                } else {
                    prev->undoChangeProperty(Pid::LYRIC_TICKS, Fraction::fromTicks(1));
                }
                prev->setNeedRemoveInvalidSegments();
                prev->triggerLayout();
            }
        }
    }
}

void Lyrics::setNeedRemoveInvalidSegments()
{
    // Allow "invalid" segments when there is a following repeat item

    const Measure* meas = measure();
    const ChordRest* separatorEndChord = m_separator ? toChordRest(m_separator->endElement()) : nullptr;
    const ChordRest* lastChordRest = meas ? meas->lastChordRest(track()) : nullptr;
    const bool endChordIsLastInMeasure = separatorEndChord == lastChordRest;
    const bool hasFollowingJump = lastChordRest ? lastChordRest->hasFollowingJumpItem() : false;

    if (endChordIsLastInMeasure && hasFollowingJump) {
        return;
    }
    m_needRemoveInvalidSegments = true;
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Lyrics::endEdit(EditData& ed)
{
    TextBase::endEdit(ed);

    triggerLayout();
    if (m_separator) {
        m_separator->triggerLayout();
    }
}

//---------------------------------------------------------
//   removeFromScore
//---------------------------------------------------------

void Lyrics::removeFromScore()
{
    if (m_ticks.isNotZero()) {
        // clear melismaEnd flag from end cr
        ChordRest* ecr = score()->findCR(endTick(), track());
        if (ecr) {
            ecr->setMelismaEnd(false);
        }
    }

    if (!plainText().isEmpty()) {
        for (auto sp : score()->spannerMap().findOverlapping(tick().ticks(), tick().ticks())) {
            if (!sp.value->isPartialLyricsLine() || sp.value->track() != track()) {
                continue;
            }
            PartialLyricsLine* partialLine = toPartialLyricsLine(sp.value);
            if (partialLine->isEndMelisma() || partialLine->no() != no() || partialLine->placement() != placement()) {
                continue;
            }
            score()->undoRemoveElement(partialLine);
        }
    }

    if (m_separator) {
        m_separator->removeUnmanaged();
        delete m_separator;
        m_separator = 0;
    }
    Lyrics* prev = prevLyrics(this);
    if (prev) {
        // check to make sure we haven't created an invalid segment by deleting this lyric
        prev->setNeedRemoveInvalidSegments();
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Lyrics::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYLLABIC:
        return int(m_syllabic);
    case Pid::LYRIC_TICKS:
        return m_ticks;
    case Pid::VERSE:
        return m_no;
    case Pid::AVOID_BARLINES:
        return m_avoidBarlines;
    default:
        return TextBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lyrics::setProperty(Pid propertyId, const PropertyValue& v)
{
    ChordRest* scr = nullptr;
    ChordRest* ecr = nullptr;

    switch (propertyId) {
    case Pid::PLACEMENT:
    {
        PlacementV newVal = v.value<PlacementV>();
        if (newVal != placement()) {
            if (Lyrics* l = prevLyrics(this)) {
                l->setNeedRemoveInvalidSegments();
            }
            if (nextLyrics(this)) {
                setNeedRemoveInvalidSegments();
            }
            setPlacement(newVal);
        }
    }
    break;
    case Pid::SYLLABIC:
        m_syllabic = LyricsSyllabic(v.toInt());
        break;
    case Pid::LYRIC_TICKS:
        if (m_ticks.isNotZero()) {
            // clear melismaEnd flag from previous end cr
            // this might be premature, as there may be other melismas ending there
            // but flag will be generated correctly on layout
            // TODO: after inserting a measure,
            // endTick info is wrong.
            // Somehow we need to fix this.
            // See https://musescore.org/en/node/285304 and https://musescore.org/en/node/311289
            ecr = score()->findCR(endTick(), track());
            if (ecr) {
                ecr->setMelismaEnd(false);
            }
        }
        scr = score()->findCR(tick(), track());
        m_ticks = v.value<Fraction>();
        if (scr && m_ticks <= scr->ticks()) {
            // if no ticks, we have to relayout in order to remove invalid melisma segments
            setNeedRemoveInvalidSegments();
        }
        break;
    case Pid::VERSE:
        if (Lyrics* l = prevLyrics(this)) {
            l->setNeedRemoveInvalidSegments();
        }
        m_no = v.toInt();
        break;
    case Pid::AVOID_BARLINES:
        m_avoidBarlines = v.toBool();
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Lyrics::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD;
    case Pid::PLACEMENT:
        return style().styleV(Sid::lyricsPlacement);
    case Pid::SYLLABIC:
        return int(LyricsSyllabic::SINGLE);
    case Pid::LYRIC_TICKS:
        return Fraction(0, 1);
    case Pid::VERSE:
        return 0;
    case Pid::AVOID_BARLINES:
        return style().styleB(Sid::lyricsAvoidBarlines);
    case Pid::ALIGN:
        if (isMelisma()) {
            return style().styleV(Sid::lyricsMelismaAlign);
        }
    // fall through
    default:
        return TextBase::propertyDefault(id);
    }
}

void Lyrics::triggerLayout() const
{
    if (m_separator) {
        // The separator may extend to next system(s), so we must use Spanner::triggerLayout()
        m_separator->triggerLayout();
    } else {
        // In this case is ok to use EngravingItem::triggerLayout()
        EngravingItem::triggerLayout();
    }
}

double Lyrics::yRelativeToStaff() const
{
    const double yOff = staffOffsetY();
    return pos().y() + chordRest()->pos().y() + yOff;
}

void Lyrics::setYRelativeToStaff(double y)
{
    const double yOff = staffOffsetY();
    mutldata()->setPosY(y - chordRest()->pos().y() - yOff);
}

//---------------------------------------------------------
//   forAllLyrics
//---------------------------------------------------------

void Score::forAllLyrics(std::function<void(Lyrics*)> f)
{
    for (Segment* s = firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->elist()) {
            if (e) {
                for (Lyrics* l : toChordRest(e)->lyrics()) {
                    f(l);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Lyrics::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::VERSE && no() != v.toInt()) {
        for (Lyrics* l : chordRest()->lyrics()) {
            if (l->no() == v.toInt()) {
                // verse already exists, swap
                l->TextBase::undoChangeProperty(id, no(), ps);
                PlacementV p = l->placement();
                l->TextBase::undoChangeProperty(Pid::PLACEMENT, int(placement()), ps);
                TextBase::undoChangeProperty(Pid::PLACEMENT, int(p), ps);
                break;
            }
        }
        TextBase::undoChangeProperty(id, v, ps);
        return;
    }

    TextBase::undoChangeProperty(id, v, ps);
}

//---------------------------------------------------------
//   removeInvalidSegments
//
// Remove lyric-final melisma lines and reset the alignment of the lyric
//---------------------------------------------------------

void Lyrics::removeInvalidSegments()
{
    m_needRemoveInvalidSegments = false;
    if (m_separator && isMelisma() && m_ticks < m_separator->startCR()->ticks()) {
        setTicks(Fraction(0, 1));
        m_separator->setTicks(Fraction(0, 1));
        m_separator->removeUnmanaged();
        m_separator = nullptr;
        setAlign(propertyDefault(Pid::ALIGN).value<Align>());
        if (m_syllabic == LyricsSyllabic::BEGIN || m_syllabic == LyricsSyllabic::SINGLE) {
            undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::SINGLE));
        } else {
            undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
        }
    }
}
}
