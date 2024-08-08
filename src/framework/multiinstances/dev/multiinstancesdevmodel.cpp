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
#include "multiinstancesdevmodel.h"

using namespace muse::mi;

MultiInstancesDevModel::MultiInstancesDevModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void MultiInstancesDevModel::init()
{
    multiInstancesProvider()->instancesChanged().onNotify(this, [this]() {
        update();
    });

    update();
}

void MultiInstancesDevModel::update()
{
    m_instances.clear();
    for (const InstanceMeta& m : multiInstancesProvider()->instances()) {
        QVariantMap item;
        item["id"] = QString::fromStdString(m.id);
        item["isServer"] = m.isServer;
        m_instances.append(item);
    }

    emit instancesChanged();
}

void MultiInstancesDevModel::ping()
{
}

const QVariantList& MultiInstancesDevModel::instances() const
{
    return m_instances;
}

QString MultiInstancesDevModel::selfID() const
{
    return QString::fromStdString(multiInstancesProvider()->selfID());
}
