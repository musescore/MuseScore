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

#ifndef MU_NOTATION_BREAKSDIALOG_H
#define MU_NOTATION_BREAKSDIALOG_H

#include "ui_breaksdialog.h"

#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

namespace mu::notation {
//---------------------------------------------------------
//   BreaksDialog
//---------------------------------------------------------

class BreaksDialog : public QDialog, public Ui::BreaksDialog
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)

public:
    BreaksDialog(QWidget* parent = nullptr);
    BreaksDialog(const BreaksDialog& dialog);

private slots:
    void accept() override;

private:
    void showEvent(QShowEvent*) override;

    bool _allSelected = false;
};
}

Q_DECLARE_METATYPE(mu::notation::BreaksDialog)

#endif // MU_NOTATION_BREAKSDIALOG_H
