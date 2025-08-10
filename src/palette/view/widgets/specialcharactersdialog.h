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
#pragma once

#include "ui_specialcharactersdialog.h"

#include "draw/types/font.h"
#include "uicomponents/view/topleveldialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

class QListWidget;

namespace mu::engraving {
class TextBase;
}

namespace mu::palette {
class PaletteWidget;

class SpecialCharactersDialog : public muse::uicomponents::TopLevelDialog, public Ui::SpecialCharactersDialog
{
    Q_OBJECT

    INJECT(mu::context::IGlobalContext, globalContext)

public:
    SpecialCharactersDialog(QWidget* parent = nullptr);

private slots:
    void populateSmufl();
    void populateUnicode();

private:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

    void setFont(const muse::draw::Font& font);
    void populateCommon();

    muse::draw::Font m_font;
    PaletteWidget* m_pCommon = nullptr;
    PaletteWidget* m_pSmufl = nullptr;
    PaletteWidget* m_pUnicode = nullptr;
    QListWidget* m_lws = nullptr;
    QListWidget* m_lwu = nullptr;
};
}
