/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_APPSHELL_APPSHELLTYPES_H
#define MU_APPSHELL_APPSHELLTYPES_H

#include <QString>

namespace mu::appshell {
using DockName = QString;

// Panels:
static const DockName PALETTES_PANEL_NAME("palettesPanel");
static const DockName INSTRUMENTS_PANEL_NAME("instrumentsPanel");
static const DockName INSPECTOR_PANEL_NAME("inspectorPanel");
static const DockName SELECTION_FILTERS_PANEL_NAME("selectionFiltersPanel");

static const DockName NOTATION_NAVIGATOR_PANEL_NAME("notationNavigatorPanel");
static const DockName NOTATION_BRAILLE_PANEL_NAME("notationBraillePanel");

static const DockName MIXER_PANEL_NAME("mixerPanel");
static const DockName PIANO_KEYBOARD_PANEL_NAME("pianoKeyboardPanel");
static const DockName TIMELINE_PANEL_NAME("timelinePanel");
static const DockName DRUMSET_PANEL_NAME("drumsetPanel");
static const DockName PERCUSSION_PANEL_NAME("percussionPanel");

// Toolbars:
static const DockName NOTATION_TOOLBAR_NAME("notationToolBar");
static const DockName UNDO_REDO_TOOLBAR_NAME("undoRedoToolBar");
static const DockName NOTE_INPUT_BAR_NAME("noteInputBar");
static const DockName PLAYBACK_TOOLBAR_NAME("playbackToolBar");

// Other:
static const DockName NOTATION_STATUSBAR_NAME("notationStatusBar");

enum class StartupModeType
{
    StartEmpty,
    ContinueLastSession,
    StartWithNewScore,
    StartWithScore,
    Recovery
};
}

#endif // MU_APPSHELL_APPSHELLTYPES_H
