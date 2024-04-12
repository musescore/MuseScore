/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_INSPECTOR_RESTSETTINGSPROXYMODEL_H
#define MU_INSPECTOR_RESTSETTINGSPROXYMODEL_H

#include "models/abstractinspectorproxymodel.h"

namespace mu::inspector {
class RestSettingsProxyModel : public AbstractInspectorProxyModel
{
    Q_OBJECT

public:
    explicit RestSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);

private slots:
    void onElementsUpdated(const QList<mu::engraving::EngravingItem*>& newElements);

private:
    InspectorModelType resolveDefaultSubModelType(const QList<mu::engraving::EngravingItem*>& newElements) const;
};
}

#endif // MU_INSPECTOR_RESTSETTINGSPROXYMODEL_H
