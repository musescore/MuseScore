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

#include "mididevicemappingsmodel.h"

#include "ui/view/iconcodes.h"

#include "log.h"
#include "translation.h"

using namespace mu::midi;
using namespace mu::actions;
using namespace mu::ui;

MidiDeviceMappingsModel::MidiDeviceMappingsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant MidiDeviceMappingsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    ActionItem action = m_actions[index.row()];

    switch (role) {
    case RoleTitle: return QString::fromStdString(action.title);
    case RoleIcon: return static_cast<int>(action.iconCode);
    case RoleStatus: return "Inactive";
    case RoleEnabled: return false;
    }

    return QVariant();
}

int MidiDeviceMappingsModel::rowCount(const QModelIndex&) const
{
    return m_actions.size();
}

QHash<int, QByteArray> MidiDeviceMappingsModel::roleNames() const
{
    return {
        { RoleTitle, "title" },
        { RoleIcon, "icon" },
        { RoleStatus, "status" },
        { RoleEnabled, "enabled" }
    };
}

void MidiDeviceMappingsModel::load()
{
    beginResetModel();
    m_actions.clear();

    for (MidiActionType type : allMidiActionTypes()) {
        ActionCode code = actionCode(type);
        ActionItem action = actionsRegister()->action(code);

        if (action.isValid()) {
            m_actions.push_back(action);
        }
    }

    endResetModel();
}

bool MidiDeviceMappingsModel::apply()
{
    NOT_IMPLEMENTED;
    return true;
}

ActionCode MidiDeviceMappingsModel::actionCode(MidiActionType type) const
{
    switch (type) {
    case MidiActionType::Rewind: return "rewind";
    case MidiActionType::Loop: return "loop";
    case MidiActionType::Play: return "play";
    case MidiActionType::Stop: return "stop";
    case MidiActionType::NoteInputMode: return "note-input";
    case MidiActionType::Note1: return "pad-note-1";
    case MidiActionType::Note2: return "pad-note-2";
    case MidiActionType::Note4: return "pad-note-4";
    case MidiActionType::Note8: return "pad-note-8";
    case MidiActionType::Note16: return "pad-note-16";
    case MidiActionType::Note32: return "pad-note-32";
    case MidiActionType::Note64: return "pad-note-64";
    case MidiActionType::Rest: return "pad-rest";
    case MidiActionType::Dot: return "pad-dot";
    case MidiActionType::DotDot: return "pad-dotdot";
    case MidiActionType::Tie: return "tie";
    case MidiActionType::Undo: return "undo";
    }

    return ActionCode();
}

bool MidiDeviceMappingsModel::useRemoteControl() const
{
    return configuration()->useRemoteControl();
}

void MidiDeviceMappingsModel::setUseRemoteControl(bool value)
{
    if (value == useRemoteControl()) {
        return;
    }

    configuration()->setUseRemoteControl(value);
    emit useRemoteControlChanged(value);
}

QItemSelection MidiDeviceMappingsModel::selection() const
{
    return m_selection;
}

void MidiDeviceMappingsModel::setSelection(const QItemSelection& selection)
{
    if (selection == m_selection) {
        return;
    }

    m_selection = selection;
    emit selectionChanged(selection);
}

void MidiDeviceMappingsModel::clearSelectedActions()
{
    NOT_IMPLEMENTED;
}

void MidiDeviceMappingsModel::clearAllActions()
{
    NOT_IMPLEMENTED;
}
