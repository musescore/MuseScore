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

#ifndef __SYMBOLDIALOG_H__
#define __SYMBOLDIALOG_H__

#include "ui_symboldialog.h"

#include "modularity/ioc.h"
#include "engraving/iengravingfontsprovider.h"
#include "context/iglobalcontext.h"

namespace mu::palette {
class PaletteWidget;

//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

class SymbolDialog : public QWidget, Ui::SymbolDialogBase
{
    Q_OBJECT
    INJECT(mu::context::IGlobalContext, globalContext)
    INJECT(engraving::IEngravingFontsProvider, engravingFonts)

public:
    SymbolDialog(const QString&, QWidget* parent = 0);

private slots:
    void systemFlagChanged(Qt::CheckState);
    void systemFontChanged(int);
    void on_search_textChanged(const QString& searchPhrase);
    void on_clearSearch_clicked();

protected:
    virtual void changeEvent(QEvent* event);
    void retranslate() { retranslateUi(this); }

private:
    void createSymbolPalette();
    void createSymbols();

    QString range;
    PaletteWidget* m_symbolsWidget = nullptr;
};
}

#endif
