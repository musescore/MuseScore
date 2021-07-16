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
#ifndef MU_INSPECTOR_NOTATIONSETTINGSPROXYMODEL_H
#define MU_INSPECTOR_NOTATIONSETTINGSPROXYMODEL_H

#include "models/abstractinspectorproxymodel.h"
#include "models/iinspectormodelcreator.h"

namespace mu::inspector {
class NotationSettingsProxyModel : public AbstractInspectorProxyModel
{
    Q_OBJECT

    INJECT(inspector, IInspectorModelCreator, inspectorModelCreator)

public:
    explicit NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository, const QSet<Ms::ElementType>& elementSet);

    bool isTypeSupported(Ms::ElementType elementType) const override;

private:
    QList<AbstractInspectorModel::InspectorModelType> modelTypes(const QSet<Ms::ElementType>& elements) const;
};
}

#endif // MU_INSPECTOR_NOTATIONSETTINGSPROXYMODEL_H
