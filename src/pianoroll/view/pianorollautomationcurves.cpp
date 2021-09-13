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

#include "pianorollautomationcurves.h"

#include <QPainter>

#include "audio/iplayer.h"

using namespace mu::pianoroll;

PianorollAutomationCurves::PianorollAutomationCurves(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

void PianorollAutomationCurves::load()
{
//    onNotationChanged();
//    globalContext()->currentNotationChanged().onNotify(this, [this]() {
//        onNotationChanged();
//    });

//    playback()->player()->playbackPositionMsecs().onReceive(this,
//                                                            [this](audio::TrackSequenceId currentTrackSequence,
//                                                                   const audio::msecs_t newPosMsecs) {
//        int tick = score()->utime2utick(newPosMsecs / 1000.);
//        setPlaybackPosition(Ms::Fraction::fromTicks(tick));
//    });
}

void PianorollAutomationCurves::paint(QPainter* p)
{
//    p->fillRect(0, 0, width(), height(), m_colorBackground);
}
