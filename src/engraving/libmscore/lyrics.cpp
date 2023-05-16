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

#include "lyrics.h"

#include "translation.h"
#include "types/translatablestring.h"

#include "layout/tlayout.h"

#include "measure.h"
#include "mscoreview.h"
#include "navigate.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "textedit.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   lyricsElementStyle
//---------------------------------------------------------

static const ElementStyle lyricsElementStyle {
    { Sid::lyricsPlacement, Pid::PLACEMENT },
};

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(ChordRest* parent)
    : TextBase(ElementType::LYRICS, parent, TextStyleType::LYRICS_ODD)
{
    _even       = false;
    _separator  = 0;
    initElementStyle(&lyricsElementStyle);
    _no         = 0;
    _ticks      = Fraction(0, 1);
    _syllabic   = LyricsSyllabic::SINGLE;
}

Lyrics::Lyrics(const Lyrics& l)
    : TextBase(l)
{
    _even      = l._even;
    _no        = l._no;
    _ticks     = l._ticks;
    _syllabic  = l._syllabic;
    _separator = 0;
}

Lyrics::~Lyrics()
{
    if (_separator) {
        remove(_separator);
    }
}

TranslatableString Lyrics::subtypeUserName() const
{
    return TranslatableString("engraving", "Verse %1").arg(_no + 1);
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(EngravingItem* el)
{
//      el->setParent(this);
//      if (el->type() == ElementType::LINE)
//            _separator.append((Line*)el);           // ignore! Internally managed
//            ;
//      else
    LOGD("Lyrics::add: unknown element %s", el->typeName());
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(EngravingItem* el)
{
    if (el->isLyricsLine()) {
        // only if separator still exists and is the right one
        if (_separator && el == _separator) {
            // Lyrics::remove() and LyricsLine::removeUnmanaged() call each other;
            // be sure each finds a clean context
            LyricsLine* separ = _separator;
            _separator = 0;
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
    if (_ticks > Fraction(0, 1)) {
        return true;
    }

    // hyphenated?
    // if so, it is a melisma only if there is no lyric in same verse on next CR
    if (_separator && (_syllabic == LyricsSyllabic::BEGIN || _syllabic == LyricsSyllabic::MIDDLE)) {
        // find next CR on same track and check for existence of lyric in same verse
        ChordRest* cr = chordRest();
        if (cr) {
            Segment* s = cr->segment()->next1();
            ChordRest* ncr = s ? s->nextChordRest(cr->track()) : 0;
            if (ncr && !ncr->lyrics(_no, placement())) {
                return true;
            }
        }
    }

    // default - not a melisma
    return false;
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//    - does not touch vertical position
//---------------------------------------------------------

void Lyrics::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
=======
>>>>>>> cd79de8b507ce5e52931bbfbce650f3fc04e0ae2
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
//   layout2
//    compute vertical position
//---------------------------------------------------------

void Lyrics::layout2(int nAbove)
{
    double lh = lineSpacing() * score()->styleD(Sid::lyricsLineHeight);

    if (placeBelow()) {
        double yo = segment()->measure()->system()->staff(staffIdx())->bbox().height();
        setPosY(lh * (_no - nAbove) + yo - chordRest()->y());
        movePos(styleValue(Pid::OFFSET, Sid::lyricsPosBelow).value<PointF>());
    } else {
        setPosY(-lh * (nAbove - _no - 1) - chordRest()->y());
        movePos(styleValue(Pid::OFFSET, Sid::lyricsPosAbove).value<PointF>());
    }
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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD

=======
>>>>>>> a4ec30994b... fix #13215 - support syllable-paste lyrics paste
=======
=======
>>>>>>> cd79de8b507ce5e52931bbfbce650f3fc04e0ae2

>>>>>>> 733d5aa1c2... Delete pasted text from clipboard
    String regex = String(u"[^\\S") + Char(0xa0) + Char(0x202F) + u"]+";
    StringList sl = txt.split(std::regex(regex.toStdString()), mu::SkipEmptyParts);
    if (sl.empty()) {
        return;
    }

    StringList hyph = sl.at(0).split(u'-');
    score()->startCmd();

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

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Lyrics::endEdit(EditData& ed)
{
    TextBase::endEdit(ed);
    triggerLayoutAll();
}

//---------------------------------------------------------
//   removeFromScore
//---------------------------------------------------------

void Lyrics::removeFromScore()
{
    if (_ticks.isNotZero()) {
        // clear melismaEnd flag from end cr
        ChordRest* ecr = score()->findCR(endTick(), track());
        if (ecr) {
            ecr->setMelismaEnd(false);
        }
    }

    if (_separator) {
        _separator->removeUnmanaged();
        delete _separator;
        _separator = 0;
    }
    Lyrics* prev = prevLyrics(this);
    if (prev) {
        // check to make sure we haven't created an invalid segment by deleting this lyric
        prev->setRemoveInvalidSegments();
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Lyrics::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYLLABIC:
        return int(_syllabic);
    case Pid::LYRIC_TICKS:
        return _ticks;
    case Pid::VERSE:
        return _no;
    default:
        return TextBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lyrics::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::PLACEMENT:
        setPlacement(v.value<PlacementV>());
        break;
    case Pid::SYLLABIC:
        _syllabic = LyricsSyllabic(v.toInt());
        break;
    case Pid::LYRIC_TICKS:
        if (_ticks.isNotZero()) {
            // clear melismaEnd flag from previous end cr
            // this might be premature, as there may be other melismas ending there
            // but flag will be generated correctly on layout
            // TODO: after inserting a measure,
            // endTick info is wrong.
            // Somehow we need to fix this.
            // See https://musescore.org/en/node/285304 and https://musescore.org/en/node/311289
            ChordRest* ecr = score()->findCR(endTick(), track());
            if (ecr) {
                ecr->setMelismaEnd(false);
            }
        }

        _ticks = v.value<Fraction>();
        if (_ticks <= Fraction(0, 1)) {
            // if no ticks, we have to relayout in order to remove invalid melisma segments
            setRemoveInvalidSegments();
            LayoutContext ctx(score());
            v0::TLayout::layout(this, ctx);
        }
        break;
    case Pid::VERSE:
        _no = v.toInt();
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
        return score()->styleV(Sid::lyricsPlacement);
    case Pid::SYLLABIC:
        return int(LyricsSyllabic::SINGLE);
    case Pid::LYRIC_TICKS:
        return Fraction(0, 1);
    case Pid::VERSE:
        return 0;
    case Pid::ALIGN:
        if (isMelisma()) {
            return score()->styleV(Sid::lyricsMelismaAlign);
        }
    // fall through
    default:
        return TextBase::propertyDefault(id);
    }
}

void Lyrics::triggerLayout() const
{
    if (_separator) {
        // The separator may extend to next system(s), so we must use Spanner::triggerLayout()
        _separator->triggerLayout();
    } else {
        // In this case is ok to use EngravingItem::triggerLayout()
        EngravingItem::triggerLayout();
    }
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
    } else if (id == Pid::AUTOPLACE && v.toBool() != autoplace()) {
        if (v.toBool()) {
            // setting autoplace
            // reset offset
            undoResetProperty(Pid::OFFSET);
        } else {
            // unsetting autoplace
            // rebase offset
            PointF off = offset();
            double y = pos().y() - propertyDefault(Pid::OFFSET).value<PointF>().y();
            off.ry() = placeAbove() ? y : y - staff()->height();
            undoChangeProperty(Pid::OFFSET, off, PropertyFlags::UNSTYLED);
        }
        TextBase::undoChangeProperty(id, v, ps);
        return;
    }

    TextBase::undoChangeProperty(id, v, ps);
}

KerningType Lyrics::doComputeKerningType(const EngravingItem* nextItem) const
{
    if (nextItem->isLyrics() || nextItem->isBarLine()) {
        return KerningType::NON_KERNING;
    }
    return KerningType::KERNING;
}

//---------------------------------------------------------
//   removeInvalidSegments
//
// Remove lyric-final melisma lines and reset the alignment of the lyric
//---------------------------------------------------------

void Lyrics::removeInvalidSegments()
{
    _removeInvalidSegments = false;
    if (_separator && isMelisma() && _ticks < _separator->startCR()->ticks()) {
        setTicks(Fraction(0, 1));
        _separator->removeUnmanaged();
        delete _separator;
        _separator = nullptr;
        setAlign(propertyDefault(Pid::ALIGN).value<Align>());
        if (_syllabic == LyricsSyllabic::BEGIN || _syllabic == LyricsSyllabic::SINGLE) {
            _syllabic = LyricsSyllabic::SINGLE;
        } else {
            _syllabic = LyricsSyllabic::END;
        }
    }
}
}
