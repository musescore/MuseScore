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

#include "ui/view/musicalsymbolcodes.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::uicomponents;
using namespace mu::workspace;
using namespace mu::shortcuts;
using namespace mu::ui;
using namespace mu::notation;

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
    case IsAdditionalRole: return isAdditionalAction(item.code);
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
        { CheckedRole, "checked" },
        { IsAdditionalRole, "isAdditional" }
    };

    return roles;
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

    playbackController()->actionEnabledChanged().onReceive(this, [this](const ActionCode&) {
        updateState();
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
    for (const ActionCode& actionCode : toolbar->actions) {
        actions.push_back(actionsRegister()->action(actionCode));
    }

    return actions;
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

QTime PlaybackToolBarModel::playTime() const
{
    NOT_IMPLEMENTED;
    return QTime::currentTime();
}

void PlaybackToolBarModel::setPlayTime(const QTime& time)
{
    Q_UNUSED(time)
    NOT_IMPLEMENTED;
    emit playTimeChanged(time);
}

int PlaybackToolBarModel::measureNumber() const
{
    NOT_IMPLEMENTED;
    return 2;
}

void PlaybackToolBarModel::setMeasureNumber(int measureNumber)
{
    Q_UNUSED(measureNumber)
    NOT_IMPLEMENTED;
    emit measureNumberChanged(measureNumber);
}

int PlaybackToolBarModel::maxMeasureNumber() const
{
    NOT_IMPLEMENTED;
    return 8;
}

int PlaybackToolBarModel::beatNumber() const
{
    NOT_IMPLEMENTED;
    return 3;
}

void PlaybackToolBarModel::setBeatNumber(int beatNumber)
{
    Q_UNUSED(beatNumber)
    NOT_IMPLEMENTED;
    emit beatNumberChanged(beatNumber);
}

int PlaybackToolBarModel::maxBeatNumber() const
{
    NOT_IMPLEMENTED;
    return 9;
}

QVariant PlaybackToolBarModel::tempo() const
{
    NOT_IMPLEMENTED;

    QVariantMap tempo;
    tempo["noteSymbol"] = noteIconToString(MusicalSymbolCodes::Code::CROTCHET, true);
    tempo["value"] = 180;

    return tempo;
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
    bool isPlayAllowed = playbackController()->isPlayAllowed();

    for (MenuItem& item : m_items) {
        item.enabled = isPlayAllowed;
        item.checked = playbackController()->isActionEnabled(item.code);
    }

    if (isPlayAllowed) {
        bool isPlaying = playbackController()->isPlaying();
        item("play").iconCode = isPlaying ? IconCode::Code::PAUSE : IconCode::Code::PLAY;
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

MenuItem& PlaybackToolBarModel::item(const ActionCode& actionCode)
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

bool PlaybackToolBarModel::isAdditionalAction(const ActionCode& actionCode) const
{
    return actionCode == "loop-in" || actionCode == "loop-out";
}
