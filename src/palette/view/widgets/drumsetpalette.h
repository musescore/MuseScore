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

#ifndef MU_PALETTE_DRUMSETPALETTE_H
#define MU_PALETTE_DRUMSETPALETTE_H

#include "palettewidget.h"
#include "notation/inotation.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "playback/iplaybackcontroller.h"
#include "engraving/rendering/isinglerenderer.h"

namespace mu::engraving {
class Drumset;
}

namespace mu::palette {
class DrumsetPalette : public PaletteScrollArea
{
    Q_OBJECT

    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(playback::IPlaybackController, playback)
    INJECT(engraving::rendering::ISingleRenderer, engravingRenderer)

public:
    explicit DrumsetPalette(QWidget* parent = nullptr);

    void setNotation(mu::notation::INotationPtr notation);
    void updateDrumset();
    bool handleEvent(QEvent* event);

    muse::async::Channel<QString> pitchNameChanged() const;

    PaletteWidget* paletteWidget() const;

private slots:
    void drumNoteClicked(int val);

private:
    void clear();

    void changeEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    int selectedDrumNote();
    void retranslate();

    void previewSound(const mu::engraving::Chord* chord, bool newChordSelected, const notation::NoteInputState& inputState);

    mu::notation::INotationNoteInputPtr noteInput() const;

    PaletteWidget* m_drumPalette = nullptr;
    const mu::engraving::Drumset* m_drumset = nullptr;
    mu::notation::INotationPtr m_notation;
    muse::async::Channel<QString> m_pitchNameChanged;
};
}

#endif // MU_PALETTE_DRUMSETPALETTE_H
