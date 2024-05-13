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

#ifndef __KEYEDIT_H__
#define __KEYEDIT_H__

#include "ui_keyedit.h"

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"
#include "internal/ipaletteprovider.h"

namespace mu::palette {
class PaletteWidget;
class PaletteScrollArea;

class KeyEditor : public QWidget, Ui::KeyEdit
{
    Q_OBJECT

    Q_PROPERTY(bool showKeyPalette READ showKeyPalette WRITE setShowKeyPalette)

    INJECT(IPaletteConfiguration, configuration)
    INJECT(IPaletteProvider, paletteProvider)

public:
    KeyEditor(QWidget* parent = 0);

    bool dirty() const { return m_dirty; }
    void save();

    bool showKeyPalette() const;

public slots:
    void setShowKeyPalette(bool showKeyPalette);

private slots:
    void addClicked();
    void clearClicked();
    void setDirty() { m_dirty = true; }

private:
    PaletteScrollArea* m_keySigArea = nullptr;
    PaletteWidget* m_keySigPaletteWidget = nullptr;
    PaletteWidget* m_accidentalsPaletteWidget = nullptr;

    bool m_dirty = false;
};

static const int KEYEDIT_ACC_ZERO_POINT = 3;
}

#endif
