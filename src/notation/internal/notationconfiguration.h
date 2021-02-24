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
#include "async/asyncable.h"
#include "ui/iuiconfiguration.h"
#include "iglobalconfiguration.h"
#include "settings.h"
#include "iworkspacesettings.h"

namespace mu::notation {
class NotationConfiguration : public INotationConfiguration, public async::Asyncable
{
    INJECT(notation, ui::IUiConfiguration, uiConfiguration)
    INJECT(notation, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(notation, framework::IWorkspaceSettings, workspaceSettings)

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
    async::Channel<QColor> foregroundColorChanged() const override;
    io::path foregroundWallpaper() const override;

    QColor playbackCursorColor() const override;
    QColor loopMarkerColor() const override;
    int cursorOpacity() const override;

    QColor selectionColor(int voiceIndex = 0) const override;

    QColor layoutBreakColor() const override;

    int selectionProximity() const override;

    ValCh<int> currentZoom() const override;
    void setCurrentZoom(int zoomPercentage) override;

    std::string fontFamily() const override;
    int fontSize() const override;

    io::path stylesDirPath() const override;

    bool isMidiInputEnabled() const override;
    void setIsMidiInputEnabled(bool enabled) override;

    bool isAutomaticallyPanEnabled() const override;
    void setIsAutomaticallyPanEnabled(bool enabled) override;

    bool isPlayRepeatsEnabled() const override;
    void setIsPlayRepeatsEnabled(bool enabled) override;

    bool isMetronomeEnabled() const override;
    void setIsMetronomeEnabled(bool enabled) override;

    bool isCountInEnabled() const override;
    void setIsCountInEnabled(bool enabled) override;

    float guiScaling() const override;
    float notationScaling() const override;

    std::string notationRevision() const override;

    std::vector<std::string> toolbarActions(const std::string& toolbarName) const override;
    void setToolbarActions(const std::string& toolbarName, const std::vector<std::string>& actions) override;

    ValCh<framework::Orientation> canvasOrientation() const override;
    void setCanvasOrientation(framework::Orientation orientation) override;

private:
    std::vector<std::string> parseToolbarActions(const std::string& actions) const;

    framework::Settings::Key toolbarSettingsKey(const std::string& toolbarName) const;

    async::Channel<QColor> m_backgroundColorChanged;
    async::Channel<QColor> m_foregroundColorChanged;
    async::Channel<int> m_currentZoomChanged;
    async::Channel<framework::Orientation> m_canvasOrientationChanged;
};
}

#endif // MU_NOTATION_NOTATIONCONFIGURATION_H
