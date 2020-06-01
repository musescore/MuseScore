//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#include "realizeharmonydialog.h"
#include "libmscore/harmony.h"
#include "libmscore/staff.h"

namespace Ms {
//---------------------------------------------------------
//   RealizeHarmonyDialog
//---------------------------------------------------------

RealizeHarmonyDialog::RealizeHarmonyDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("TestingName");
    setupUi(this);

    chordTable->setVisible(false);
    connect(showButton, SIGNAL(clicked()), SLOT(toggleChordTable()));

    voicingSelect->setLiteral(true);

    //make the chord list uneditable
    chordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//---------------------------------------------------------
//   toggleChordTable
//---------------------------------------------------------
void RealizeHarmonyDialog::toggleChordTable()
{
    int visible = chordTable->isVisible();
    chordTable->setVisible(!visible);
    showButton->setText(!visible ? tr("Show less…") : tr("Show more…"));
}

//---------------------------------------------------------
//   setChordList
///   fill the chord list with the list of harmonies
//---------------------------------------------------------
void RealizeHarmonyDialog::setChordList(QList<Harmony*> hlist)
{
    static const QStringList header = { "ID", "Name", "Intervals", "Notes" };
    QString s;    //chord label string
    int rows = hlist.size();

    chordTable->setRowCount(rows);
    chordTable->setColumnCount(header.size());
    chordTable->setHorizontalHeaderLabels(header);
    for (int i = 0; i < rows; ++i) {
        Harmony* h = hlist.at(i);
        if (!h->isRealizable()) {
            continue;
        }

        s += h->harmonyName() + " ";
        QString intervals;
        QString noteNames;
        int rootTpc;

        //adjust for nashville function
        if (h->harmonyType() == HarmonyType::NASHVILLE) {
            rootTpc = function2Tpc(h->hFunction(), h->staff()->key(h->tick()));
        } else {
            rootTpc = h->rootTpc();
        }

        noteNames = tpc2name(rootTpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO);
        RealizedHarmony::PitchMap map = h->getRealizedHarmony().notes();
        for (int pitch : map.keys()) {
            intervals += QString::number((pitch - tpc2pitch(rootTpc)) % 128 % 12) + " ";
        }
        for (int tpc : map.values()) {
            noteNames += ", " + tpc2name(tpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO);
        }

        chordTable->setItem(i, 0, new QTableWidgetItem(QString::number(h->id())));
        chordTable->setItem(i, 1, new QTableWidgetItem(h->harmonyName()));
        chordTable->setItem(i, 2, new QTableWidgetItem(intervals));
        chordTable->setItem(i, 3, new QTableWidgetItem(noteNames));
    }
    chordLabel->setText(s);

    //set table uneditable again
    chordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
}
