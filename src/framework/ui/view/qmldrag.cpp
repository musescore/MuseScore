/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "qmldrag.h"

#include "log.h"

using namespace muse::ui;

QmlDrag::QmlDrag(QObject* parent)
    : QObject(parent)
{
    m_data = std::make_shared<DragData>();
}

QmlDrag* QmlDrag::qmlAttachedProperties(QObject* object)
{
    return new QmlDrag(object);
}

QQuickItem* QmlDrag::target() const
{
    return qobject_cast<QQuickItem*>(parent());
}

void QmlDrag::onActiveDrag()
{
    controller()->setCurrentData(m_data);
}

void QmlDrag::onDeactivateDrag()
{
}

bool QmlDrag::active() const
{
    return m_active;
}

void QmlDrag::setActive(bool newActive)
{
    if (m_active == newActive) {
        return;
    }
    m_active = newActive;

    if (m_active) {
        onActiveDrag();
    } else {
        onDeactivateDrag();
    }

    emit activeChanged();
}

QVariantMap QmlDrag::mimeData() const
{
    QVariantMap map;
    for (const QString& f : m_data->mimeData.formats()) {
        map[f] = m_data->mimeData.data(f);
    }
    return map;
}

void QmlDrag::setMimeData(const QVariantMap& map)
{
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        m_data->mimeData.setData(it.key(), it.value().toByteArray());
    }
    emit mimeDataChanged();
}
