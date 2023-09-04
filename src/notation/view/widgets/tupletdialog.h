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
#ifndef MU_NOTATION_TUPLETDIALOG_H
#define MU_NOTATION_TUPLETDIALOG_H

#include "ui_tupletdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"

#include "engraving/dom/durationelement.h"

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------
namespace mu::notation {
class TupletDialog : public QDialog, Ui::TupletDialog
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)
    INJECT(actions::IActionsDispatcher, dispatcher)

    virtual void hideEvent(QHideEvent*);

public:
    TupletDialog(QWidget* parent = nullptr);
    TupletDialog(const TupletDialog&);

    void defaultToStyleSettings();

    TupletNumberType numberType() const;
    TupletBracketType bracketType() const;

    INotationStylePtr style() const;
    INotationPtr notation() const;

    void apply();

private slots:
    void bboxClicked(QAbstractButton* button);
};
}

Q_DECLARE_METATYPE(mu::notation::TupletDialog)

#endif // MU_NOTATION_TUPLETDIALOG_H
