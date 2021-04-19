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

#ifndef __KEYEDIT_H__
#define __KEYEDIT_H__

#include "ui_keyedit.h"

#include "modularity/ioc.h"
#include "../ipaletteconfiguration.h"

namespace Ms {
class Palette;
class PaletteScrollArea;
class KeySig;

//---------------------------------------------------------
//   KeyEditor
//---------------------------------------------------------

class KeyEditor : public QWidget, Ui::KeyEdit
{
    Q_OBJECT

    INJECT(palette, mu::palette::IPaletteConfiguration, configuration)

    PaletteScrollArea* _keyPalette = nullptr;
    Palette* sp = nullptr;
    Palette* sp1 = nullptr;
    bool _dirty = false;

private slots:
    void addClicked();
    void clearClicked();
    void setDirty() { _dirty = true; }

signals:
    void keySigAdded(const std::shared_ptr<KeySig>);

public:
    KeyEditor(QWidget* parent = 0);
    bool dirty() const { return _dirty; }
    void showKeyPalette(bool val);
    void save();
};
} // namespace Ms
#endif
