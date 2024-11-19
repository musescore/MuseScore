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
#include <QQmlParserStatus>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

namespace mu::notation {
class UndoHistoryModel : public QAbstractListModel, public QQmlParserStatus, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)

    muse::Inject<context::IGlobalContext> context = { this };

public:
    explicit UndoHistoryModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const;

    Q_INVOKABLE void undoRedoToIndex(int index);

signals:
    void currentIndexChanged();

private:
    void classBegin() override;
    void componentComplete() override {}

    void onCurrentNotationChanged();
    void onUndoRedo();
    void updateCurrentIndex();

    INotationUndoStackPtr undoStack() const;

    int m_rowCount = 0;
};
}
