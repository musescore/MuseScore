//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "offsetSelect.h"

#include "libmscore/types.h"
#include "mscore/musescore.h"

using namespace mu::notation;

//---------------------------------------------------------
//   OffsetSelect
//---------------------------------------------------------

OffsetSelect::OffsetSelect(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    showRaster(false);

    //! TODO Need to port for MU4
//    QAction* a = Ms::getAction("hraster");
//    if (a) {
//        a->setCheckable(true);
//        hRaster->setDefaultAction(a);
//        hRaster->setContextMenuPolicy(Qt::ActionsContextMenu);
//        hRaster->addAction(Ms::getAction("config-raster"));
//    }

//    a = Ms::getAction("vraster");
//    if (a) {
//        a->setCheckable(true);
//        vRaster->setDefaultAction(a);
//        vRaster->setContextMenuPolicy(Qt::ActionsContextMenu);
//        vRaster->addAction(Ms::getAction("config-raster"));
//    }
    //!---

    connect(xVal, SIGNAL(valueChanged(double)), SLOT(_offsetChanged()));
    connect(yVal, SIGNAL(valueChanged(double)), SLOT(_offsetChanged()));
}

//---------------------------------------------------------
//   setSuffix
//---------------------------------------------------------

void OffsetSelect::setSuffix(const QString& s)
{
    xVal->setSuffix(s);
    yVal->setSuffix(s);
}

//---------------------------------------------------------
//   showRaster
//---------------------------------------------------------

void OffsetSelect::showRaster(bool v)
{
    hRaster->setVisible(v);
    vRaster->setVisible(v);
}

//---------------------------------------------------------
//   _offsetChanged
//---------------------------------------------------------

void OffsetSelect::_offsetChanged()
{
    emit offsetChanged(QPointF(xVal->value(), yVal->value()));
}

//---------------------------------------------------------
//   offset
//---------------------------------------------------------

QPointF OffsetSelect::offset() const
{
    return QPointF(xVal->value(), yVal->value());
}

//---------------------------------------------------------
//   blockOffset
//---------------------------------------------------------

void OffsetSelect::blockOffset(bool val)
{
    xVal->blockSignals(val);
    yVal->blockSignals(val);
}

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void OffsetSelect::setOffset(const QPointF& o)
{
    blockOffset(true);
    xVal->setValue(o.x());
    yVal->setValue(o.y());
    blockOffset(false);
}
