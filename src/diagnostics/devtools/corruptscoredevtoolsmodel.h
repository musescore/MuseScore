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

#ifndef MU_DIAGNOSTICS_CORRUPTSCORENDEVTOOLSMODEL_H
#define MU_DIAGNOSTICS_CORRUPTSCORENDEVTOOLSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::diagnostics {
class CorruptScoreDevToolsModel : public QObject
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

public:
    explicit CorruptScoreDevToolsModel(QObject* parent = nullptr);

    Q_INVOKABLE void corruptOpenScore();
};
}

#endif // MU_DIAGNOSTICS_CORRUPTSCORENDEVTOOLSMODEL_H
