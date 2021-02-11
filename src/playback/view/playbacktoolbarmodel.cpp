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
#include "playback/playbacktypes.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::uicomponents;
using namespace mu::workspace;
using namespace mu::shortcuts;
using namespace mu::ui;
using namespace mu::notation;

static const std::string PLAYBACK_TOOLBAR_KEY("playbackControl");
static const std::string PLAYBACK_SETTINGS_KEY("playback-settings");

static MusicalSymbolCodes::Code tempoDurationToNoteIcon(DurationType durationType)
{
    QMap<DurationType, MusicalSymbolCodes::Code> durationToNoteIcon {
        { DurationType::V_WHOLE, MusicalSymbolCodes::Code::SEMIBREVE },
        { DurationType::V_HALF, MusicalSymbolCodes::Code::MINIM },
        { DurationType::V_QUARTER, MusicalSymbolCodes::Code::CROTCHET },
        { DurationType::V_EIGHTH, MusicalSymbolCodes::Code::SEMIQUAVER },
        { DurationType::V_16TH, MusicalSymbolCodes::Code::DEMISEMIQUAVER }
    };

    return durationToNoteIcon[durationType];
}

PlaybackToolBarModel::PlaybackToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant PlaybackToolBarModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

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
        updatePlayTime();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
        updatePlayTime();
    });

    playbackController()->midiTickPlayed().onReceive(this, [this](uint64_t) {
        updatePlayTime();
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

QDateTime PlaybackToolBarModel::playTime() const
{
    return QDateTime(QDate::currentDate(), m_playTime);
}

void PlaybackToolBarModel::setPlayTime(const QDateTime& time)
{
    QTime newTime = time.time();
    if (m_playTime == newTime) {
        return;
    }

    doSetPlayTime(newTime);

    uint64_t msec = timeToMilliseconds(newTime);
    rewind(msec);
}

qreal PlaybackToolBarModel::playPosition() const
{
    qreal allMsecs = totalPlayTimeMilliseconds();
    qreal msecsDifference = allMsecs - m_playTime.msecsTo(totalPlayTime());

    qreal position = msecsDifference / allMsecs;
    return position;
}

void PlaybackToolBarModel::setPlayPosition(qreal position)
{
    uint64_t allMsecs = totalPlayTimeMilliseconds();
    uint64_t playPositionMsecs = allMsecs * position;

    QTime time = timeFromMillisecons(playPositionMsecs);
    setPlayTime(QDateTime(QDate::currentDate(), time));
}

QTime PlaybackToolBarModel::totalPlayTime() const
{
    return playbackController()->totalPlayTime();
}

uint64_t PlaybackToolBarModel::totalPlayTimeMilliseconds() const
{
    return timeToMilliseconds(totalPlayTime());
}

MeasureBeat PlaybackToolBarModel::measureBeat() const
{
    return playbackController()->currentMeasureBeat();
}

void PlaybackToolBarModel::updatePlayTime()
{
    float seconds = playbackController()->playbackPositionInSeconds();
    QTime playTime = timeFromSeconds(seconds);

    if (m_playTime == playTime) {
        return;
    }

    doSetPlayTime(playTime);
}

void PlaybackToolBarModel::doSetPlayTime(const QTime& time)
{
    m_playTime = time;

    emit playTimeChanged(time);
    emit playPositionChanged(playPosition());
    emit measureNumberChanged(measureNumber());
    emit maxMeasureNumberChanged(maxMeasureNumber());
    emit beatNumberChanged(beatNumber());
    emit maxBeatNumberChanged(maxBeatNumber());
    emit tempoChanged(tempo());
}

void PlaybackToolBarModel::rewind(uint64_t milliseconds)
{
    dispatcher()->dispatch("rewind", ActionData::make_arg1<uint64_t>(milliseconds));
}

void PlaybackToolBarModel::rewindToMeasureBeat(const notation::MeasureBeat& barBeat)
{
    uint64_t milliseconds = playbackController()->measureBeatToMilliseconds(barBeat);
    rewind(milliseconds);
}

int PlaybackToolBarModel::measureNumber() const
{
    return measureBeat().measureIndex + 1;
}

void PlaybackToolBarModel::setMeasureNumber(int measureNumber)
{
    int measureIndex = measureNumber - 1;
    MeasureBeat measureBeat = this->measureBeat();

    if (measureIndex == measureBeat.measureIndex) {
        return;
    }

    measureBeat.measureIndex = measureIndex;
    rewindToMeasureBeat(measureBeat);

    emit measureNumberChanged(measureNumber);
}

int PlaybackToolBarModel::maxMeasureNumber() const
{
    return measureBeat().maxMeasureIndex + 1;
}

int PlaybackToolBarModel::beatNumber() const
{
    return measureBeat().beatIndex + 1;
}

void PlaybackToolBarModel::setBeatNumber(int beatNumber)
{
    int beatIndex = beatNumber - 1;
    MeasureBeat measureBeat = this->measureBeat();

    if (beatIndex == measureBeat.beatIndex) {
        return;
    }

    measureBeat.beatIndex = beatIndex;
    rewindToMeasureBeat(measureBeat);

    emit beatNumberChanged(beatNumber);
}

int PlaybackToolBarModel::maxBeatNumber() const
{
    return measureBeat().maxBeatIndex + 1;
}

QVariant PlaybackToolBarModel::tempo() const
{
    Tempo tempo = playbackController()->currentTempo();
    MusicalSymbolCodes::Code noteIcon = tempoDurationToNoteIcon(tempo.duration);

    QVariantMap obj;
    obj["noteSymbol"] = noteIconToString(noteIcon, tempo.withDot);
    obj["value"] = tempo.value;

    return obj;
}

void PlaybackToolBarModel::handleAction(const QString& action)
{
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
