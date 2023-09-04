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
#ifndef MU_NOTATION_EDITSTRINGDATA_H
#define MU_NOTATION_EDITSTRINGDATA_H

#include "ui_editstringdata.h"
#include "engraving/dom/stringdata.h"

namespace mu::notation {
//---------------------------------------------------------
//   EditStringData
//---------------------------------------------------------

class EditStringData : public QDialog, private Ui::EditStringDataBase
{
    Q_OBJECT

public:
    EditStringData(QWidget* parent, std::vector<mu::engraving::instrString>* strings, int* frets);
    ~EditStringData();

protected:
    QString midiCodeToStr(int midiCode);

private slots:
    void accept() override;
    void deleteStringClicked();
    void editStringClicked();
    void listItemClicked(QTableWidgetItem* item);
    void newStringClicked();

private:
    virtual void hideEvent(QHideEvent*) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    QString openColumnAccessibleText(const QTableWidgetItem* item) const;

    int* _frets = nullptr;
    bool _modified = false;
    std::vector<mu::engraving::instrString>* _strings;           // pointer to original string list
    std::vector<mu::engraving::instrString> _stringsLoc;         // local working copy of string list
};
}

#endif // MU_NOTATION_EDITSTRINGDATA_H
