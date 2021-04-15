/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_INSPECTOR_ABSTRACTINSPECTORPROXYMODEL_H
#define MU_INSPECTOR_ABSTRACTINSPECTORPROXYMODEL_H

#include "models/abstractinspectormodel.h"

#include <QHash>

namespace mu::inspector {
class AbstractInspectorProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

public:
    explicit AbstractInspectorProxyModel(QObject* parent);

    Q_INVOKABLE QObject* modelByType(const InspectorModelType type);

    void createProperties() override {}
    void requestElements() override {}
    void loadProperties() override {}
    void resetProperties() override {}

    void requestResetToDefaults() override;
    bool hasAcceptableElements() const override;

protected:
    void addModel(AbstractInspectorModel* model);

private:
    QHash<int, AbstractInspectorModel*> m_modelsHash;
};
}

#endif // MU_INSPECTOR_ABSTRACTINSPECTORPROXYMODEL_H
