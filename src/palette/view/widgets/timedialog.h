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

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"
#include "internal/ipaletteprovider.h"
#include "engraving/rendering/isinglerendering.h"

namespace mu::palette {
class PaletteWidget;
class PaletteScrollArea;

class TimeDialog : public QWidget, Ui::TimeDialogBase
{
    Q_OBJECT

    Q_PROPERTY(bool showTimePalette READ showTimePalette WRITE setShowTimePalette)

    INJECT(IPaletteConfiguration, configuration)
    INJECT(IPaletteProvider, paletteProvider)
    INJECT(engraving::rendering::ISingleRendering, engravingRendering)

public:
    TimeDialog(QWidget* parent = 0);
    TimeDialog(const TimeDialog& dialog);

    bool dirty() const;
    bool showTimePalette() const;
    void save();

private slots:
    void addClicked();
    void zChanged(int);
    void nChanged(int);
    void paletteChanged(int idx);
    void textChanged();
    void setDirty();
    void setShowTimePalette(bool val);

private:
    int denominator() const;
    int denominator2Idx(int) const;

    PaletteScrollArea* _timePalette = nullptr;
    PaletteWidget* sp = nullptr;
    bool _dirty = false;
};
}

#endif
