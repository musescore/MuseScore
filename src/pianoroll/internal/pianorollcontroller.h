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
#ifndef MU_PIANOROLL_PIANOROLLCONTROLLER_H
#define MU_PIANOROLL_PIANOROLLCONTROLLER_H

#include <unordered_map>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "notation/notationtypes.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationplayback.h"
#include "audio/iplayer.h"
#include "audio/itracks.h"
#include "audio/iaudiooutput.h"
#include "audio/iplayback.h"
#include "audio/audiotypes.h"

#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/measure.h"

#include "../ipianorollcontroller.h"
#include "../ipianorollconfiguration.h"

namespace mu::pianoroll {
static const int PIANO_KEYBOARD_WIDTH = 100;
static const int BLACK_KEY_WIDTH = PIANO_KEYBOARD_WIDTH * 9 / 14;
const int MAX_KEY_HEIGHT = 20;
const int MIN_KEY_HEIGHT = 8;
const int DEFAULT_KEY_HEIGHT = 14;
const int BEAT_WIDTH_IN_PIXELS = 50;
const double X_ZOOM_RATIO = 1.1;
const double X_ZOOM_INITIAL = 0.1;

class PianorollController : public QObject, public IPianorollController, public actions::Actionable, public async::Asyncable
{
    Q_OBJECT

    INJECT(pianoroll, actions::IActionsDispatcher, dispatcher)
    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollConfiguration, configuration)
//    INJECT(pianoroll, notation::INotationConfiguration, notationConfiguration)
    INJECT(pianoroll, audio::IPlayback, playback)

    Q_PROPERTY(double xZoom READ xZoom WRITE setXZoom NOTIFY xZoomChanged)
    Q_PROPERTY(int noteHeight READ noteHeight WRITE setNoteHeight NOTIFY noteHeightChanged)

    const int MAX_VOICES = 4;

public:
    void init();

    int getNotes() const;

    async::Notification noteLayoutChanged() const override;

    double xZoom() const { return m_xZoom; }
    void setXZoom(double value);
    int noteHeight() const { return m_noteHeight; }
    void setNoteHeight(int value);

    int widthInTicks() const { return m_widthInTicks; }
    mu::engraving::Fraction widthInBeats();

    async::Notification pitchHighlightChanged() const override;
    bool isPitchHighlight(int pitch) const override { return m_pitchHighlight[pitch]; }
    void setPitchHighlight(int pitch, bool value) override;

signals:
    void xZoomChanged();
    void noteHeightChanged();
    void pitchHighlightChanged();

private:

    void onUndoStackChanged();
    void onCurrentNotationChanged();
    void onNotationChanged();
    void onSelectionChanged();

    void buildNoteBlocks();
    void addChord(engraving::Chord* chrd, int voice);

    engraving::Measure* lastMeasure();
    engraving::Score* score();

    bool m_pitchHighlight[128];

    std::vector<int> m_selectedStaves;
    int m_activeStaff = -1;

    async::Notification m_noteLayoutChanged;
    async::Notification m_pitchHighlightChanged;

    double m_xZoom = X_ZOOM_INITIAL;
    int m_widthInTicks;
    int m_noteHeight = DEFAULT_KEY_HEIGHT;
};
}

#endif // MU_PIANOROLL_PIANOROLLCONTROLLER_H
