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

#include "ui_masterpalette.h"

#include "internal/palette.h"

#include "uicomponents/view/topleveldialog.h"

namespace mu::palette {
class TimeDialog;
class KeyEditor;
class SymbolDialog;

class MasterPalette : public muse::uicomponents::TopLevelDialog, Ui::MasterPalette
{
    Q_OBJECT

    Q_PROPERTY(QString selectedPaletteName READ selectedPaletteName WRITE setSelectedPaletteName NOTIFY selectedPaletteNameChanged)

public:
    explicit MasterPalette(QWidget* parent = nullptr);

    QString selectedPaletteName() const;

public slots:
    void setSelectedPaletteName(const QString& name);

signals:
    void selectedPaletteNameChanged(QString name);

private slots:
    void currentChanged(QTreeWidgetItem*, QTreeWidgetItem*);
    void clicked(QTreeWidgetItem*, int);

    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void addPalette(PalettePtr palette);
    void retranslate(bool firstTime = false);

    TimeDialog* m_timeDialog = nullptr;
    KeyEditor* m_keyEditor = nullptr;
    QTreeWidgetItem* m_keyItem = nullptr;
    QTreeWidgetItem* m_timeItem = nullptr;
    QTreeWidgetItem* m_symbolItem = nullptr;

    int m_idxAllSymbols = -1;
    QHash<int, SymbolDialog*> m_symbolWidgets;
};
}
