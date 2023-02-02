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

    //: `%1` will be replaced with a number input field.
    //: Text before it will appear before that number field, text after will appear after the field.
    QString text = qtrc("notation/add-remove-system-breaks", "Break systems every %1 measures");
    QStringList pieces = text.split(QStringLiteral("%1"));

    IF_ASSERT_FAILED(pieces.size() >= 2) {
        return;
    }

    QString part1 = pieces[0].trimmed();
    QString part2 = pieces[1].trimmed();
    intervalButton->setText(part1);
    intervalLabel2->setText(part2);
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

    int interval = intervalButton->isChecked() ? intervalBox->value() : 0;
    BreaksSpawnIntervalType intervalType = BreaksSpawnIntervalType::MeasuresInterval;

    if (removeButton->isChecked()) {
        intervalType = BreaksSpawnIntervalType::None;
    } else if (lockButton->isChecked()) {
        intervalType = BreaksSpawnIntervalType::AfterEachSystem;
    }

    interaction->setBreaksSpawnInterval(intervalType, interval);

    if (_allSelected) {
        interaction->clearSelection();
    }

    QDialog::accept();
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void BreaksDialog::showEvent(QShowEvent* ev)
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    INotationInteractionPtr interaction = notation->interaction();
    INotationSelectionPtr selection = interaction->selection();

    if (!selection->isRange()) {
        interaction->selectAll();
        _allSelected = true;
    } else {
        _allSelected = false;
    }

    QWidget::showEvent(ev);
}
