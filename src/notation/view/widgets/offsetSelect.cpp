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

#include "offsetSelect.h"

#include "libmscore/types.h"

using namespace mu::notation;

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

void OffsetSelect::setSuffix(const QString& s)
{
    xVal->setSuffix(s);
    yVal->setSuffix(s);
}

void OffsetSelect::showRaster(bool v)
{
    hRaster->setVisible(v);
    vRaster->setVisible(v);
}

void OffsetSelect::_offsetChanged()
{
    emit offsetChanged(QPointF(xVal->value(), yVal->value()));
}

QPointF OffsetSelect::offset() const
{
    return QPointF(xVal->value(), yVal->value());
}

void OffsetSelect::blockOffset(bool val)
{
    xVal->blockSignals(val);
    yVal->blockSignals(val);
}

void OffsetSelect::setOffset(const QPointF& o)
{
    blockOffset(true);
    xVal->setValue(o.x());
    yVal->setValue(o.y());
    blockOffset(false);
}
