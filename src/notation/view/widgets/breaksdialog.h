//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2014 Werner Schweer and others
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

    INJECT(notation, context::IGlobalContext, context)

public:
    BreaksDialog(QWidget* parent = nullptr);
    BreaksDialog(const BreaksDialog& dialog);

private slots:
    void accept() override;
};
}

Q_DECLARE_METATYPE(mu::notation::BreaksDialog)

#endif // MU_NOTATION_BREAKSDIALOG_H
