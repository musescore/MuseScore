/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#ifndef MU_NOTATION_ORGANPEDALMARKPOPUPMODEL_H
#define MU_NOTATION_ORGANPEDALMARKPOPUPMODEL_H

#include <QObject>

#include "context/iglobalcontext.h"

#include "engraving/dom/organpedalmark.h"

#include "view/abstractelementpopupmodel.h"

namespace mu::notation {
class OrganPedalMarkPopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

public:
    explicit OrganPedalMarkPopupModel(QObject* parent = nullptr);

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void updatePedalMark(QString PedalMarkName);
    Q_INVOKABLE bool isAbove();
};
} //namespace mu::notation

#endif // MU_NOTATION_ORGANPEDALMARKPOPUPMODEL_H
