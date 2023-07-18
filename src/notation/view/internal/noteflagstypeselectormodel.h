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
#ifndef MU_NOTATION_NOTEFLAGsTYPESELECTORMODEL_H
#define MU_NOTATION_NOTEFLAGsTYPESELECTORMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class NoteFlagsTypeSelectorModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool useStraightNoteFlags READ useStraightNoteFlags WRITE setUseStraightNoteFlags NOTIFY useStraightNoteFlagsChanged)

    INJECT(context::IGlobalContext, context)

public:
    explicit NoteFlagsTypeSelectorModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    bool useStraightNoteFlags() const;
    void setUseStraightNoteFlags(bool use);

signals:
    void useStraightNoteFlagsChanged();
};
}

#endif // MU_NOTATION_NOTEFLAGSTYPESELECTORMODEL_H
