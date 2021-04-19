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
#ifndef MU_NOTATION_SELECTDIALOG_H
#define MU_NOTATION_SELECTDIALOG_H

#include "ui_selectdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace Ms {
class System;
}

namespace mu::notation {
//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

class SelectDialog : public QDialog, Ui::SelectDialog
{
    Q_OBJECT
    INJECT(notation, context::IGlobalContext, globalContext)

public:
    SelectDialog(QWidget* parent = nullptr);
    SelectDialog(const SelectDialog& other);

    static int metaTypeId();

    bool doReplace() const;
    bool doAdd() const;
    bool doSubtract() const;
    bool doFromSelection() const;
    bool isInSelection() const;

private slots:
    void buttonClicked(QAbstractButton* button);

private:
    virtual void hideEvent(QHideEvent*);

    INotationPtr currentNotation() const;
    INotationInteractionPtr currentNotationInteraction() const;
    INotationElementsPtr currentNotationElements() const;

    void apply() const;
    FilterElementsOptions elementOptions() const;

    Ms::System* elementSystem(const Element* element) const;

    const Element* m_element = nullptr;
};
} // namespace Ms
#endif
