/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "context/iglobalcontext.h"
#include "ui/iuiactionsregister.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"

namespace mu::notation {
class UndoRedoModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariant undoItem READ makeUndoItem NOTIFY stackChanged)
    Q_PROPERTY(QVariant redoItem READ makeRedoItem NOTIFY stackChanged)

    INJECT(context::IGlobalContext, context)
    INJECT(ui::IUiActionsRegister, actionsRegister)

public:
    explicit UndoRedoModel(QObject* parent = nullptr);

    QVariant makeUndoItem();
    QVariant makeRedoItem();

    Q_INVOKABLE void load();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

signals:
    void stackChanged();

private:
    INotationUndoStackPtr undoStack() const;
};
}

#endif // MU_NOTATION_UNDOREDOMODEL_H
