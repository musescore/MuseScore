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


#ifndef MU_PIANOROLL_PIANOROLLGENERAL_H
#define MU_PIANOROLL_PIANOROLLGENERAL_H

#include <QObject>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "pianoroll/ipianorollcontroller.h"

namespace mu::pianoroll {

class PianorollGeneral: public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)

    Q_PROPERTY(double xZoom READ xZoom WRITE setXZoom NOTIFY xZoomChanged)
    Q_PROPERTY(int noteHeight READ noteHeight WRITE setNoteHeight NOTIFY noteHeightChanged)

public:
    PianorollGeneral(QObject* parent = nullptr);


    Q_INVOKABLE void load();

    double xZoom() const { return m_xZoom; }
    void setXZoom(double value);
    int noteHeight() const { return m_noteHeight; }
    void setNoteHeight(int value);

signals:
    void xZoomChanged();
    void noteHeightChanged();

private:

    double m_xZoom;
    int m_noteHeight;

};

}

#endif // MU_PIANOROLL_PIANOROLLGENERAL_H
