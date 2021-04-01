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

static const QString TITLE_KEY("title");
static const QString ICON_KEY("icon");
static const QString STATUS_KEY("status");
static const QString ENABLED_KEY("enabled");
static const QString MAPPED_VALUE_KEY("mappedValue");

inline std::vector<std::string> allMidiActions()
{
    return {
        "rewind",
        "loop",
        "play",
        "stop",
        "note-input",
        "pad-note-1",
        "pad-note-2",
        "pad-note-4",
        "pad-note-8",
        "pad-note-16",
        "pad-note-32",
        "pad-note-64",
        "pad-rest",
        "pad-dot",
        "pad-dotdot",
        "tie",
        "undo"
    };
}

MidiDeviceMappingsModel::MidiDeviceMappingsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant MidiDeviceMappingsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QVariantMap obj = actionToObject(m_actions[index.row()]);

    switch (role) {
    case RoleTitle: return obj[TITLE_KEY].toString();
    case RoleIcon: return obj[ICON_KEY].toInt();
    case RoleStatus: return obj[STATUS_KEY].toString();
    case RoleEnabled: return obj[ENABLED_KEY].toBool();
    case RoleMappedValue: return obj[MAPPED_VALUE_KEY].toInt();
    }

    return QVariant();
}

QVariantMap MidiDeviceMappingsModel::actionToObject(const UiAction& action) const
{
    QVariantMap obj;

    obj[TITLE_KEY] = action.title;
    obj[ICON_KEY] = static_cast<int>(action.iconCode);

    // not implemented:
    obj[STATUS_KEY] = "Inactive";
    obj[ENABLED_KEY] = false;
    obj[MAPPED_VALUE_KEY] = 0;

    return obj;
}

int MidiDeviceMappingsModel::rowCount(const QModelIndex&) const
{
    return m_actions.size();
}

QHash<int, QByteArray> MidiDeviceMappingsModel::roleNames() const
{
    return {
        { RoleTitle, TITLE_KEY.toUtf8() },
        { RoleIcon, ICON_KEY.toUtf8() },
        { RoleStatus, STATUS_KEY.toUtf8() },
        { RoleEnabled, ENABLED_KEY.toUtf8() },
        { RoleMappedValue, MAPPED_VALUE_KEY.toUtf8() }
    };
}

void MidiDeviceMappingsModel::load()
{
    beginResetModel();
    m_actions.clear();

    for (const std::string& actionCode : allMidiActions()) {
        UiAction action = uiActionsRegister()->action(actionCode);

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

bool MidiDeviceMappingsModel::canEditAction() const
{
    return currentAction().isValid();
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

QVariant MidiDeviceMappingsModel::currentAction() const
{
    if (m_selection.size() != 1) {
        return QVariant();
    }

    UiAction action = m_actions[m_selection.indexes().first().row()];
    return actionToObject(action);
}

void MidiDeviceMappingsModel::mapCurrentActionToMidiValue(int value)
{
    UNUSED(value);
    NOT_IMPLEMENTED;
}
