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

#include "translation.h"
#include "ui/view/iconcodes.h"
#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetnavigationfix.h"

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
