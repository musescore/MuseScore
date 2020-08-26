//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "mu3paletteadapter.h"

#include "shortcut.h"
#include "musescore.h"
#include "scoreview.h"

#include "libmscore/chord.h"
#include "libmscore/chordrest.h"
#include "libmscore/segment.h"
#include "libmscore/spanner.h"
#include "libmscore/icon.h"
#include "libmscore/system.h"
#include "libmscore/staff.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"
#include "libmscore/page.h"
#include "libmscore/part.h"

using namespace Ms;

MU3PaletteAdapter::MU3PaletteAdapter()
{
    m_paletteEnabled.val = true;
}

QAction* MU3PaletteAdapter::getAction(const char* id) const
{
    return Shortcut::getActionByName(id);
}

QString MU3PaletteAdapter::actionHelp(const char* id) const
{
    Shortcut* sc = Shortcut::getShortcut(id);
    return sc ? sc->help() : QString();
}

void MU3PaletteAdapter::showMasterPalette(const QString& arg)
{
    mscore->showMasterPalette(arg);
}

void MU3PaletteAdapter::selectInstrument(Ms::InstrumentChange* i)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->selectInstrument(i);
    }
}

bool MU3PaletteAdapter::isSelected() const
{
    Score* score = mscore->currentScore();
    if (!score) {
        return false;
    }
    const Selection sel = score->selection();
    return !sel.isNone();
}

bool MU3PaletteAdapter::editMode() const
{
    auto view = mscore->currentScoreView();
    return view ? view->editMode() : false;
}

Ms::ScoreState MU3PaletteAdapter::mscoreState() const
{
    auto view = mscore->currentScoreView();
    return view ? view->mscoreState() : Ms::ScoreState::STATE_DISABLED;
}

void MU3PaletteAdapter::changeState(Ms::ViewState s)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->changeState(s);
    }
}

void MU3PaletteAdapter::cmdAddSlur(const Ms::Slur* slurTemplate)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->cmdAddSlur(slurTemplate);
    }
}

void MU3PaletteAdapter::applyDrop(Ms::Score* score, Ms::Element* target, Ms::Element* e,
                                  Qt::KeyboardModifiers modifiers,
                                  QPointF pt, bool pasteMode)
{
    ScoreView* viewer = mscore->currentScoreView();
    if (!viewer) {
        return;
    }

    EditData& dropData = viewer->getEditData();
    dropData.pos         = pt.isNull() ? target->pagePos() : pt;
    dropData.dragOffset  = QPointF();
    dropData.modifiers   = modifiers;
    dropData.dropElement = e;

    if (target->acceptDrop(dropData)) {
        // use same code path as drag&drop

        QByteArray a = e->mimeData(QPointF());
//printf("<<%s>>\n", a.data());

        XmlReader n(a);
        n.setPasteMode(pasteMode);
        Fraction duration;      // dummy
        QPointF dragOffset;
        ElementType type = Element::readType(n, &dragOffset, &duration);
        dropData.dropElement = Element::create(type, score);

        dropData.dropElement->read(n);
        dropData.dropElement->styleChanged();       // update to local style

        Element* el = target->drop(dropData);
        if (el && el->isInstrumentChange()) {
            selectInstrument(toInstrumentChange(el));
        }
        if (el && !viewer->noteEntryMode()) {
            score->select(el, SelectType::SINGLE, 0);
        }
        dropData.dropElement = 0;
    }
}

