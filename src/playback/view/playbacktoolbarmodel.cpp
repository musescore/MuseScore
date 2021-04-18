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
#include "playback/internal/playbackuiactions.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::workspace;
using namespace mu::ui;
using namespace mu::notation;

static const std::string PLAYBACK_TOOLBAR_KEY("playbackControl");
static const ActionCode PLAY_ACTION_CODE("play");

static MusicalSymbolCodes::Code tempoDurationToNoteIcon(DurationType durationType)
{
    QMap<DurationType, MusicalSymbolCodes::Code> durationToNoteIcon {
        { DurationType::V_WHOLE, MusicalSymbolCodes::Code::SEMIBREVE },
        { DurationType::V_HALF, MusicalSymbolCodes::Code::MINIM },
        { DurationType::V_QUARTER, MusicalSymbolCodes::Code::CROTCHET },
        { DurationType::V_EIGHTH, MusicalSymbolCodes::Code::QUAVER },
        { DurationType::V_16TH, MusicalSymbolCodes::Code::SEMIQUAVER }
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

    const QVariantMap& item = items().at(index.row()).toMap();

    switch (role) {
    case CodeRole: return item["code"];
    case HintRole: return item["description"];
    case IconRole: return item["icon"];
    case CheckedRole: return item["checked"];
    case SubitemsRole: return item["subitems"];
    }

    return QVariant();
}

int PlaybackToolBarModel::rowCount(const QModelIndex&) const
{
    return items().count();
}

QHash<int, QByteArray> PlaybackToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { CodeRole, "code" },
        { HintRole, "hint" },
        { IconRole, "icon" },
        { CheckedRole, "checked" },
        { SubitemsRole, "subitems" },
    };

    return roles;
}

void PlaybackToolBarModel::load()
{
    updateActions();
    listenActionsStateChanges();
    setupConnections();
}

void PlaybackToolBarModel::setupConnections()
{
    connect(this, &PlaybackToolBarModel::isToolbarFloatingChanged, this, &PlaybackToolBarModel::updateActions);

    playbackController()->isPlayAllowedChanged().onNotify(this, [this]() {
        emit maxPlayTimeChanged();
        updatePlayTime();
        emit isPlayAllowedChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onActionsStateChanges({ PLAY_ACTION_CODE });
    });

    playbackController()->playbackPositionChanged().onNotify(this, [this]() {
        updatePlayTime();
    });

    workspaceManager()->currentWorkspace().ch.onReceive(this, [this](IWorkspacePtr) {
        updateActions();
    });
}

void PlaybackToolBarModel::updateActions()
{
    beginResetModel();
    clear();

    MenuItemList settingsItems;
    MenuItemList additionalItems;

    for (const ActionCode& code : currentWorkspaceActionCodes()) {
        if (isAdditionalAction(code)) {
            //! NOTE: In this case, we want to see the actions' description instead of the title
            additionalItems << makeActionWithDescriptionAsTitle(code);
        } else {
            appendItem(makeAction(code));
        }
    }

    for (const UiAction& action : PlaybackUiActions::settingsActions()) {
        settingsItems << makeActionWithDescriptionAsTitle(action.code);
    }

    if (!m_isToolbarFloating) {
        settingsItems << makeSeparator();
        settingsItems << additionalItems;
    }

    MenuItem settingsItem = makeMenu(qtrc("action", "Playback settings"), settingsItems);
    settingsItem.iconCode = IconCode::Code::SETTINGS_COG;
    appendItem(settingsItem);

    if (m_isToolbarFloating) {
        appendItems(additionalItems);
    }

    endResetModel();
}

void PlaybackToolBarModel::onActionsStateChanges(const actions::ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);

    if (isPlayAllowed() && containsAction(codes, PLAY_ACTION_CODE)) {
        bool isPlaying = playbackController()->isPlaying();
        findItem(PLAY_ACTION_CODE).iconCode = isPlaying ? IconCode::Code::PAUSE : IconCode::Code::PLAY;
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

ActionCodeList PlaybackToolBarModel::currentWorkspaceActionCodes() const
{
    RetValCh<IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();
    if (!workspace.ret || !workspace.val) {
        LOGE() << workspace.ret.toString();
        return {};
    }

    AbstractDataPtr abstractData = workspace.val->data(WorkspaceTag::Toolbar, PLAYBACK_TOOLBAR_KEY);
    ToolbarDataPtr toolbar = std::dynamic_pointer_cast<ToolbarData>(abstractData);
    if (!toolbar) {
        return {};
    }

    return toolbar->actions;
}

bool PlaybackToolBarModel::isAdditionalAction(const actions::ActionCode& actionCode) const
{
    return PlaybackUiActions::loopBoundaryActions().contains(actionCode);
}

MenuItem PlaybackToolBarModel::makeActionWithDescriptionAsTitle(const actions::ActionCode& actionCode) const
{
    MenuItem item = makeAction(actionCode);
    item.title = item.description;
    return item;
}

void PlaybackToolBarModel::handleAction(const QString& actionCode)
{
    dispatcher()->dispatch(actions::codeFromQString(actionCode));
}

bool PlaybackToolBarModel::isPlayAllowed() const
{
    return playbackController()->isPlayAllowed();
}

bool PlaybackToolBarModel::isToolbarFloating() const
{
    return m_isToolbarFloating;
}

void PlaybackToolBarModel::setIsToolbarFloating(bool floating)
{
    if (floating == m_isToolbarFloating) {
        return;
    }

    m_isToolbarFloating = floating;
    emit isToolbarFloatingChanged(floating);
}

QDateTime PlaybackToolBarModel::maxPlayTime() const
{
    return QDateTime(QDate::currentDate(), totalPlayTime());
}

QDateTime PlaybackToolBarModel::playTime() const
{
    return QDateTime(QDate::currentDate(), m_playTime);
}

void PlaybackToolBarModel::setPlayTime(const QDateTime& time)
{
    QTime newTime = time.time();
    if (m_playTime == newTime || time > maxPlayTime()) {
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

    QTime time = timeFromMilliseconds(playPositionMsecs);
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
    return playbackController()->currentBeat();
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
    emit playTimeChanged();
}

void PlaybackToolBarModel::rewind(uint64_t milliseconds)
{
    dispatcher()->dispatch("rewind", ActionData::make_arg1<uint64_t>(milliseconds));
}

void PlaybackToolBarModel::rewindToBeat(const MeasureBeat& beat)
{
    uint64_t msec = playbackController()->beatToMilliseconds(beat.measureIndex, beat.beatIndex);
    rewind(msec);
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
    rewindToBeat(measureBeat);
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
    rewindToBeat(measureBeat);
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
    obj["value"] = tempo.valueBpm;

    return obj;
}
