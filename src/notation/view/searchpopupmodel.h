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
#ifndef MU_NOTATION_SEARCHPOPUPMODEL_H
#define MU_NOTATION_SEARCHPOPUPMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class SearchPopupModel : public QObject, public muse::actions::Actionable
{
    Q_OBJECT

    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(context::IGlobalContext, globalContext)

public:
    Q_INVOKABLE void load();
    Q_INVOKABLE void search(const QString& text);

signals:
    void showPopupRequested();

private:
    INotationPtr notation() const;
};
}

#endif // MU_NOTATION_SEARCHPOPUPMODEL_H
