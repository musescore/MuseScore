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

#ifndef MU_PALETTE_DRUMSETPALETTE_H
#define MU_PALETTE_DRUMSETPALETTE_H

#include "palette/palette.h"
#include "notation/inotation.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

namespace Ms {
class Score;
class Drumset;
class Staff;

class DrumsetPalette : public PaletteScrollArea
{
    Q_OBJECT

    INJECT(Ms, mu::actions::IActionsDispatcher, dispatcher)

public:
    explicit DrumsetPalette(QWidget* parent = nullptr);

    void setNotation(mu::notation::INotationPtr notation);
    void updateDrumset();
    bool handleEvent(QEvent* event);

    mu::async::Channel<QString> pitchNameChanged() const;

private slots:
    void drumNoteSelected(int val);

private:
    void clear();

    void changeEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

    int selectedDrumNote();
    void retranslate();

    mu::notation::INotationNoteInputPtr noteInput() const;

    Palette* m_drumPalette = nullptr;
    const Drumset* m_drumset = nullptr;

    mu::notation::INotationPtr m_notation;
    mu::async::Channel<QString> m_pitchNameChanged;
};
} // namespace Ms

#endif // MU_PALETTE_DRUMSETPALETTE_H
