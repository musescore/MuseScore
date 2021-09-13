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

#ifndef MU_PIANOROLL_PIANOROLLAUTOMATIONCURVES_H
#define MU_PIANOROLL_PIANOROLLAUTOMATIONCURVES_H

#include <QQuickPaintedItem>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "audio/iplayback.h"
#include "pianoroll/ipianorollcontroller.h"

namespace mu::pianoroll {

class PianorollAutomationCurves : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)
    INJECT(playback, audio::IPlayback, playback)

//    Q_PROPERTY(double wholeNoteWidth READ wholeNoteWidth WRITE setWholeNoteWidth NOTIFY wholeNoteWidthChanged)
//    Q_PROPERTY(double centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
//    Q_PROPERTY(double displayObjectWidth READ displayObjectWidth WRITE setDisplayObjectWidth NOTIFY displayObjectWidthChanged)
//    Q_PROPERTY(int tuplet READ tuplet WRITE setTuplet NOTIFY tupletChanged)
//    Q_PROPERTY(int subdivision READ subdivision WRITE setSubdivision NOTIFY subdivisionChanged)
//    Q_PROPERTY(AutomationType automationType READ automationType WRITE setAutomationType NOTIFY automationTypeChanged)

public:
    PianorollAutomationCurves(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();

    void paint(QPainter*) override;
};
}

#endif // MU_PIANOROLL_PIANOROLLAUTOMATIONCURVES_H
