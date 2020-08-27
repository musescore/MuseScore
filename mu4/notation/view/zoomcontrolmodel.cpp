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

#include "zoomcontrolmodel.h"

using namespace mu::notation;

void ZoomControlModel::load()
{
    mu::ValCh<int> zoomCh = configuration()->currentZoom();
    setCurrentZoom(zoomCh.val);

    zoomCh.ch.onReceive(this, [this](int zoom) {
        setCurrentZoom(zoom);
    });
}

int ZoomControlModel::currentZoom() const
{
    return m_currentZoom;
}

void ZoomControlModel::setCurrentZoom(int zoom)
{
    if (m_currentZoom == zoom) {
        return;
    }

    m_currentZoom = zoom;
    currentZoomChanged(zoom);
}

void ZoomControlModel::zoomIn()
{
    dispatcher()->dispatch("zoomin");
}

void ZoomControlModel::zoomOut()
{
    dispatcher()->dispatch("zoomout");
}
