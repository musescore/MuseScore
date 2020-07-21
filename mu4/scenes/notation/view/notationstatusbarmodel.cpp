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

#include "notationstatusbarmodel.h"

#include "domain/notation/inotationaccessibility.h"

using namespace mu::scene::notation;
using namespace mu::domain::notation;

void NotationStatusBarModel::load()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto currentNotation = globalContext()->currentNotation();

        if (!currentNotation) {
            return;
        }

        mu::ValCh<std::string> accessibiltyInfoCh = currentNotation->accessibility()->accessibilityInfo();
        setAccessibilityInfo(accessibiltyInfoCh.val);

        accessibiltyInfoCh.ch.onReceive(this, [this](std::string info) {
            setAccessibilityInfo(info);
        });
    });

    mu::ValCh<int> zoomCh = configuration()->currentZoom();
    setCurrentZoom(zoomCh.val);

    zoomCh.ch.onReceive(this, [this](int zoom) {
        setCurrentZoom(zoom);
    });
}

QString NotationStatusBarModel::accessibilityInfo() const
{
    return m_accessibilityInfo;
}

void NotationStatusBarModel::setAccessibilityInfo(const std::string& info)
{
    m_accessibilityInfo = QString::fromStdString(info);
    accessibilityInfoChanged(m_accessibilityInfo);
}

int NotationStatusBarModel::currentZoom() const
{
    return m_currentZoom;
}

void NotationStatusBarModel::setCurrentZoom(int zoom)
{
    m_currentZoom = zoom;
    currentZoomChanged(zoom);
}

void NotationStatusBarModel::zoomIn()
{
    dispatcher()->dispatch("zoomin");
}

void NotationStatusBarModel::zoomOut()
{
    dispatcher()->dispatch("zoomout");
}
