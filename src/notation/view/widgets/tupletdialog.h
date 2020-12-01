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
#ifndef MU_NOTATION_TUPLETDIALOG_H
#define MU_NOTATION_TUPLETDIALOG_H

#include "libmscore/duration.h"
#include "ui_tupletdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------
namespace mu::notation {
class TupletDialog : public QDialog, Ui::TupletDialog
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, globalContext)

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
