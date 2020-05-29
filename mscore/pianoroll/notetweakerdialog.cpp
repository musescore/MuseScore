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

#include "notetweakerdialog.h"

#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "libmscore/noteevent.h"
#include "libmscore/undo.h"

namespace Ms {
//---------------------------------------------------------
//   NoteTweakerDialog
//---------------------------------------------------------

NoteTweakerDialog::NoteTweakerDialog(QWidget* parent)
    : QDialog(parent),
    _staff(nullptr),
    ui(new Ui::NoteTweakerDialog)
{
    ui->setupUi(this);

    connect(ui->bnClose, &QPushButton::clicked, this, &QDialog::close);
    connect(ui->bnSetNoteOffTime, &QPushButton::clicked, this, &NoteTweakerDialog::setNoteOffTime);
}

//---------------------------------------------------------
//   ~NoteTweakerDialog
//---------------------------------------------------------

NoteTweakerDialog::~NoteTweakerDialog()
{
    delete ui;
}

//---------------------------------------------------------
//   setNoteOffTime
//---------------------------------------------------------

void NoteTweakerDialog::setNoteOffTime()
{
    if (!_staff) {
        return;
    }

    QString s = ui->comboOffTimeFrac->currentText();
    QStringList parts = s.split("/");
    int num = parts[0].toInt();
    double denom = parts[1].toInt();
    double gapTicks = MScore::division * num / denom;

    Score* score = _staff->score();

    score->startCmd();

    for (Note* note: noteList) {
        if (!note->selected()) {
            continue;
        }

        Chord* chord = note->chord();
        int ticks = chord->ticks().ticks();

        int scalar = 1000 * (ticks - gapTicks) / ticks;
        if (scalar <= 0) {
            scalar = 1;
        }

        for (NoteEvent& e : note->playEvents()) {
            NoteEvent ne = e;
            ne.setLen(scalar);
            score->undo(new ChangeNoteEvent(note, &e, ne));
        }
    }

    score->endCmd();

    emit notesChanged();
}

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void NoteTweakerDialog::setStaff(Staff* s)
{
    if (_staff == s) {
        return;
    }

    _staff    = s;
    updateNotes();
}

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void NoteTweakerDialog::addChord(Chord* chord, int voice)
{
    for (Chord* c : chord->graceNotes()) {
        addChord(c, voice);
    }
    for (Note* note : chord->notes()) {
        if (note->tieBack()) {
            continue;
        }
        noteList.append(note);
    }
}

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void NoteTweakerDialog::updateNotes()
{
    clearNoteData();

    if (!_staff) {
        return;
    }

    int staffIdx = _staff->idx();
    if (staffIdx == -1) {
        return;
    }

    SegmentType st = SegmentType::ChordRest;
    for (Segment* s = _staff->score()->firstSegment(st); s; s = s->next1(st)) {
        for (int voice = 0; voice < VOICES; ++voice) {
            int track = voice + staffIdx * VOICES;
            Element* e = s->element(track);
            if (e && e->isChord()) {
                addChord(toChord(e), voice);
            }
        }
    }

    update();
}

//---------------------------------------------------------
//   clearNoteData
//---------------------------------------------------------

void NoteTweakerDialog::clearNoteData()
{
    noteList.clear();
}
}
