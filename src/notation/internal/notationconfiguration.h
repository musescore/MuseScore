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
#include "system/ifilesystem.h"

namespace mu::notation {
class NotationConfiguration : public INotationConfiguration, public async::Asyncable
{
    INJECT(notation, ui::IUiConfiguration, uiConfiguration)
    INJECT(notation, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(notation, framework::IWorkspaceSettings, workspaceSettings)
    INJECT(notation, system::IFileSystem, fileSystem)

public:
    void init();

    QColor anchorLineColor() const override;

    QColor backgroundColor() const override;
    void setBackgroundColor(const QColor& color) override;

    io::path backgroundWallpaperPath() const override;
    void setBackgroundWallpaperPath(const io::path& path) override;

    bool backgroundUseColor() const override;
    void setBackgroundUseColor(bool value) override;

    async::Notification backgroundChanged() const override;

    QColor foregroundColor() const override;
    void setForegroundColor(const QColor& color) override;

    io::path foregroundWallpaperPath() const override;
    void setForegroundWallpaperPath(const io::path& path) override;

    bool foregroundUseColor() const override;
    void setForegroundUseColor(bool value) override;

    async::Notification foregroundChanged() const override;

    io::path wallpapersDefaultDirPath() const override;

    QColor borderColor() const override;
    int borderWidth() const override;

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

    ValCh<io::path> stylesPath() const override;
    void setStylesPath(const io::path& path) override;

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

    bool advanceToNextNoteOnKeyRelease() const override;
    void setAdvanceToNextNoteOnKeyRelease(bool value) override;

    bool colorNotesOusideOfUsablePitchRange() const override;
    void setColorNotesOusideOfUsablePitchRange(bool value) override;

    int delayBetweenNotesInRealTimeModeMilliseconds() const override;
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) override;

    int notePlayDurationMilliseconds() const override;
    void setNotePlayDurationMilliseconds(int durationMs) override;

private:
    std::vector<std::string> parseToolbarActions(const std::string& actions) const;

    framework::Settings::Key toolbarSettingsKey(const std::string& toolbarName) const;

    async::Notification m_backgroundChanged;
    async::Notification m_foregroundChanged;
    async::Channel<int> m_currentZoomChanged;
    async::Channel<framework::Orientation> m_canvasOrientationChanged;
    async::Channel<io::path> m_stylesPathChnaged;
};
}

#endif // MU_NOTATION_NOTATIONCONFIGURATION_H
