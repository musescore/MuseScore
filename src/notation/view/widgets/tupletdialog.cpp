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
#include "tupletdialog.h"

#include "libmscore/tuplet.h"

#include "widgetstatestore.h"

using namespace mu::notation;

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

TupletDialog::TupletDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("TupletDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    WidgetStateStore::restoreGeometry(this);

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));

    defaultToStyleSettings();
}

TupletDialog::TupletDialog(const TupletDialog& other)
    : QDialog(other.parentWidget())
{
}

//---------------------------------------------------------
//   defaultToStyleSettings
//---------------------------------------------------------

void TupletDialog::defaultToStyleSettings()
{
    Ms::TupletNumberType nt = Ms::TupletNumberType(style()->styleValue(Ms::Sid::tupletNumberType).toInt());
    number->setChecked(nt == Ms::TupletNumberType::SHOW_NUMBER);
    relation->setChecked(nt == Ms::TupletNumberType::SHOW_RELATION);
    noNumber->setChecked(nt == Ms::TupletNumberType::NO_TEXT);

    Ms::TupletBracketType bt = Ms::TupletBracketType(style()->styleValue(Ms::Sid::tupletBracketType).toInt());
    autoBracket->setChecked(bt == Ms::TupletBracketType::AUTO_BRACKET);
    bracket->setChecked(bt == Ms::TupletBracketType::SHOW_BRACKET);
    noBracket->setChecked(bt == Ms::TupletBracketType::SHOW_NO_BRACKET);
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

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

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
    return globalContext()->currentNotation();
}

void TupletDialog::apply()
{
    auto interaction = notation()->interaction();
    if (!interaction) {
        return;
    }

    auto noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    TupletOptions options;
    options.ratio = Fraction(actualNotes->value(), normalNotes->value());
    options.numberType = numberType();
    options.bracketType = bracketType();

    if (noteInput->isNoteInputMode()) {
        noteInput->addTuplet(options);
    } else {
        interaction->addTupletToSelectedChordRests(options);
    }
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
