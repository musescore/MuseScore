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

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "async/channel.h"

#include "iinteractive.h"
#include "inotationconfiguration.h"

#include "engraving/dom/drumset.h"

#include "percussionpanelpadmodel.h"

namespace mu::notation {
class PercussionPanelPadListModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<INotationConfiguration> configuration = { this };

    Q_OBJECT

    Q_PROPERTY(int numColumns READ numColumns NOTIFY numColumnsChanged)
    Q_PROPERTY(int numPads READ numPads NOTIFY numPadsChanged)

public:
    explicit PercussionPanelPadListModel(QObject* parent = nullptr);
    ~PercussionPanelPadListModel();

    int rowCount(const QModelIndex&) const override { return m_padModels.count(); }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();

    Q_INVOKABLE void addEmptyRow(bool focusFirstInNewRow = false);
    Q_INVOKABLE void deleteRow(int row);

    void removeEmptyRows();

    Q_INVOKABLE void startPadSwap(int startIndex);
    Q_INVOKABLE void endPadSwap(int endIndex);
    bool swapInProgress() const { return indexIsValid(m_padSwapStartIndex); }

    bool hasActivePads() const { return m_drumset; }

    int numColumns() const { return m_drumset ? static_cast<int>(m_drumset->percussionPanelColumns()) : DEFAULT_NUM_COLUMNS; }
    int numPads() const { return m_padModels.count(); }

    void setDrumset(const engraving::Drumset* drumset);
    const engraving::Drumset* drumset() const { return m_drumset; }

    QList<PercussionPanelPadModel*> padList() const { return m_padModels; }

    mu::engraving::Drumset constructDefaultLayout(const engraving::Drumset& templateDrumset) const;
    int nextAvailableIndex(int pitch) const; //! NOTE: This may be equal to m_padModels.size()
    int nextAvailablePitch(int pitch) const;

    void focusFirstActivePad();
    void focusLastActivePad();

    muse::async::Notification hasActivePadsChanged() const { return m_hasActivePadsChanged; }
    muse::async::Channel<PercussionPanelPadModel::PadAction, int /*pitch*/> padActionRequested() const { return m_padActionRequestChannel; }

signals:
    void numPadsChanged();
    void numColumnsChanged();
    void padFocusRequested(int padIndex); //! NOTE: This won't work if it is called immediately before a layoutChange

private:
    static constexpr int DEFAULT_NUM_COLUMNS = 8;

    enum Roles {
        PadModelRole = Qt::UserRole + 1,
    };

    void load();

    bool indexIsValid(int index) const;

    PercussionPanelPadModel* createPadModelForPitch(int pitch);
    int createModelIndexForPitch(int pitch) const;

    int getModelIndexForPitch(int pitch) const;

    void movePad(int fromIndex, int toIndex);

    muse::RetVal<muse::Val> openPadSwapDialog();
    void swapMidiNotesAndShortcuts(int fromIndex, int toIndex);

    int numEmptySlotsAtRow(int row) const;

    engraving::Drumset* m_drumset = nullptr; //! NOTE: Pointer may be invalid, see PercussionPanelModel::setUpConnections
    QList<PercussionPanelPadModel*> m_padModels;

    int m_padSwapStartIndex = -1;

    muse::async::Notification m_hasActivePadsChanged;
    muse::async::Channel<PercussionPanelPadModel::PadAction, int /*pitch*/> m_padActionRequestChannel;
};
}
