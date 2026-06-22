/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include <QQmlParserStatus>
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "modularity/ioc.h"
//#include "actions/iactionsdispatcher.h"

namespace mu::notation {
class UndoHistoryModel : public QAbstractListModel, public QQmlParserStatus, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus);
    QML_ELEMENT;

    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)

    muse::ContextInject<context::IGlobalContext> context = { this };

    Q_PROPERTY(QVariantList snapshots READ snapshots NOTIFY snapshotsChanged)
    QVariantList snapshots() const;

    //muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit UndoHistoryModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const;

    Q_INVOKABLE void undoRedoToIndex(int index);

    Q_INVOKABLE void addSnapshot(const QString& name);
    Q_INVOKABLE void removeSnapshot(int index);
    Q_INVOKABLE void restoreSnapshot(int index);

signals:
    void currentIndexChanged();
    void snapshotsChanged();

private:
    void classBegin() override;
    void componentComplete() override {}
    void init();

    void onCurrentNotationChanged();
    void onUndoRedo();
    void updateCurrentIndex();

    INotationUndoStackPtr undoStack() const;
    int m_rowCount = 0;
};
}
