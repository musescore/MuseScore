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

    // Populate table with localized pitch names and center them in cells
    static constexpr const char* noteName[] =
    { "C", "C♯", "D", "D♯", "E", "F", "F♯", "G", "G♯", "A", "A♯", "B", "C" };

    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            if (row == 0 && col > 7) {
                // Skip MIDI notes above G9
                break;
            }
            QTableWidgetItem* item = tableWidget->item(row, col);
            if (item) {
                int octave;
                if (col == tableWidget->columnCount() - 1) {
                    octave = 10 - row;
                } else {
                    octave = 9 - row;
                }
                std::string pitch = std::string(noteName[col]) + std::to_string(octave);
                item->setText(SPN_PITCH_DISPLAY(wantSPN, pitch.c_str()));
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
