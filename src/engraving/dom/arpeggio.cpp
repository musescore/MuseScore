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

#include "arpeggio.h"

#include <cmath>

#include "containers.h"

#include "types/typesconv.h"

#include "accidental.h"
#include "chord.h"
#include "mscoreview.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "property.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

Arpeggio::Arpeggio(Chord* parent)
    : EngravingItem(ElementType::ARPEGGIO, parent, ElementFlag::MOVABLE)
{
    m_arpeggioType = ArpeggioType::NORMAL;
    m_span     = 1;
    m_userLen1 = 0.0;
    m_userLen2 = 0.0;
    m_playArpeggio = true;
    m_stretch = 1.0;

    parent->setSpanArpeggio(this);
}

Arpeggio::~Arpeggio()
{
    // Remove reference to this arpeggio in any chords it may have spanned
    Chord* _chord = chord();
    if (!_chord || !_chord->segment()) {
        return;
    }
    for (track_idx_t _track = track(); _track <= track() + m_span; _track++) {
        EngravingItem* e = _chord->segment()->element(_track);
        if (e && e->isChord() && toChord(e)->spanArpeggio() == this) {
            toChord(e)->setSpanArpeggio(nullptr);
        }
    }
}

const muse::TranslatableString& Arpeggio::arpeggioTypeName() const
{
    return TConv::userName(m_arpeggioType);
}

void Arpeggio::findAndAttachToChords()
{
    const track_idx_t strack = track();
    const Chord* _chord = chord();
    const Part* part = _chord->part();
    const track_idx_t btrack = part->endTrack();

    if (strack + m_span > btrack) {
        rebaseEndAnchor(AnchorRebaseDirection::UP);
    }

    const track_idx_t etrack = strack + (m_span - 1);
    track_idx_t lastTrack = strack;
    for (track_idx_t track = strack; track <= etrack; track++) {
        EngravingItem* e = _chord->segment()->element(track);
        if (e && e->isChord()) {
            toChord(e)->undoChangeSpanArpeggio(this);
            lastTrack = track;
        }
    }

    if (lastTrack != etrack) {
        track_idx_t newSpan = lastTrack - strack + 1;
        undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
    }
}

void Arpeggio::detachFromChords(track_idx_t strack, track_idx_t etrack)
{
    Chord* _chord = chord();
    if (!_chord) {
        return;
    }
    for (track_idx_t track = strack; track <= etrack; track++) {
        EngravingItem* e = _chord->segment()->element(track);
        if (e && e->isChord() && toChord(e)->spanArpeggio() == this) {
            toChord(e)->undoChangeSpanArpeggio(nullptr);
        }
    }
}

void Arpeggio::rebaseStartAnchor(AnchorRebaseDirection direction)
{
    if (direction == AnchorRebaseDirection::UP) {
        // Move arpeggio to chord above
        Staff* s = staff();
        Part* part = s->part();
        track_idx_t topTrack = part->startTrack();
        if (track() > topTrack) {
            // Loop through voices til we find a chord
            for (int curTrack = static_cast<int>(track()) - 1; curTrack >= static_cast<int>(topTrack); curTrack--) {
                EngravingItem* e = chord()->segment()->element(curTrack);
                if (e && e->isChord()) {
                    track_idx_t newSpan = m_span + track() - curTrack;
                    if (newSpan != 0) {
                        undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                        score()->undoChangeParent(this, e, e->staffIdx());
                        break;
                    }
                }
            }
        }
    } else if (direction == AnchorRebaseDirection::DOWN) {
        // Move arpeggio to chord below
        for (track_idx_t curTrack = track() + 1; curTrack <= track() + m_span - 1; curTrack++) {
            EngravingItem* e = chord()->segment()->element(curTrack);
            if (e && e->isChord()) {
                track_idx_t newSpan = m_span + track() - curTrack;
                if (newSpan != 0) {
                    chord()->undoChangeSpanArpeggio(nullptr);
                    undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                    score()->undoChangeParent(this, e, e->staffIdx());
                    break;
                }
            }
        }
    }
}