//! NOTE Copied from Palette::applyPaletteElement
bool MU3PaletteAdapter::applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers)
{
    Score* score = mscore->currentScore();
    if (score == 0) {
        return false;
    }
    const Selection sel = score->selection();   // make a copy of selection state before applying the operation.
    if (sel.isNone()) {
        return false;
    }

//       Element* element = 0;
//       if (cell)
//             element = cell->element.get();
    if (element == 0) {
        return false;
    }

    if (element->isSpanner()) {
        //TourHandler::startTour("spanner-drop-apply");
    }

#ifdef MSCORE_UNSTABLE
//    if (ScriptRecorder* rec = mscore->getScriptRecorder()) {
//        if (modifiers == 0) {
//            //rec->recordPaletteElement(element);
//        }
//    }
#endif

    // exit edit mode, to allow for palette element to be applied properly
    if (this->editMode() && !(this->mscoreState() & STATE_ALLTEXTUAL_EDIT)) {
        this->changeState(ViewState::NORMAL);
    }

    if (this->mscoreState() != STATE_EDIT
        && this->mscoreState() != STATE_LYRICS_EDIT
        && this->mscoreState() != STATE_HARMONY_FIGBASS_EDIT) {  // Already in startCmd in this case
        score->startCmd();
    }
    if (sel.isList()) {
        ChordRest* cr1 = sel.firstChordRest();
        ChordRest* cr2 = sel.lastChordRest();
        bool addSingle = false;           // add a single line only
        if (cr1 && cr2 == cr1) {
            // one chordrest selected, ok to add line
            addSingle = true;
        } else if (sel.elements().size() == 2 && cr1 && cr2 && cr1 != cr2) {
            // two chordrests selected
            // must be on same staff in order to add line, except for slur
            if (element->isSlur() || cr1->staffIdx() == cr2->staffIdx()) {
                addSingle = true;
            }
        }
        if (this->mscoreState() == STATE_NOTE_ENTRY_STAFF_DRUM && element->isChord()) {
            InputState& is = score->inputState();
            Element* e = nullptr;
            if (!(modifiers & Qt::ShiftModifier)) {
                // shift+double-click: add note to "chord"
                // use input position rather than selection if possible
                // look for a cr in the voice predefined for the drum in the palette
                // back up if necessary
                // TODO: refactor this with similar code in putNote()
                if (is.segment()) {
                    Segment* seg = is.segment();
                    while (seg) {
                        if (seg->element(is.track())) {
                            break;
                        }
                        seg = seg->prev(SegmentType::ChordRest);
                    }
                    if (seg) {
                        is.setSegment(seg);
                    } else {
                        is.setSegment(is.segment()->measure()->first(SegmentType::ChordRest));
                    }
                }
                score->expandVoice();
                e = is.cr();
            }
            if (!e) {
                e = sel.elements().first();
            }
            if (e) {
                // get note if selection was full chord
                if (e->isChord()) {
                    e = toChord(e)->upNote();
                }

                applyDrop(score, e, element, modifiers, QPointF(), true);
                // note has already been played (and what would play otherwise may be *next* input position)
                score->setPlayNote(false);
                score->setPlayChord(false);
                // continue in same track
                is.setTrack(e->track());
            } else {
                qDebug("nowhere to place drum note");
            }
        } else if (element->isLayoutBreak()) {
            LayoutBreak* breakElement = toLayoutBreak(element);
            score->cmdToggleLayoutBreak(breakElement->layoutBreakType());
        } else if (element->isSlur() && addSingle) {
            this->cmdAddSlur(toSlur(element));
        } else if (element->isSLine() && !element->isGlissando() && addSingle) {
            Segment* startSegment = cr1->segment();
            Segment* endSegment = cr2->segment();
            if (element->type() == ElementType::PEDAL && cr2 != cr1) {
                endSegment = endSegment->nextCR(cr2->track());
            }
            // TODO - handle cross-voice selections
            int idx = cr1->staffIdx();

            QByteArray a = element->mimeData(QPointF());
//printf("<<%s>>\n", a.data());
            XmlReader e(a);
            Fraction duration;        // dummy
            QPointF dragOffset;
            ElementType type = Element::readType(e, &dragOffset, &duration);
            Spanner* spanner = static_cast<Spanner*>(Element::create(type, score));
            spanner->read(e);
            spanner->styleChanged();
            score->cmdAddSpanner(spanner, idx, startSegment, endSegment);
        } else {
            for (Element* e : sel.elements()) {
                applyDrop(score, e, element, modifiers);
            }
        }
    } else if (sel.isRange()) {
        if (element->type() == ElementType::BAR_LINE
            || element->type() == ElementType::MARKER
            || element->type() == ElementType::JUMP
            || element->type() == ElementType::SPACER
            || element->type() == ElementType::VBOX
            || element->type() == ElementType::HBOX
            || element->type() == ElementType::TBOX
            || element->type() == ElementType::MEASURE
            || element->type() == ElementType::BRACKET
            || element->type() == ElementType::STAFFTYPE_CHANGE
            || (element->type() == ElementType::ICON
                && (toIcon(element)->iconType() == IconType::VFRAME
                    || toIcon(element)->iconType() == IconType::HFRAME
                    || toIcon(element)->iconType() == IconType::TFRAME
                    || toIcon(element)->iconType() == IconType::MEASURE
                    || toIcon(element)->iconType() == IconType::BRACKETS))) {
            Measure* last = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
            for (Measure* m = sel.startSegment()->measure(); m; m = m->nextMeasureMM()) {
                QRectF r = m->staffabbox(sel.staffStart());
                QPointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                pt += m->system()->page()->pos();
                applyDrop(score, m, element, modifiers, pt);
                if (m == last) {
                    break;
                }
            }
        } else if (element->type() == ElementType::LAYOUT_BREAK) {
            LayoutBreak* breakElement = static_cast<LayoutBreak*>(element);
            score->cmdToggleLayoutBreak(breakElement->layoutBreakType());
        } else if (element->isClef() || element->isKeySig() || element->isTimeSig()) {
            Measure* m1 = sel.startSegment()->measure();
            Measure* m2 = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
            if (m2 == m1 && sel.startSegment()->rtick().isZero()) {
                m2 = nullptr;             // don't restore original if one full measure selected
            } else if (m2) {
                m2 = m2->nextMeasureMM();
            }
            // for clefs, apply to each staff separately
            // otherwise just apply to top staff
            int staffIdx1 = sel.staffStart();
            int staffIdx2 = element->type() == ElementType::CLEF ? sel.staffEnd() : staffIdx1 + 1;
            for (int i = staffIdx1; i < staffIdx2; ++i) {
                // for clefs, use mid-measure changes if appropriate
                Element* e1 = nullptr;
                Element* e2 = nullptr;
                // use mid-measure clef changes as appropriate
                if (element->type() == ElementType::CLEF) {
                    if (sel.startSegment()->isChordRestType() && sel.startSegment()->rtick().isNotZero()) {
                        ChordRest* cr = static_cast<ChordRest*>(sel.startSegment()->nextChordRest(i * VOICES));
                        if (cr && cr->isChord()) {
                            e1 = static_cast<Chord*>(cr)->upNote();
                        } else {
                            e1 = cr;
                        }
                    }
                    if (sel.endSegment() && sel.endSegment()->segmentType() == SegmentType::ChordRest) {
                        ChordRest* cr = static_cast<ChordRest*>(sel.endSegment()->nextChordRest(i * VOICES));
                        if (cr && cr->isChord()) {
                            e2 = static_cast<Chord*>(cr)->upNote();
                        } else {
                            e2 = cr;
                        }
                    }
                }
                if (m2 || e2) {
                    // restore original clef/keysig/timesig
                    Staff* staff = score->staff(i);
                    Fraction tick1 = sel.startSegment()->tick();
                    Element* oelement = nullptr;
                    switch (element->type()) {
                    case ElementType::CLEF:
                    {
                        Clef* oclef = new Clef(score);
                        oclef->setClefType(staff->clef(tick1));
                        oelement = oclef;
                        break;
                    }
                    case ElementType::KEYSIG:
                    {
                        KeySig* okeysig = new KeySig(score);
                        okeysig->setKeySigEvent(staff->keySigEvent(tick1));
                        if (!score->styleB(Sid::concertPitch) && !okeysig->isCustom() && !okeysig->isAtonal()) {
                            Interval v = staff->part()->instrument(tick1)->transpose();
                            if (!v.isZero()) {
                                Key k = okeysig->key();
                                okeysig->setKey(transposeKey(k, v, okeysig->part()->preferSharpFlat()));
                            }
                        }
                        oelement = okeysig;
                        break;
                    }
                    case ElementType::TIMESIG:
                    {
                        TimeSig* otimesig = new TimeSig(score);
                        otimesig->setFrom(staff->timeSig(tick1));
                        oelement = otimesig;
                        break;
                    }
                    default:
                        break;
                    }
                    if (oelement) {
                        if (e2) {
                            applyDrop(score, e2, oelement, modifiers);
                        } else {
                            QRectF r = m2->staffabbox(i);
                            QPointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                            pt += m2->system()->page()->pos();
                            applyDrop(score, m2, oelement, modifiers, pt);
                        }
                        delete oelement;
                    }
                }
                // apply new clef/keysig/timesig
                if (e1) {
                    applyDrop(score, e1, element, modifiers);
                } else {
                    QRectF r = m1->staffabbox(i);
                    QPointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                    pt += m1->system()->page()->pos();
                    applyDrop(score, m1, element, modifiers, pt);
                }
            }
        } else if (element->isSlur()) {
            this->cmdAddSlur(toSlur(element));
        } else if (element->isSLine() && element->type() != ElementType::GLISSANDO) {
            Segment* startSegment = sel.startSegment();
            Segment* endSegment = sel.endSegment();
            bool firstStaffOnly = element->isVolta() && !(modifiers & Qt::ControlModifier);
            int startStaff = firstStaffOnly ? 0 : sel.staffStart();
            int endStaff   = firstStaffOnly ? 1 : sel.staffEnd();
            for (int i = startStaff; i < endStaff; ++i) {
                Spanner* spanner = static_cast<Spanner*>(element->clone());
                spanner->setScore(score);
                spanner->styleChanged();
                score->cmdAddSpanner(spanner, i, startSegment, endSegment);
            }
        } else {
            int track1 = sel.staffStart() * VOICES;
            int track2 = sel.staffEnd() * VOICES;
            Segment* startSegment = sel.startSegment();
            Segment* endSegment = sel.endSegment();       //keep it, it could change during the loop

            for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                for (int track = track1; track < track2; ++track) {
                    Element* e = s->element(track);
                    if (e == 0 || !score->selectionFilter().canSelect(e)
                        || !score->selectionFilter().canSelectVoice(track)) {
                        continue;
                    }
                    if (e->isChord()) {
                        Chord* chord = toChord(e);
                        for (Note* n : chord->notes()) {
                            applyDrop(score, n, element, modifiers);
                            if (!(element->isAccidental() || element->isNoteHead())) {             // only these need to apply to every note
                                break;
                            }
                        }
                    } else {
                        // do not apply articulation to barline in a range selection
                        if (!e->isBarLine() || !element->isArticulation()) {
                            applyDrop(score, e, element, modifiers);
                        }
                    }
                }
                if (!element->placeMultiple()) {
                    break;
                }
            }
        }
    } else {
        qDebug("unknown selection state");
    }

    if (this->mscoreState() != STATE_EDIT
        && this->mscoreState() != STATE_LYRICS_EDIT
        && this->mscoreState() != STATE_HARMONY_FIGBASS_EDIT) {  //Already in startCmd mode in this case
        score->endCmd();
        if (this->mscoreState() == STATE_NOTE_ENTRY_STAFF_DRUM) {
            this->moveCursor();
        }
    } else if (this->mscoreState() & STATE_ALLTEXTUAL_EDIT) {
        this->setFocus();
    }
    this->setDropTarget(0);
//      mscore->endCmd();
    return true;
}

void MU3PaletteAdapter::moveCursor()
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->moveCursor();
    }
}

void MU3PaletteAdapter::setFocus()
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->setFocus();
    }
}

void MU3PaletteAdapter::setDropTarget(const Ms::Element* e)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->setDropTarget(e);
    }
}

Ms::PaletteWorkspace* MU3PaletteAdapter::paletteWorkspace() const
{
    return mscore->getPaletteWorkspace();
}

mu::ValCh<bool> MU3PaletteAdapter::paletteEnabled() const
{
    return m_paletteEnabled;
}

void MU3PaletteAdapter::setPaletteEnabled(bool arg)
{
    m_paletteEnabled.set(arg);
}

void MU3PaletteAdapter::requestPaletteSearch()
{
    m_paletteSearchRequested.notify();
}

mu::async::Notification MU3PaletteAdapter::paletteSearchRequested() const
{
    return m_paletteSearchRequested;
}

void MU3PaletteAdapter::notifyElementDraggedToScoreView()
{
    m_elementDraggedToScoreView.notify();
}

mu::async::Notification MU3PaletteAdapter::elementDraggedToScoreView() const
{
    return m_elementDraggedToScoreView;
}
