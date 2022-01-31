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

#ifndef __KEYCANVAS_H__
#define __KEYCANVAS_H__

#include <QFrame>

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"
#include "iengravingconfiguration.h"

namespace Ms {
class Accidental;
class Clef;

//---------------------------------------------------------
//   KeyCanvas
//---------------------------------------------------------

class KeyCanvas : public QFrame
{
    Q_OBJECT

    INJECT(palette, mu::palette::IPaletteConfiguration, configuration)
    INJECT(notation, mu::engraving::IEngravingConfiguration, engravingConfiguration)

    Accidental* dragElement = nullptr;
    Accidental* moveElement = nullptr;
    QTransform _matrix, imatrix;
    double extraMag = 1.0;
    QList<Accidental*> accidentals;
    QPointF startMove;
    QPointF base;
    Clef* clef = nullptr;

    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dropEvent(QDropEvent*);
    void snap(Accidental*);

private slots:
    void deleteElement();

public:
    KeyCanvas(QWidget* parent = 0);
    void clear();
    const QList<Accidental*> getAccidentals() const { return accidentals; }
};
} // namespace Ms
#endif
