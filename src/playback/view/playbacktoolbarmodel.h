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
#ifndef MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H
#define MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "iplaybackcontroller.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "ui/view/abstractmenumodel.h"
#include "actions/actiontypes.h"
#include "workspace/iworkspacemanager.h"
#include "iinteractive.h"

namespace mu::playback {
class PlaybackToolBarModel : public QAbstractListModel, public ui::AbstractMenuModel
{
    Q_OBJECT

    INJECT(playback, actions::IActionsDispatcher, dispatcher)
    INJECT(playback, ui::IUiActionsRegister, uiactionsRegister)
    INJECT(playback, IPlaybackController, playbackController)
    INJECT(playback, workspace::IWorkspaceManager, workspaceManager)

    Q_PROPERTY(bool isToolbarFloating READ isToolbarFloating WRITE setIsToolbarFloating NOTIFY isToolbarFloatingChanged)
    Q_PROPERTY(bool isPlayAllowed READ isPlayAllowed NOTIFY isPlayAllowedChanged)

    Q_PROPERTY(QDateTime maxPlayTime READ maxPlayTime NOTIFY maxPlayTimeChanged)
    Q_PROPERTY(QDateTime playTime READ playTime WRITE setPlayTime NOTIFY playTimeChanged)
    Q_PROPERTY(qreal playPosition READ playPosition WRITE setPlayPosition NOTIFY playTimeChanged)

    Q_PROPERTY(int measureNumber READ measureNumber WRITE setMeasureNumber NOTIFY playTimeChanged)
    Q_PROPERTY(int maxMeasureNumber READ maxMeasureNumber NOTIFY playTimeChanged)
    Q_PROPERTY(int beatNumber READ beatNumber WRITE setBeatNumber NOTIFY playTimeChanged)
    Q_PROPERTY(int maxBeatNumber READ maxBeatNumber NOTIFY playTimeChanged)

    Q_PROPERTY(QVariant tempo READ tempo NOTIFY playTimeChanged)

public:
    explicit PlaybackToolBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isToolbarFloating() const;
    bool isPlayAllowed() const;

    QDateTime maxPlayTime() const;
    QDateTime playTime() const;
    qreal playPosition() const;

    int measureNumber() const;
    int maxMeasureNumber() const;
    int beatNumber() const;
    int maxBeatNumber() const;

    QVariant tempo() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCode);

public slots:
    void setIsToolbarFloating(bool floating);
    void setPlayPosition(qreal position);
    void setPlayTime(const QDateTime& time);
    void setMeasureNumber(int measureNumber);
    void setBeatNumber(int beatNumber);

signals:
    void isToolbarFloatingChanged(bool floating);
    void isPlayAllowedChanged();
    void maxPlayTimeChanged();
    void playTimeChanged();

private:
    enum Roles {
        CodeRole = Qt::UserRole + 1,
        HintRole,
        IconRole,
        CheckedRole,
        SubitemsRole,
    };

    void setupConnections();

    void updateActions();
    void onActionsStateChanges(const actions::ActionCodeList& codes) override;

    actions::ActionCodeList currentWorkspaceActionCodes() const;

    bool isAdditionalAction(const actions::ActionCode& actionCode) const;

    ui::MenuItem makeActionWithDescriptionAsTitle(const actions::ActionCode& actionCode) const;

    QTime totalPlayTime() const;
    uint64_t totalPlayTimeMilliseconds() const;
    notation::MeasureBeat measureBeat() const;

    void updatePlayTime();
    void doSetPlayTime(const QTime& time);

    void rewind(uint64_t milliseconds);
    void rewindToBeat(const notation::MeasureBeat& beat);

    bool m_isToolbarFloating = false;
    QTime m_playTime;
};
}

#endif // MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H
