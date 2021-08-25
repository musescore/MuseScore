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
#include "pianorollcontroller.h"

#include "log.h"

#include <vector>

#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "libmscore/segment.h"
#include "libmscore/noteevent.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "libmscore/mscore.h"

#include <qdebug.h>

using namespace mu::pianoroll;
using namespace mu::midi;
using namespace mu::notation;
using namespace mu::async;
using namespace mu::audio;
using namespace mu::actions;
//using namespace mu::Ms;


NoteBlock::NoteBlock(Note* note):
    m_note(note)
{
}

void PianorollController::init()
{

    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

}

int PianorollController::getNotes() const
{
    return 49;
}

Ms::Measure* PianorollController::lastMeasure()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Ms::Score* score = notation->elements()->msScore();
    return score->lastMeasure();
}

Ms::Score* PianorollController::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Ms::Score* score = notation->elements()->msScore();
    return score;
}


Notification PianorollController::noteLayoutChanged() const
{
    return m_noteLayoutChanged;
}

void PianorollController::onSelectionChanged()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }
    std::vector<Element*> selectedElements = notation->interaction()->selection()->elements();
}

void PianorollController::onNotationChanged()
{
    auto notation = globalContext()->currentNotation();
    if (notation)
    {
        notation->interaction()->selectionChanged().onNotify(this, [this]() {
            onSelectionChanged();
        });
    }

    buildNoteBlocks();
}

void PianorollController::setXZoom(double value)
{
    if (value == m_xZoom)
        return;
    m_xZoom = value;
    emit xZoomChanged();
}

void PianorollController::setNoteHeight(int value)
{
    if (value == m_noteHeight)
        return;
    m_noteHeight = value;
    emit noteHeightChanged();
}

void PianorollController::addChord(Chord* chrd, int voice)
{
    for (Chord* c : chrd->graceNotes())
        addChord(c, voice);

    for (Note* note : chrd->notes()) {
        if (note->tieBack())
              continue;
        m_notes.push_back(NoteBlock(note));
//        _noteList.append(new PianoItem(note, this));
    }
}

 

Ms::Fraction PianorollController::widthInBeats()
{
//    return m_widthInTicks; 
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return Ms::Fraction();
    }

    qDebug() << "---------";

    Ms::Score* score = notation->elements()->msScore();
    //std::vector<Element*> selectedElements = notation->interaction()->selection()->elements();
    //m_selectedStaves.clear();
    //m_activeStaff = -1;
    //for (Element* e : selectedElements)
    //{
    //    int idx = e->staffIdx();
    //    qDebug() << "ele idx " << idx;
    //    m_activeStaff = idx;
    //    if (std::find(m_selectedStaves.begin(), m_selectedStaves.end(), idx) == m_selectedStaves.end())
    //    {
    //        m_selectedStaves.push_back(idx);
    //    }
    //}

//    Ms::Staff* staff = score->staff(m_activeStaff);
    Ms::Measure* lm = score->lastMeasure();
    Ms::Fraction beats = lm->tick() + lm->ticks();
    return beats;
}

void PianorollController::buildNoteBlocks()
{
    m_notes.clear();


    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    qDebug() << "---------";

    Ms::Score* score = notation->elements()->msScore();
    std::vector<Element*> selectedElements = notation->interaction()->selection()->elements();
    m_selectedStaves.clear();
    m_activeStaff = -1;
    for (Element* e: selectedElements)
    {
        int idx = e->staffIdx();
        qDebug() << "ele idx " << idx;
        m_activeStaff = idx;
        if (std::find(m_selectedStaves.begin(), m_selectedStaves.end(), idx) == m_selectedStaves.end())
        {
            m_selectedStaves.push_back(idx);

        }
    }


//    int staffIdx = score->staffIdx();

//    for (Ms::Segment* s = score->firstSegment(Ms::SegmentType::ChordRest); s; s = s->next1(Ms::SegmentType::ChordRest))
//    {
//        for (int voice = 0; voice < MAX_VOICES; ++voice)
//        {
//            int track = voice + staffIdx * MAX_VOICES;
//            Element* e = s->element(track);
//            if (e && e->isChord())
//                addChord(toChord(e), voice);
//        }
//    }


    //Update bounds
//    Measure* lm = _staff->score()->lastMeasure();
//    m_widthInTicks = (lm->tick() + lm->ticks()).ticks();




//    INotationElementsPtr elements = notation->elements();
////    auto list = elements->elements();
//    std::vector<Element*> list = elements->elements();
//    for (Element* ele: list) {
//        bool sel = ele->selected();
//        auto type = ele->type();
//        auto name = ele->name();

//        int j = 9;
//    }

    m_noteLayoutChanged.notify();

}
