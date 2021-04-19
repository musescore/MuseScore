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

#ifndef __TIMEDIALOG_H__
#define __TIMEDIALOG_H__

#include "ui_timedialog.h"
#include "libmscore/fraction.h"

#include "modularity/ioc.h"
#include "../ipaletteconfiguration.h"

namespace Ms {
class Palette;
class PaletteScrollArea;
class TimeSig;
class Score;
class Chord;

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

class TimeDialog : public QWidget, Ui::TimeDialogBase
{
    Q_OBJECT

    INJECT(palette, mu::palette::IPaletteConfiguration, configuration)

    PaletteScrollArea* _timePalette = nullptr;
    Palette* sp = nullptr;
    bool _dirty = false;

    int denominator() const;
    int denominator2Idx(int) const;

private slots:
    void addClicked();
    void zChanged(int);
    void nChanged(int);
    void paletteChanged(int idx);
    void textChanged();
    void setDirty() { _dirty = true; }

signals:
    void timeSigAdded(const std::shared_ptr<TimeSig>);

public:
    TimeDialog(QWidget* parent = 0);
    bool dirty() const { return _dirty; }
    void showTimePalette(bool val);
    void save();
};
}

#endif
