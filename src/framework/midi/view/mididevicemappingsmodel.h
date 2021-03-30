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

#ifndef MU_MIDI_MIDIDEVICEMAPPINGSMODEL_H
#define MU_MIDI_MIDIDEVICEMAPPINGSMODEL_H

#include <QAbstractListModel>
#include <QItemSelection>

#include "modularity/ioc.h"
#include "imidiconfiguration.h"
#include "midi/miditypes.h"

#include "actions/iactionsregister.h"

namespace mu::midi {
class MidiDeviceMappingsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool useRemoteControl READ useRemoteControl WRITE setUseRemoteControl NOTIFY useRemoteControlChanged)
    Q_PROPERTY(QItemSelection selection READ selection WRITE setSelection NOTIFY selectionChanged)

    INJECT(midi, IMidiConfiguration, configuration)
    INJECT(Midi, actions::IActionsRegister, actionsRegister)

public:
    explicit MidiDeviceMappingsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool useRemoteControl() const;
    QItemSelection selection() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE bool apply();

    Q_INVOKABLE void clearSelectedActions();
    Q_INVOKABLE void clearAllActions();

public slots:
    void setUseRemoteControl(bool value);
    void setSelection(const QItemSelection& selection);

signals:
    void useRemoteControlChanged(bool value);
    void selectionChanged(const QItemSelection& selection);

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleIcon,
        RoleStatus
    };

    actions::ActionCode actionCode(MidiActionType type) const;
    QString formatActionTitle(const actions::ActionItem& action) const;

    actions::ActionList m_actions;
    QItemSelection m_selection;
};
}

#endif // MU_MIDI_MIDIDEVICEMAPPINGSMODEL_H
