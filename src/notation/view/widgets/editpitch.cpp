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
#include "editpitch.h"

#include <QKeyEvent>

#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetnavigationfix.h"

#include "engraving/dom/pitchspelling.h"

#define SPN_PITCH_DISPLAY(wantSPN, pitch) (wantSPN ? muse::String(pitch) : muse::mtrc("global/pitchName", pitch))

using namespace mu::notation;
using namespace muse::ui;

//---------------------------------------------------------
//   EditPitch
//    To select a MIDI pitch code using human-readable note names
//---------------------------------------------------------

EditPitch::EditPitch(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("EditPitchNew");

    setup();

    tableWidget->setCurrentCell(tableWidget->rowCount() - 1 - 5, 0);                  // select centre C by default
}

EditPitch::EditPitch(QWidget* parent, int midiCode)
    : QDialog(parent)
{
    setObjectName("EditPitchEdit");

    setup();

    tableWidget->setCurrentCell(tableWidget->rowCount() - 1 - (midiCode / 12), midiCode % 12);
}

void EditPitch::showEvent(QShowEvent* event)
{
    WidgetStateStore::restoreGeometry(this);
    QWidget::showEvent(event);
}

void EditPitch::hideEvent(QHideEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

bool EditPitch::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent
            && WidgetNavigationFix::fixNavigationForTableWidget(
                WidgetNavigationFix::NavigationChain { tableWidget, buttonBox, buttonBox },
                keyEvent->key())) {
            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void EditPitch::setup()
{
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();

    qApp->installEventFilter(this);

    // Display pitch names according to user preference
    bool wantSPN = engravingConfiguration()->pitchNotationSPN();
    // octave 9
    tableWidget->setItem(0, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C9")));
    tableWidget->setItem(0, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯9")));
    tableWidget->setItem(0, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D9")));
    tableWidget->setItem(0, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯9")));
    tableWidget->setItem(0, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E9")));
    tableWidget->setItem(0, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F9")));
    tableWidget->setItem(0, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯9")));
    tableWidget->setItem(0, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G9")));
    // octave 8
    tableWidget->setItem(1, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C8")));
    tableWidget->setItem(1, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯8")));
    tableWidget->setItem(1, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D8")));
    tableWidget->setItem(1, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯8")));
    tableWidget->setItem(1, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E8")));
    tableWidget->setItem(1, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F8")));
    tableWidget->setItem(1, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯8")));
    tableWidget->setItem(1, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G8")));
    tableWidget->setItem(1, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯8")));
    tableWidget->setItem(1, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A8")));
    tableWidget->setItem(1, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯8")));
    tableWidget->setItem(1, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B8")));
    // octave 7
    tableWidget->setItem(2, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C7")));
    tableWidget->setItem(2, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯7")));
    tableWidget->setItem(2, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D7")));
    tableWidget->setItem(2, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯7")));
    tableWidget->setItem(2, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E7")));
    tableWidget->setItem(2, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F7")));
    tableWidget->setItem(2, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯7")));
    tableWidget->setItem(2, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G7")));
    tableWidget->setItem(2, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯7")));
    tableWidget->setItem(2, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A7")));
    tableWidget->setItem(2, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯7")));
    tableWidget->setItem(2, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B7")));
    // octave 6
    tableWidget->setItem(3, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C6")));
    tableWidget->setItem(3, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯6")));
    tableWidget->setItem(3, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D6")));
    tableWidget->setItem(3, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯6")));
    tableWidget->setItem(3, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E6")));
    tableWidget->setItem(3, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F6")));
    tableWidget->setItem(3, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯6")));
    tableWidget->setItem(3, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G6")));
    tableWidget->setItem(3, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯6")));
    tableWidget->setItem(3, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A6")));
    tableWidget->setItem(3, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯6")));
    tableWidget->setItem(3, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B6")));
    // octave 5
    tableWidget->setItem(4, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C5")));
    tableWidget->setItem(4, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯5")));
    tableWidget->setItem(4, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D5")));
    tableWidget->setItem(4, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯5")));
    tableWidget->setItem(4, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E5")));
    tableWidget->setItem(4, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F5")));
    tableWidget->setItem(4, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯5")));
    tableWidget->setItem(4, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G5")));
    tableWidget->setItem(4, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯5")));
    tableWidget->setItem(4, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A5")));
    tableWidget->setItem(4, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯5")));
    tableWidget->setItem(4, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B5")));
    // octave 4
    tableWidget->setItem(5, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C4")));
    tableWidget->setItem(5, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯4")));
    tableWidget->setItem(5, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D4")));
    tableWidget->setItem(5, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯4")));
    tableWidget->setItem(5, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E4")));
    tableWidget->setItem(5, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F4")));
    tableWidget->setItem(5, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯4")));
    tableWidget->setItem(5, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G4")));
    tableWidget->setItem(5, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯4")));
    tableWidget->setItem(5, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A4")));
    tableWidget->setItem(5, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯4")));
    tableWidget->setItem(5, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B4")));
    // octave 3
    tableWidget->setItem(6, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C3")));
    tableWidget->setItem(6, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯3")));
    tableWidget->setItem(6, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D3")));
    tableWidget->setItem(6, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯3")));
    tableWidget->setItem(6, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E3")));
    tableWidget->setItem(6, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F3")));
    tableWidget->setItem(6, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯3")));
    tableWidget->setItem(6, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G3")));
    tableWidget->setItem(6, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯3")));
    tableWidget->setItem(6, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A3")));
    tableWidget->setItem(6, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯3")));
    tableWidget->setItem(6, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B3")));
    // octave 2
    tableWidget->setItem(7, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C2")));
    tableWidget->setItem(7, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯2")));
    tableWidget->setItem(7, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D2")));
    tableWidget->setItem(7, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯2")));
    tableWidget->setItem(7, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E2")));
    tableWidget->setItem(7, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F2")));
    tableWidget->setItem(7, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯2")));
    tableWidget->setItem(7, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G2")));
    tableWidget->setItem(7, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯2")));
    tableWidget->setItem(7, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A2")));
    tableWidget->setItem(7, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯2")));
    tableWidget->setItem(7, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B2")));
    // octave 1
    tableWidget->setItem(8, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C1")));
    tableWidget->setItem(8, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯1")));
    tableWidget->setItem(8, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D1")));
    tableWidget->setItem(8, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯1")));
    tableWidget->setItem(8, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E1")));
    tableWidget->setItem(8, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F1")));
    tableWidget->setItem(8, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯1")));
    tableWidget->setItem(8, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G1")));
    tableWidget->setItem(8, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯1")));
    tableWidget->setItem(8, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A1")));
    tableWidget->setItem(8, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯1")));
    tableWidget->setItem(8, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B1")));
    // octave 0
    tableWidget->setItem(9, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C0")));
    tableWidget->setItem(9, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯0")));
    tableWidget->setItem(9, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D0")));
    tableWidget->setItem(9, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯0")));
    tableWidget->setItem(9, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E0")));
    tableWidget->setItem(9, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F0")));
    tableWidget->setItem(9, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯0")));
    tableWidget->setItem(9, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G0")));
    tableWidget->setItem(9, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯0")));
    tableWidget->setItem(9, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A0")));
    tableWidget->setItem(9, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯0")));
    tableWidget->setItem(9, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B0")));
    // octave -1
    tableWidget->setItem(10, 0, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C-1")));
    tableWidget->setItem(10, 1, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "C♯-1")));
    tableWidget->setItem(10, 2, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D-1")));
    tableWidget->setItem(10, 3, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "D♯-1")));
    tableWidget->setItem(10, 4, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "E-1")));
    tableWidget->setItem(10, 5, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F-1")));
    tableWidget->setItem(10, 6, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "F♯-1")));
    tableWidget->setItem(10, 7, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G-1")));
    tableWidget->setItem(10, 8, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "G♯-1")));
    tableWidget->setItem(10, 9, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A-1")));
    tableWidget->setItem(10, 10, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "A♯-1")));
    tableWidget->setItem(10, 11, new QTableWidgetItem(SPN_PITCH_DISPLAY(wantSPN, "B-1")));

    // Center cell contents
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = tableWidget->item(row, col);
            if (item) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
}

void EditPitch::accept()
{
    on_tableWidget_cellDoubleClicked(tableWidget->currentRow(), tableWidget->currentColumn());
}

void EditPitch::on_tableWidget_cellDoubleClicked(int row, int col)
{
    // topmost row contains notes for 10-th MIDI octave (numbered as '9')
    int pitch = (tableWidget->rowCount() - 1 - row) * 12 + col;
    done(std::min(pitch, engraving::MAX_PITCH));
}
