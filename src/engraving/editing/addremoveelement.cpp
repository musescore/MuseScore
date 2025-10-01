/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "addremoveelement.h"

#include "containers.h"

#include "../dom/arpeggio.h"
#include "../dom/chord.h"
#include "../dom/engravingitem.h"
#include "../dom/keysig.h"
#include "../dom/linkedobjects.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/spanner.h"
#include "../dom/staff.h"
#include "../dom/system.h"
#include "../dom/tempotext.h"
#include "../dom/tie.h"
#include "../dom/tremolotwochord.h"
#include "../dom/tuplet.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   undoRemoveTuplet
//---------------------------------------------------------

static void undoRemoveTuplet(DurationElement* cr)
{
    if (cr->tuplet()) {
        cr->tuplet()->remove(cr);
        if (cr->tuplet()->elements().empty()) {
            undoRemoveTuplet(cr->tuplet());
        }
    }
}

//---------------------------------------------------------
//   undoAddTuplet
//---------------------------------------------------------

static void undoAddTuplet(DurationElement* cr)
{
    if (cr->tuplet()) {
        cr->tuplet()->add(cr);
        if (cr->tuplet()->elements().size() == 1) {
            undoAddTuplet(cr->tuplet());
        }
    }
}

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

AddElement::AddElement(EngravingItem* e)
{
    DO_ASSERT_X(!e->generated(), String(u"Generated item %1 passed to AddElement").arg(String::fromAscii(e->typeName())));
    element = e;
}

//---------------------------------------------------------
//   AddElement::cleanup
//---------------------------------------------------------

void AddElement::cleanup(bool undo)
{
    if (!undo) {
        delete element;
        element = nullptr;
    }
}

//---------------------------------------------------------
//   endUndoRedo
//---------------------------------------------------------

