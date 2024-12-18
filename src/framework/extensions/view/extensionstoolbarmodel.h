/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "../iextensionsprovider.h"
#include "ui/iuiactionsregister.h"
#include "uicomponents/view/abstracttoolbarmodel.h"
#include "actions/iactionsdispatcher.h"

namespace muse::extensions {
class ExtensionsToolBarModel : public uicomponents::AbstractToolBarModel
{
    Q_OBJECT

    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged FINAL)

    Inject<IExtensionsProvider> extensionsProvider = { this };
    Inject<ui::IUiActionsRegister> actionsRegister = { this };
    Inject<actions::IActionsDispatcher> dispatcher = { this };

public:

    Q_INVOKABLE void load() override;

    bool isEmpty() const;

signals:
    void isEmptyChanged();

private:

    bool m_isEmpty = true;
};
}
