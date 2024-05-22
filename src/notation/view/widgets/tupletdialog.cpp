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
#include "tupletdialog.h"

#include "engraving/dom/tuplet.h"

#include "ui/view/widgetstatestore.h"

using namespace mu::notation;
using namespace muse::ui;
using namespace muse::actions;

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

TupletDialog::TupletDialog(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setObjectName("TupletDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    WidgetStateStore::restoreGeometry(this);

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
    TupletOptions options;
    options.ratio = Fraction(actualNotes->value(), normalNotes->value());
    options.numberType = numberType();
    options.bracketType = bracketType();

    ActionData data_ = ActionData::make_arg1<TupletOptions>(options);
    dispatcher()->dispatch("custom-tuplet", data_);
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
