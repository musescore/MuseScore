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
#ifndef MU_EXTENSIONS_DEVEXTENSIONSLISTMODEL_H
#define MU_EXTENSIONS_DEVEXTENSIONSLISTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "../iextensionsprovider.h"
#include "global/iinteractive.h"

namespace mu::extensions {
class DevExtensionsListModel : public QObject
{
    Q_OBJECT

    Inject<IExtensionsProvider> provider;
    Inject<IInteractive> interactive;

public:
    DevExtensionsListModel(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList extensionsList();
    Q_INVOKABLE void clicked(const QString& uri);
};
}

#endif // MU_EXTENSIONS_DEVEXTENSIONSLISTMODEL_H
