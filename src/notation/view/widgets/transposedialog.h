//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef MU_NOTATION_TRANSPOSEDIALOG_H
#define MU_NOTATION_TRANSPOSEDIALOG_H

#include "ui_transposedialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "notationtypes.h"

namespace mu::notation {
//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

class TransposeDialog : public QDialog, Ui::TransposeDialogBase
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)

public:
    TransposeDialog(QWidget* parent = 0);
    TransposeDialog(const TransposeDialog& dialog);

private slots:
    void transposeByKeyToggled(bool);
    void transposeByIntervalToggled(bool);
    void chromaticBoxToggled(bool val);
    void diatonicBoxToggled(bool val);
    void apply();

private:
    void hideEvent(QHideEvent*) override;

    INotationPtr notation() const;
    INotationSelectionPtr selection() const;
    INotationInteractionPtr interaction() const;

    Key firstPitchedStaffKey() const;
    void setEnableTransposeKeys(bool val);
    void setEnableTransposeToKey(bool val);
    void setEnableTransposeChordNames(bool val);
    bool getTransposeKeys() const;
    bool getTransposeChordNames() const;
    Key transposeKey() const;
    int transposeInterval() const;
    TransposeDirection direction() const;
    TransposeMode mode() const;
    void setKey(Key k);
    bool useDoubleSharpsFlats() const;

    bool m_allSelected = false;
};
}

Q_DECLARE_METATYPE(mu::notation::TransposeDialog)

#endif // MU_NOTATION_TRANSPOSEDIALOG_H
