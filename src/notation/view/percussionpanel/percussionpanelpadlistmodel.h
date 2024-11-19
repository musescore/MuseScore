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

static constexpr int NUM_COLUMNS(8);

class PercussionPanelPadListModel : public QAbstractListModel, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int numColumns READ numColumns CONSTANT)
    Q_PROPERTY(int numPads READ numPads NOTIFY numPadsChanged)

public:
    explicit PercussionPanelPadListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex&) const override { return m_padModels.count(); }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();

    Q_INVOKABLE void addRow();
    Q_INVOKABLE void deleteRow(int row);
    Q_INVOKABLE bool rowIsEmpty(int row) const;

    Q_INVOKABLE void startPadSwap(int startIndex);
    Q_INVOKABLE void endPadSwap(int endIndex);

    int numColumns() const { return NUM_COLUMNS; }
    int numPads() const { return m_padModels.count(); }

    void setDrumset(const mu::engraving::Drumset* drumset);
    const mu::engraving::Drumset* drumset() const { return m_drumset; }

    void resetLayout();

    muse::async::Channel<int /*pitch*/> padTriggered() const { return m_triggeredChannel; }

signals:
    void numPadsChanged();
    void rowIsEmptyChanged(int row, bool empty);

private:
    enum Roles {
        PadModelRole = Qt::UserRole + 1,
    };

    bool indexIsValid(int index) const;
    void movePad(int fromIndex, int toIndex);

    int numEmptySlotsAtRow(int row) const;

    const mu::engraving::Drumset* m_drumset = nullptr;
    QList<PercussionPanelPadModel*> m_padModels;

    int m_padSwapStartIndex = -1;

    muse::async::Channel<int /*pitch*/> m_triggeredChannel;
};