void Arpeggio::rebaseEndAnchor(AnchorRebaseDirection direction)
{
    if (direction == AnchorRebaseDirection::UP) {
        // Move end to chord above
        for (int curTrack = static_cast<int>(track()) + m_span - 2; curTrack >= static_cast<int>(track()); curTrack--) {
            EngravingItem* e = chord()->segment()->element(curTrack);
            if (e && e->isChord()) {
                track_idx_t newSpan = curTrack - track() + 1;
                if (newSpan != 0) {
                    EngravingItem* oldEndChord = chord()->segment()->element(endTrack());
                    if (oldEndChord && oldEndChord->isChord()) {
                        toChord(oldEndChord)->undoChangeSpanArpeggio(nullptr);
                    }
                    undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                }
                break;
            }
        }
    } else if (direction == AnchorRebaseDirection::DOWN) {
        // Move end to chord below
        Staff* s = staff();
        Part* part = s->part();
        track_idx_t btrack = part->endTrack();
        if (track() + m_span < btrack) {
            // Loop through voices til we find a chord
            for (track_idx_t curTrack = track() + m_span; curTrack < btrack; curTrack++) {
                EngravingItem* e = chord()->segment()->element(curTrack);
                if (e && e->isChord()) {
                    track_idx_t newSpan = curTrack - track() + 1;
                    toChord(e)->undoChangeSpanArpeggio(this);
                    undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                    break;
                }
            }
        }
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Arpeggio::gripsPositions(const EditData&) const
{
    const LayoutData* ldata = this->ldata();
    const PointF pp(pagePos());
    PointF p1(ldata->bbox().width() / 2, ldata->bbox().top());
    PointF p2(ldata->bbox().width() / 2, ldata->bbox().bottom());
    return { p1 + pp, p2 + pp };
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(EditData& ed)
{
    Part* p = part();
    double d = ed.delta.y();
    PointF pos = PointF(chord()->canvasPos().x(), ed.pos.y());
    EngravingItem* e = ed.view()->elementNear(pos);

    if (ed.curGrip == Grip::START) {
        m_userLen1 -= d;
        if (e && e->isNote() && e->part() == p) {
            Chord* c = toNote(e)->chord();
            int newSpan = std::max(1, m_span + int(track()) - int(c->track()));
            if (track() != c->track()) {
                // if new track is greater than old we have chords to unmark
                if (track() < c->track()) {
                    detachFromChords(track(), c->track() - 1);
                }
                undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                score()->undoChangeParent(this, c, c->staffIdx());
                m_userLen1 = 0.0;
            }
        }
    } else if (ed.curGrip == Grip::END) {
        m_userLen2 += d;
        // Increase span
        if (e && e->isNote() && e->part() == p) {
            int newSpan = std::max(1, int(e->track()) - int(track()) + 1);
            if (e->track() != endTrack()) {
                // if new endTrack is less than old we have chords to unmark
                if (e->track() < endTrack()) {
                    detachFromChords(e->track() + 1, endTrack());
                }
                undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                m_userLen2 = 0.0;
            }
        }
    }

    triggerLayout();
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> Arpeggio::dragAnchorLines() const
{
    std::vector<LineF> result;

    Chord* c = chord();
    if (c) {
        result.push_back(LineF(canvasPos(), c->upNote()->canvasPos()));
    }
    return std::vector<LineF>();
}

//---------------------------------------------------------
//   gripAnchorLines
//---------------------------------------------------------

std::vector<LineF> Arpeggio::gripAnchorLines(Grip grip) const
{
    std::vector<LineF> result;

    Chord* _chord = chord();
    if (!_chord) {
        return result;
    }

    const Page* p = toPage(findAncestor(ElementType::PAGE));
    const PointF pageOffset = p ? p->pos() : PointF();

    const PointF gripCanvasPos = gripsPositions()[static_cast<int>(grip)] + pageOffset;

    if (grip == Grip::START) {
        Note* upNote = _chord->upNote();
        EngravingItem* e = _chord->segment()->element(track());
        if (e && e->isChord()) {
            upNote = toChord(e)->upNote();
        }
        result.push_back(LineF(upNote->canvasPos(), gripCanvasPos));
    } else if (grip == Grip::END) {
        Note* downNote = _chord->downNote();
        EngravingItem* e = _chord->segment()->element(track() + m_span - 1);
        if (e && e->isChord()) {
            downNote = toChord(e)->downNote();
        }

        result.push_back(LineF(downNote->canvasPos(), gripCanvasPos));
    }
    return result;
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Arpeggio::startEdit(EditData& ed)
{
    EngravingItem::startEdit(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::ARP_USER_LEN1);
    eed->pushProperty(Pid::ARP_USER_LEN2);
}

void Arpeggio::startEditDrag(EditData& ed)
{
    EngravingItem::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }
    eed->pushProperty(Pid::ARP_USER_LEN1);
    eed->pushProperty(Pid::ARP_USER_LEN2);
}

bool Arpeggio::isEditAllowed(EditData& ed) const
{
    if ((ed.curGrip != Grip::END && ed.curGrip != Grip::START) || !(ed.modifiers & ShiftModifier)) {
        return false;
    }

    return ed.key == Key_Down || ed.key == Key_Up;
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Arpeggio::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    if (ed.curGrip == Grip::START) {
        if (ed.key == Key_Down) {
            rebaseStartAnchor(AnchorRebaseDirection::DOWN);
        } else if (ed.key == Key_Up) {
            rebaseStartAnchor(AnchorRebaseDirection::UP);
        }
    }

    if (ed.curGrip == Grip::END) {
        if (ed.key == Key_Down) {
            rebaseEndAnchor(AnchorRebaseDirection::DOWN);
        } else if (ed.key == Key_Up) {
            if (m_span > 1) {
                rebaseEndAnchor(AnchorRebaseDirection::UP);
            }
        }
    }

    triggerLayout();

    return true;
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Arpeggio::spatiumChanged(double oldValue, double newValue)
{
    m_userLen1 *= (newValue / oldValue);
    m_userLen2 *= (newValue / oldValue);
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Arpeggio::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::ARPEGGIO;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Arpeggio::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::ARPEGGIO:
    {
        Arpeggio* a = toArpeggio(e);
        if (explicitParent()) {
            score()->undoRemoveElement(this);
        }
        a->setTrack(track());
        a->setParent(explicitParent());
        score()->undoAddElement(a);
    }
        return e;
    default:
        delete e;
        break;
    }
    return 0;
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Arpeggio::reset()
{
    undoChangeProperty(Pid::ARP_USER_LEN1, 0.0);
    undoChangeProperty(Pid::ARP_USER_LEN2, 0.0);
    EngravingItem::reset();
}

bool Arpeggio::crossStaff() const
{
    return (track() + span() - 1) / VOICES != vStaffIdx();
}

staff_idx_t Arpeggio::vStaffIdx() const
{
    return chord() ? chord()->vStaffIdx() : EngravingItem::vStaffIdx();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

engraving::PropertyValue Arpeggio::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ARPEGGIO_TYPE:
        return int(m_arpeggioType);
    case Pid::TIME_STRETCH:
        return Stretch();
    case Pid::ARP_USER_LEN1:
        return userLen1();
    case Pid::ARP_USER_LEN2:
        return userLen2();
    case Pid::PLAY:
        return m_playArpeggio;
    case Pid::ARPEGGIO_SPAN:
        return m_span;
    default:
        break;
    }
    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Arpeggio::setProperty(Pid propertyId, const engraving::PropertyValue& val)
{
    switch (propertyId) {
    case Pid::ARPEGGIO_TYPE:
        setArpeggioType(ArpeggioType(val.toInt()));
        break;
    case Pid::TIME_STRETCH:
        setStretch(val.toDouble());
        break;
    case Pid::ARP_USER_LEN1:
        setUserLen1(val.toDouble());
        break;
    case Pid::ARP_USER_LEN2:
        setUserLen2(val.toDouble());
        break;
    case Pid::PLAY:
        setPlayArpeggio(val.toBool());
        break;
    case Pid::ARPEGGIO_SPAN:
        setSpan(val.toInt());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, val)) {
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

engraving::PropertyValue Arpeggio::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ARP_USER_LEN1:
        return 0.0;
    case Pid::ARP_USER_LEN2:
        return 0.0;
    case Pid::TIME_STRETCH:
        return 1.0;
    case Pid::PLAY:
        return true;
    case Pid::ARPEGGIO_SPAN:
        return 1;
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Arpeggio::subtypeUserName() const
{
    return arpeggioTypeName();
}
