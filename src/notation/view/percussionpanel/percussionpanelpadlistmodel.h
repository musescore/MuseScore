/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QAbstractListModel>

#include "async/asyncable.h"
#include "async/channel.h"

#include "engraving/dom/drumset.h"

#include "percussionpanelpadmodel.h"

namespace mu::notation {
class PercussionPanelPadListModel : public QAbstractListModel, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int numColumns READ numColumns CONSTANT)
    Q_PROPERTY(int numPads READ numPads NOTIFY numPadsChanged)

public:
    explicit PercussionPanelPadListModel(QObject* parent = nullptr);
    ~PercussionPanelPadListModel();

    int rowCount(const QModelIndex&) const override { return m_padModels.count(); }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();

    Q_INVOKABLE void addEmptyRow();
    Q_INVOKABLE void deleteRow(int row);

    void removeEmptyRows();

    Q_INVOKABLE bool rowIsEmpty(int row) const;

    Q_INVOKABLE void startDrag(int startIndex);
    Q_INVOKABLE void endDrag(int endIndex);

    bool hasActivePads() const { return m_drumset; }

    int numColumns() const { return NUM_COLUMNS; }
    int numPads() const { return m_padModels.count(); }

    void setDrumset(const engraving::Drumset* drumset);
    engraving::Drumset* drumset() const { return m_drumset; }

    QList<PercussionPanelPadModel*> padList() const { return m_padModels; }

    muse::async::Notification hasActivePadsChanged() const { return m_hasActivePadsChanged; }
    muse::async::Channel<int /*pitch*/> padTriggered() const { return m_triggeredChannel; }

signals:
    void numPadsChanged();
    void rowIsEmptyChanged(int row, bool empty);

private:
    static constexpr int NUM_COLUMNS = 8;

    enum Roles {
        PadModelRole = Qt::UserRole + 1,
    };

    void load();

    bool indexIsValid(int index) const;

    PercussionPanelPadModel* createPadModelForPitch(int pitch);
    int createModelIndexForPitch(int pitch) const;

    void movePad(int fromIndex, int toIndex);

    int numEmptySlotsAtRow(int row) const;

    engraving::Drumset* m_drumset = nullptr; //! NOTE: Pointer may be invalid, see PercussionPanelModel::setUpConnections
    QList<PercussionPanelPadModel*> m_padModels;

    int m_dragStartIndex = -1;

    muse::async::Notification m_hasActivePadsChanged;
    muse::async::Channel<int /*pitch*/> m_triggeredChannel;
};
}
