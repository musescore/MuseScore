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
#include "editpitch.h"

#include <QKeyEvent>

#include "translation.h"
#include "ui/view/iconcodes.h"
#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetnavigationfix.h"

using namespace mu::notation;
using namespace mu::ui;

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

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

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
                new WidgetNavigationFix::NavigationChain { tableWidget, buttonBox, buttonBox },
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

    auto clefTrebleItem = tableWidget->item(5, 7);
    if (clefTrebleItem) {
        clefTrebleItem->setText(iconCodeToChar(IconCode::Code::CLEF_TREBLE) + QString(" ") + qtrc("notation", "G 4"));
    }

    auto clefBassItem = tableWidget->item(6, 5);
    if (clefBassItem) {
        clefBassItem->setText(iconCodeToChar(IconCode::Code::CLEF_BASS) + QString(" ") + qtrc("notation", "F 3"));
    }

    WidgetStateStore::restoreGeometry(this);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();

    qApp->installEventFilter(this);
}

void EditPitch::accept()
{
    on_tableWidget_cellDoubleClicked(tableWidget->currentRow(), tableWidget->currentColumn());
}

void EditPitch::on_tableWidget_cellDoubleClicked(int row, int col)
{
    // topmost row contains notes for 10-th MIDI octave (numbered as '9')
    int pitch = (tableWidget->rowCount() - 1 - row) * 12 + col;
    done((pitch > 127) ? 127 : pitch);
}
