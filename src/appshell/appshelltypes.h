//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_APPSHELL_APPSHELLTYPES_H
#define MU_APPSHELL_APPSHELLTYPES_H

namespace mu::appshell {
enum class PanelType
{
    Palette,
    Instruments,
    Inspector,
    NotationToolBar,
    NoteInputBar,
    UndoRedoToolBar,
    NotationNavigator,
    NotationStatusBar,
    PlaybackToolBar,
    Mixer,
    TimeLine,
    Synthesizer,
    SelectionFilter,
    Piano,
    ComparisonTool
};

inline std::string panelActionCode(PanelType panelType)
{
    std::map<PanelType, std::string> panelTypeStrings {
        { PanelType::Palette, "toggle-palette" },
        { PanelType::Instruments, "toggle-instruments" },
        { PanelType::Inspector, "inspector" },
        { PanelType::NotationToolBar, "toggle-notationtoolbar" },
        { PanelType::NoteInputBar, "toggle-noteinput" },
        { PanelType::UndoRedoToolBar, "toggle-undoredo" },
        { PanelType::NotationNavigator, "toggle-navigator" },
        { PanelType::NotationStatusBar, "toggle-statusbar" },
        { PanelType::PlaybackToolBar, "toggle-transport" },
        { PanelType::Mixer, "toggle-mixer" },
        { PanelType::TimeLine, "toggle-timeline" },
        { PanelType::Synthesizer, "synth-control" },
        { PanelType::SelectionFilter, "toggle-selection-window" },
        { PanelType::Piano, "toggle-piano" },
        { PanelType::ComparisonTool, "toggle-scorecmp-tool" }
    };

    return panelTypeStrings[panelType];
}
}

#endif // MU_APPSHELL_APPSHELLTYPES_H
