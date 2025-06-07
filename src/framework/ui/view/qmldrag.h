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
#pragma once

#include <QObject>
#include <QQmlEngine>

#include "global/modularity/ioc.h"
#include "../idragcontroller.h"

namespace muse::ui {
class QmlDrag : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(QVariantMap mimeData READ mimeData WRITE setMimeData NOTIFY mimeDataChanged FINAL)

    QML_ELEMENT
    QML_ATTACHED(QmlDrag)

    Inject<IDragController> controller;

public:
    QmlDrag(QObject* parent);

    bool active() const;
    void setActive(bool newActive);

    static QmlDrag* qmlAttachedProperties(QObject* object);

    QVariantMap mimeData() const;
    void setMimeData(const QVariantMap& newMimeData);

signals:
    void activeChanged();
    void mimeDataChanged();

private:

    QQuickItem* target() const;

    void onActiveDrag();
    void onDeactivateDrag();

    QQuickItem* m_originTargetParent = nullptr;

    DragDataPtr m_data;
    bool m_active = false;
};
}
