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
#ifndef MU_NOTATION_EDITSTRINGDATA_H
#define MU_NOTATION_EDITSTRINGDATA_H

#include "ui_editstringdata.h"
#include "libmscore/stringdata.h"

namespace mu::notation {
//---------------------------------------------------------
//   EditStringData
//---------------------------------------------------------

class EditStringData : public QDialog, private Ui::EditStringDataBase
{
    Q_OBJECT

    int* _frets;
    bool _modified;
    QList<Ms::instrString>* _strings;           // pointer to original string list
    QList<Ms::instrString> _stringsLoc;         // local working copy of string list

    virtual void hideEvent(QHideEvent*);

public:
    EditStringData(QWidget* parent, QList<Ms::instrString>* strings, int* frets);
    ~EditStringData();

protected:
    QString midiCodeToStr(int midiCode);

private slots:
    void accept();
    void deleteStringClicked();
    void editStringClicked();
    void listItemClicked(QTableWidgetItem* item);
    void newStringClicked();
};
}

#endif // MU_NOTATION_EDITSTRINGDATA_H
