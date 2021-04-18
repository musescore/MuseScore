//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2011 Werner Schweer and others
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

#include "breaksdialog.h"

using namespace mu::notation;

static constexpr int DEFAULT_INTERVAL = 4;

//---------------------------------------------------------
//   BreaksDialog
//---------------------------------------------------------

BreaksDialog::BreaksDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    intervalButton->setChecked(true);
    intervalBox->setValue(DEFAULT_INTERVAL);
}

BreaksDialog::BreaksDialog(const BreaksDialog& dialog)
    : BreaksDialog(dialog.parentWidget())
{
}

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void BreaksDialog::accept()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    INotationInteractionPtr interaction = notation->interaction();
    INotationSelectionPtr selection = interaction->selection();

    bool allSelected = false;

    if (selection->isNone()) {
        interaction->selectAll();
        allSelected = true;
    }

    int interval = intervalButton->isChecked() ? intervalBox->value() : 0;
    BreaksSpawnIntervalType intervalType = BreaksSpawnIntervalType::MeasuresInterval;

    if (removeButton->isChecked()) {
        intervalType = BreaksSpawnIntervalType::None;
    } else if (lockButton->isChecked()) {
        intervalType = BreaksSpawnIntervalType::AfterEachSystem;
    }

    interaction->setBreaksSpawnInterval(intervalType, interval);

    if (allSelected) {
        interaction->clearSelection();
    }

    QDialog::accept();
}