void AddElement::endUndoRedo(bool isUndo) const
{
    if (element->isChordRest()) {
        if (isUndo) {
            undoRemoveTuplet(toChordRest(element));
        } else {
            undoAddTuplet(toChordRest(element));
        }
    } else if (element->isClef()) {
        element->triggerLayout();
        element->score()->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        element->triggerLayout();
        element->score()->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void AddElement::undo(EditData*)
{
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->removeElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    }

    endUndoRedo(true);
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo(EditData*)
{
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->addElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    }

    endUndoRedo(false);
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* AddElement::name() const
{
    static char buffer[64];
    if (element->isTextBase()) {
        snprintf(buffer, 64, "Add:    %s <%s> %p", element->typeName(),
                 muPrintable(toTextBase(element)->plainText()), element);
    } else if (element->isSegment()) {
        snprintf(buffer, 64, "Add:    <%s-%s> %p", element->typeName(), toSegment(element)->subTypeName(), element);
    } else {
        snprintf(buffer, 64, "Add:    <%s> %p", element->typeName(), element);
    }
    return buffer;
}

//---------------------------------------------------------
//   AddElement::isFiltered
//---------------------------------------------------------

bool AddElement::isFiltered(UndoCommand::Filter f, const EngravingItem* target) const
{
    using Filter = UndoCommand::Filter;
    switch (f) {
    case Filter::AddElement:
        return target == element;
    case Filter::AddElementLinked:
        return muse::contains(target->linkList(), static_cast<EngravingObject*>(element));
    default:
        break;
    }
    return false;
}

std::vector<EngravingObject*> AddElement::objectItems() const
{
    return compoundObjects(element);
}

//---------------------------------------------------------
//   removeNote
//    Helper function for RemoveElement class
//---------------------------------------------------------

static void removeNote(const Note* note)
{
    Score* score = note->score();
    Tie* tieFor = note->tieFor();
    Tie* tieBack = note->tieBack();
    if (tieFor && tieFor->endNote()) {
        score->doUndoRemoveElement(tieFor);
    }
    if (tieBack) {
        if (tieBack->tieJumpPoints() && tieBack->tieJumpPoints()->size() > 1) {
            Tie::changeTieType(tieBack);
        } else {
            score->doUndoRemoveElement(tieBack);
        }
    }
    for (Spanner* s : note->spannerBack()) {
        score->doUndoRemoveElement(s);
    }
    for (Spanner* s : note->spannerFor()) {
        score->doUndoRemoveElement(s);
    }
}

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(EngravingItem* e)
{
    DO_ASSERT_X(!e->generated(), String(u"Generated item %1 passed to RemoveElement").arg(String::fromAscii(e->typeName())));
    element = e;

    Score* score = element->score();
    if (element->isChordRest()) {
        ChordRest* cr = toChordRest(element);
        if (cr->tuplet() && cr->tuplet()->elements().size() <= 1) {
            score->doUndoRemoveElement(cr->tuplet());
        }
        if (e->isChord()) {
            Chord* chord = toChord(e);
            // remove tremolo between 2 notes
            if (chord->tremoloTwoChord()) {
                TremoloTwoChord* tremolo = chord->tremoloTwoChord();
                score->doUndoRemoveElement(tremolo);
            }
            // Move arpeggio down to next available note
            if (chord->arpeggio()) {
                chord->arpeggio()->rebaseStartAnchor(AnchorRebaseDirection::DOWN);
            } else {
                // If this chord is the end of an arpeggio, move the end of the arpeggio upwards to the next available chord
                Arpeggio* spanArp = chord->spanArpeggio();
                if (spanArp && chord->track() == spanArp->endTrack()) {
                    spanArp->rebaseEndAnchor(AnchorRebaseDirection::UP);
                }
            }
            for (const Note* note : chord->notes()) {
                removeNote(note);
            }
        }
    } else if (element->isNote()) {
        // Removing an individual note within a chord
        const Note* note = toNote(element);
        removeNote(note);
    }
}

//---------------------------------------------------------
//   RemoveElement::cleanup
//---------------------------------------------------------

void RemoveElement::cleanup(bool undo)
{
    if (undo) {
        delete element;
        element = nullptr;
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void RemoveElement::undo(EditData*)
{
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->addElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    } else if (element->isChordRest()) {
        if (element->isChord()) {
            Chord* chord = toChord(element);
            for (Note* note : chord->notes()) {
                note->connectTiedNotes();
            }
        }
        undoAddTuplet(toChordRest(element));
    } else if (element->isClef()) {
        score->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        score->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo(EditData*)
{
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->removeElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    } else if (element->isChordRest()) {
        undoRemoveTuplet(toChordRest(element));
        if (element->isChord()) {
            Chord* chord = toChord(element);
            for (Note* note : chord->notes()) {
                note->disconnectTiedNotes();
            }
        }
    } else if (element->isClef()) {
        score->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        score->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* RemoveElement::name() const
{
    static char buffer[64];
    if (element->isTextBase()) {
        snprintf(buffer, 64, "Remove: %s <%s> %p", element->typeName(),
                 muPrintable(toTextBase(element)->plainText()), element);
    } else if (element->isSegment()) {
        snprintf(buffer, 64, "Remove: <%s-%s> %p", element->typeName(), toSegment(element)->subTypeName(), element);
    } else {
        snprintf(buffer, 64, "Remove: %s %p", element->typeName(), element);
    }
    return buffer;
}

//---------------------------------------------------------
//   RemoveElement::isFiltered
//---------------------------------------------------------

bool RemoveElement::isFiltered(UndoCommand::Filter f, const EngravingItem* target) const
{
    using Filter = UndoCommand::Filter;
    switch (f) {
    case Filter::RemoveElement:
        return target == element;
    case Filter::RemoveElementLinked:
        return muse::contains(target->linkList(), static_cast<EngravingObject*>(element));
    default:
        break;
    }
    return false;
}

std::vector<EngravingObject*> RemoveElement::objectItems() const
{
    return compoundObjects(element);
}

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

ChangeElement::ChangeElement(EngravingItem* oe, EngravingItem* ne)
{
    oldElement = oe;
    newElement = ne;
}

void ChangeElement::flip(EditData*)
{
    const LinkedObjects* links = oldElement->links();
    if (links) {
        newElement->linkTo(oldElement);
        oldElement->unlink();
    }

    newElement->setTrack(oldElement->track());
    if (newElement->isSpanner()) {
        toSpanner(newElement)->setTrack2(toSpanner(oldElement)->track2());
    }

    Score* score = oldElement->score();
    if (!score->selection().isRange()) {
        if (oldElement->selected()) {
            score->deselect(oldElement);
        }
        if (newElement->selected()) {
            score->select(newElement, SelectType::ADD);
        }
    }

    if (oldElement->explicitParent() == nullptr) {
        newElement->setScore(score);
        score->removeElement(oldElement);
        score->addElement(newElement);
    } else {
        oldElement->parentItem()->change(oldElement, newElement);
    }

    if (newElement->isKeySig()) {
        KeySig* ks = toKeySig(newElement);
        if (!ks->generated()) {
            ks->staff()->setKey(ks->tick(), ks->keySigEvent());
        }
    } else if (newElement->isTempoText()) {
        TempoText* t = toTempoText(oldElement);
        score->setTempo(t->segment(), t->tempo());
    }

    if (newElement->isSpannerSegment()) {
        SpannerSegment* os = toSpannerSegment(oldElement);
        SpannerSegment* ns = toSpannerSegment(newElement);
        if (os->system()) {
            os->system()->remove(os);
        }
        if (ns->system()) {
            ns->system()->add(ns);
        }
    }

    if (newElement->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(newElement), score);
    }

    std::swap(oldElement, newElement);
    oldElement->triggerLayout();
    newElement->triggerLayout();
}

//---------------------------------------------------------
//   ChangeParent
//---------------------------------------------------------

void ChangeParent::flip(EditData*)
{
    EngravingItem* p = element->parentItem();
    staff_idx_t si = element->staffIdx();
    p->remove(element);
    element->setParent(parent);
    element->setTrack(staffIdx * VOICES + element->voice());
    parent->add(element);
    staffIdx = si;
    parent = p;

    element->triggerLayout();
}

//---------------------------------------------------------
//   LinkUnlink
//---------------------------------------------------------

LinkUnlink::~LinkUnlink()
{
    if (le && mustDelete) {
        assert(le->size() <= 1);
        delete le;
    }
}

void LinkUnlink::link()
{
    if (le->size() == 1) {
        le->front()->setLinks(le);
    }
    mustDelete = false;
    le->push_back(e);
    e->setLinks(le);
}

void LinkUnlink::unlink()
{
    assert(le->contains(e));
    le->remove(e);
    if (le->size() == 1) {
        le->front()->setLinks(nullptr);
        mustDelete = true;
    }

    e->setLinks(nullptr);
}

//---------------------------------------------------------
//   Link
//    link e1 to e2
//---------------------------------------------------------

Link::Link(EngravingObject* e1, EngravingObject* e2)
{
    assert(e1->links() == nullptr);
    le = e2->links();
    if (!le) {
        le = new LinkedObjects();
        le->push_back(e2);
    }
    e = e1;
}

//---------------------------------------------------------
//   Link::isFiltered
//---------------------------------------------------------

bool Link::isFiltered(UndoCommand::Filter f, const EngravingItem* target) const
{
    using Filter = UndoCommand::Filter;
    if (f == Filter::Link) {
        return e == target || le->contains(const_cast<EngravingItem*>(target));
    }
    return false;
}

//---------------------------------------------------------
//   Unlink
//---------------------------------------------------------

Unlink::Unlink(EngravingObject* _e)
{
    e  = _e;
    le = e->links();
    assert(le);
}
