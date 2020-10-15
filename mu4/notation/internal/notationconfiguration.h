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
#ifndef MU_NOTATION_NOTATIONCONFIGURATION_H
#define MU_NOTATION_NOTATIONCONFIGURATION_H

#include "../inotationconfiguration.h"
#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "iglobalconfiguration.h"

namespace mu {
namespace notation {
class NotationConfiguration : public INotationConfiguration
{
    INJECT(notation, framework::IUiConfiguration, uiConfiguration)
    INJECT(notation, framework::IGlobalConfiguration, globalConfiguration)

public:
    void init();

    QColor anchorLineColor() const override;

    QColor backgroundColor() const override;
    async::Channel<QColor> backgroundColorChanged() const override;

    QColor pageColor() const override;
    QColor borderColor() const override;
    int borderWidth() const override;

    bool foregroundUseColor() const override;
    QColor foregroundColor() const override;
    QColor defaultForegroundColor() const override;
    async::Channel<QColor> foregroundColorChanged() const override;
    io::path foregroundWallpaper() const override;

    QColor playbackCursorColor() const override;

    int selectionProximity() const override;

    ValCh<int> currentZoom() const override;
    void setCurrentZoom(int zoomPercentage) override;

    int fontSize() const override;

    io::path stylesDirPath() const override;

    bool isMidiInputEnabled() const override;

    float guiScaling() const override;

private:
    async::Channel<QColor> m_backgroundColorChanged;
    async::Channel<QColor> m_foregroundColorChanged;
    async::Channel<int> m_currentZoomChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONCONFIGURATION_H
