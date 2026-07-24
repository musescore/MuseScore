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

#include "engraving/dom/tuplet.h"

#include "notation/inotationstyle.h"
#include "notation/types/noteinputtypes.h"
#include "notationcommands.h"

#include "ui/view/widgetstatestore.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::ui;
using namespace muse;

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

    numberGroupBox->setAccessibleName(formatGroupBox->title() + " " + numberGroupBox->title());
    bracketGroupBox->setAccessibleName(formatGroupBox->title() + " " + bracketGroupBox->title());

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

//---------------------------------------------------------
//   defaultToStyleSettings
//---------------------------------------------------------

void TupletDialog::defaultToStyleSettings()
{
    mu::engraving::TupletNumberType nt = mu::engraving::TupletNumberType(style()->styleValue(mu::engraving::Sid::tupletNumberType).toInt());
    number->setChecked(nt == mu::engraving::TupletNumberType::SHOW_NUMBER);
    relation->setChecked(nt == mu::engraving::TupletNumberType::SHOW_RELATION);
    noNumber->setChecked(nt == mu::engraving::TupletNumberType::NO_TEXT);

    mu::engraving::TupletBracketType bt = mu::engraving::TupletBracketType(style()->styleValue(
                                                                               mu::engraving::Sid::tupletBracketType).toInt());
    autoBracket->setChecked(bt == mu::engraving::TupletBracketType::AUTO_BRACKET);
    bracket->setChecked(bt == mu::engraving::TupletBracketType::SHOW_BRACKET);
    noBracket->setChecked(bt == mu::engraving::TupletBracketType::SHOW_NO_BRACKET);
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
    return globalContext()->currentNotation();
}

void TupletDialog::apply()
{
    rcommand::CommandQuery query(ADD_TUPLET_COMMAND);
    query.addParam("ratio", Val(Fraction(actualNotes->value(), normalNotes->value()).toString().toStdString()));
    query.addParam("number-type", Val(engraving::str_conv(numberType())));
    query.addParam("bracket-type", Val(engraving::str_conv(bracketType())));

    dispatcher()->dispatch(query);
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
