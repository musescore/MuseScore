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

#ifndef __SYMBOLDIALOG_H__
#define __SYMBOLDIALOG_H__

#include "ui_symboldialog.h"

namespace Ms {
class Palette;
class Element;

//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

class SymbolDialog : public QWidget, Ui::SymbolDialogBase
{
    Q_OBJECT

    QString range;
    Palette* sp;
    void createSymbolPalette();
    void createSymbols();

private slots:
    void systemFlagChanged(int);
    void systemFontChanged(int);
    void on_search_textChanged(const QString& searchPhrase);
    void on_clearSearch_clicked();

protected:
    virtual void changeEvent(QEvent* event);
    void retranslate() { retranslateUi(this); }

public:
    SymbolDialog(const QString&, QWidget* parent = 0);
};
}

#endif
