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
#include "playbacktoolbarmodel.h"

#include "log.h"
#include "translation.h"

#include "shortcuts/shortcutstypes.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::uicomponents;
using namespace mu::workspace;
using namespace mu::shortcuts;
using namespace mu::ui;

static const std::string PLAYBACK_TOOLBAR_KEY("playbackControl");
static const std::string PLAYBACK_SETTINGS_KEY("playback-settings");

PlaybackToolBarModel::PlaybackToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant PlaybackToolBarModel::data(const QModelIndex& index, int role) const
{
    const MenuItem& item = m_items.at(index.row());
    switch (role) {
    case HintRole: return QString::fromStdString(item.description);
    case IconRole: return static_cast<int>(item.iconCode);
    case CodeRole: return QString::fromStdString(item.code);
    case EnabledRole: return item.enabled;
    case CheckedRole: return item.checked;
    }
    return QVariant();
}

int PlaybackToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> PlaybackToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { CodeRole, "code" },
        { HintRole, "hint" },
        { IconRole, "icon" },
        { EnabledRole, "enabled" },
        { CheckedRole, "checked" }
    };

    return roles;
}

qreal PlaybackToolBarModel::playPosition() const
{
    NOT_IMPLEMENTED;
    return 0.75;
}

void PlaybackToolBarModel::setPlayPosition(qreal position)
{
    Q_UNUSED(position)
    NOT_IMPLEMENTED;
    emit playPositionChanged(position);
}

void PlaybackToolBarModel::load()
{
    beginResetModel();
    m_items.clear();

    for (const ActionItem& action : currentWorkspaceActions()) {
        m_items << action;
    }

    m_items << settingsItem();

    endResetModel();

    updateState();

    playbackController()->isPlayAllowedChanged().onNotify(this, [this]() {
        updateState();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });

    workspaceManager()->currentWorkspace().ch.onReceive(this, [this](IWorkspacePtr) {
        load();
    });
}

ActionList PlaybackToolBarModel::currentWorkspaceActions() const
{
    RetValCh<IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();
    if (!workspace.ret || !workspace.val) {
        LOGE() << workspace.ret.toString();
        return ActionList();
    }

    AbstractDataPtr abstractData = workspace.val->data(WorkspaceTag::Toolbar, PLAYBACK_TOOLBAR_KEY);
    ToolbarDataPtr toolbar = std::dynamic_pointer_cast<ToolbarData>(abstractData);
    if (!toolbar) {
        return ActionList();
    }

    ActionList actions;
    for (const std::string& actionCode : toolbar->actions) {
        actions.push_back(actionsRegister()->action(actionCode));
    }

    return actions;
}

void PlaybackToolBarModel::handleAction(const QString& action)
{
    //! NOTE: Temporary solution
    if (action.toStdString() == PLAYBACK_SETTINGS_KEY) {
        interactive()->open("musescore://playback/settings");
        return;
    }

    dispatcher()->dispatch(actions::codeFromQString(action));
}

void PlaybackToolBarModel::updateState()
{
    if (playbackController()->isPlayAllowed()) {
        for (MenuItem& item : m_items) {
            item.enabled = true;
            item.checked = false;
        }

        bool isPlaying = playbackController()->isPlaying();
        item("play").iconCode = isPlaying ? IconCode::Code::STOP : IconCode::Code::PLAY;
    } else {
        for (MenuItem& item : m_items) {
            item.enabled = false;
            item.checked = false;
        }
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

MenuItem& PlaybackToolBarModel::item(const actions::ActionCode& actionCode)
{
    for (MenuItem& item : m_items) {
        if (item.code == actionCode) {
            return item;
        }
    }

    LOGE() << "item not found with name: " << actionCode;
    static MenuItem null;
    return null;
}

MenuItem PlaybackToolBarModel::settingsItem() const
{
    return ActionItem(PLAYBACK_SETTINGS_KEY,
                      ShortcutContext::Any,
                      QT_TRANSLATE_NOOP("action", "Playback settings"),
                      QT_TRANSLATE_NOOP("action", "Open playback settings"),
                      IconCode::Code::SETTINGS_COG
                      );
}
