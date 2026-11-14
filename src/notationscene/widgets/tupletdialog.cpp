/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "tupletdialog.h"

#include "notation/inotation.h"
#include "notation/inotationstyle.h"
#include "notationcommands.h"

#include "ui/view/widgetstatestore.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::ui;
using namespace muse;
using namespace muse::rcommand;

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

TupletDialog::TupletDialog(QWidget* parent)
    : muse::ui::WidgetDialog(parent)
{
    setObjectName("TupletDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void TupletDialog::componentComplete()
{
    connect(buttonBox, &QDialogButtonBox::clicked, this, &TupletDialog::bboxClicked);

    defaultToStyleSettings();
    setSelectedDurationAs(configuration()->tupletCommandsSelectedDurationAs());
    restorePreviousState();

    numberGroupBox->setAccessibleName(formatGroupBox->title() + " " + numberGroupBox->title());
    bracketGroupBox->setAccessibleName(formatGroupBox->title() + " " + bracketGroupBox->title());
    subdivision->setAccessibleName(useSelectedDurationAsLabel->text() + " " + subdivision->text());
    tupletLength->setAccessibleName(useSelectedDurationAsLabel->text() + " " + tupletLength->text());

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

//---------------------------------------------------------
//   defaultToStyleSettings
//---------------------------------------------------------

void TupletDialog::defaultToStyleSettings()
{
    setNumberType(TupletNumberType(style()->styleValue(Sid::tupletNumberType).toInt()));
    setBracketType(TupletBracketType(style()->styleValue(Sid::tupletBracketType).toInt()));
}

void TupletDialog::setNumberType(TupletNumberType type)
{
    number->setChecked(type == TupletNumberType::SHOW_NUMBER);
    relation->setChecked(type == TupletNumberType::SHOW_RELATION);
    noNumber->setChecked(type == TupletNumberType::NO_TEXT);
}

void TupletDialog::setBracketType(TupletBracketType type)
{
    autoBracket->setChecked(type == TupletBracketType::AUTO_BRACKET);
    bracket->setChecked(type == TupletBracketType::SHOW_BRACKET);
    noBracket->setChecked(type == TupletBracketType::SHOW_NO_BRACKET);
}

void TupletDialog::setSelectedDurationAs(TupletSelectedDuration value)
{
    subdivision->setChecked(value == TupletSelectedDuration::SUBDIVISION);
    tupletLength->setChecked(value == TupletSelectedDuration::TUPLET_LENGTH);
}

TupletNumberType TupletDialog::numberType() const
{
    if (number->isChecked()) {
        return TupletNumberType::SHOW_NUMBER;
    } else if (relation->isChecked()) {
        return TupletNumberType::SHOW_RELATION;
    } else if (noNumber->isChecked()) {
        return TupletNumberType::NO_TEXT;
    }

    return TupletNumberType::SHOW_NUMBER;
}

TupletBracketType TupletDialog::bracketType() const
{
    if (autoBracket->isChecked()) {
        return TupletBracketType::AUTO_BRACKET;
    } else if (bracket->isChecked()) {
        return TupletBracketType::SHOW_BRACKET;
    } else if (noBracket->isChecked()) {
        return TupletBracketType::SHOW_NO_BRACKET;
    }

    return TupletBracketType::AUTO_BRACKET;
}

TupletSelectedDuration TupletDialog::selectedDurationAs() const
{
    if (tupletLength->isChecked()) {
        return TupletSelectedDuration::TUPLET_LENGTH;
    }

    return TupletSelectedDuration::SUBDIVISION;
}

TupletDialogState& TupletDialog::previousState()
{
    static TupletDialogState state;
    return state;
}

void TupletDialog::saveState()
{
    TupletDialogState& state = previousState();

    state.masterNotation = projectSession()->masterNotation();
    state.ratio = Fraction(actualNotes->value(), normalNotes->value());
    state.selectedDurationAs = selectedDurationAs();
    state.numberType = numberType();
    state.bracketType = bracketType();
}

void TupletDialog::restorePreviousState()
{
    const TupletDialogState& state = previousState();

    // choices made in another score don't carry over
    if (state.masterNotation.lock() != projectSession()->masterNotation()) {
        return;
    }

    actualNotes->setValue(state.ratio.numerator());
    normalNotes->setValue(state.ratio.denominator());
    setSelectedDurationAs(state.selectedDurationAs);
    setNumberType(state.numberType);
    setBracketType(state.bracketType);
}

void TupletDialog::showEvent(QShowEvent* event)
{
    WidgetStateStore::restoreGeometry(this);
    QDialog::showEvent(event);
}

void TupletDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(event);
}

INotationStylePtr TupletDialog::style() const
{
    return notation() ? notation()->style() : nullptr;
}

INotationPtr TupletDialog::notation() const
{
    return projectSession()->currentNotation();
}

void TupletDialog::apply()
{
    AddTupletCommand cmd;
    cmd.ratio = Fraction(actualNotes->value(), normalNotes->value()).toString().toStdString();
    cmd.numberType = numberType();
    cmd.bracketType = bracketType();
    cmd.selectedDurationAs = selectedDurationAs();
    cmd.autoBaseLen = false;

    saveState();

    dispatcher()->dispatch(cmd);
}

void TupletDialog::bboxClicked(QAbstractButton* button)
{
    QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
    switch (br) {
    case QDialogButtonBox::ApplyRole:
        apply();
        break;
    case QDialogButtonBox::AcceptRole:
        apply();
        done(1);
        break;
    case QDialogButtonBox::RejectRole:
        done(0);
        break;
    default:
        break;
    }
}
