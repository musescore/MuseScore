/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_NOTATION_UNDOREDOMODEL_H
#define MU_NOTATION_UNDOREDOMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "modularity/ioc.h"
#include "ui/iuiactionsregister.h"

namespace muse::uicomponents {
class MenuItem;
}

namespace mu::notation {
class UndoRedoModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(muse::uicomponents::MenuItem * undoItem READ undoItem NOTIFY itemsChanged)
    Q_PROPERTY(muse::uicomponents::MenuItem * redoItem READ redoItem NOTIFY itemsChanged)

    muse::Inject<context::IGlobalContext> context = { this };
    muse::Inject<muse::ui::IUiActionsRegister> actionsRegister = { this };

public:
    explicit UndoRedoModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    muse::uicomponents::MenuItem* undoItem() const;
    muse::uicomponents::MenuItem* redoItem() const;

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    Q_INVOKABLE size_t undoRedoActionCount() const;
    Q_INVOKABLE size_t undoRedoActionCurrentIdx() const;
    Q_INVOKABLE const QString undoRedoActionNameAtIdx(size_t idx) const;

signals:
    void itemsChanged();

private:
    INotationUndoStackPtr undoStack() const;

    void updateItems();

    muse::uicomponents::MenuItem* m_undoItem = nullptr;
    muse::uicomponents::MenuItem* m_redoItem = nullptr;
};
}

#endif // MU_NOTATION_UNDOREDOMODEL_H
