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
#ifndef MU_MI_MULTIINSTANCESDEVMODEL_H
#define MU_MI_MULTIINSTANCESDEVMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "imultiinstancesprovider.h"

#include "async/asyncable.h"

namespace mu::mi {
class MultiInstancesDevModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString selfID READ selfID CONSTANT)
    Q_PROPERTY(QVariantList instances READ instances NOTIFY instancesChanged)

    INJECT(IMultiInstancesProvider, multiInstancesProvider)

public:
    explicit MultiInstancesDevModel(QObject* parent = nullptr);

    const QVariantList& instances() const;
    QString selfID() const;

    Q_INVOKABLE void init();
    Q_INVOKABLE void update();
    Q_INVOKABLE void ping();

signals:
    void instancesChanged();

private:

    QVariantList m_instances;
};
}

#endif // MU_MI_MULTIINSTANCESDEVMODEL_H
