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
using PanelTypeList = std::vector<PanelType>;

inline std::string panelActionCode(PanelType panelType)
{
    switch (panelType) {
    case PanelType::Palette: return "toggle-palette";
    case PanelType::Instruments: return "toggle-instruments";
    case PanelType::Inspector: return "inspector";
    case PanelType::NotationToolBar: return "toggle-notationtoolbar";
    case PanelType::NoteInputBar: return "toggle-noteinput";
    case PanelType::UndoRedoToolBar: return "toggle-undoredo";
    case PanelType::NotationNavigator: return "toggle-navigator";
    case PanelType::NotationStatusBar: return "toggle-statusbar";
    case PanelType::PlaybackToolBar: return "toggle-transport";
    case PanelType::Mixer: return "toggle-mixer";
    case PanelType::TimeLine: return "toggle-timeline";
    case PanelType::Synthesizer: return "synth-control";
    case PanelType::SelectionFilter: return "toggle-selection-window";
    case PanelType::Piano: return "toggle-piano";
    case PanelType::ComparisonTool: return "toggle-scorecmp-tool";
    }

    return "";
}
}

#endif // MU_APPSHELL_APPSHELLTYPES_H
