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
#include "inspectorformwidget.h"

InspectorFormWidget::InspectorFormWidget(QQmlEngine* qmlEngine, QWidget* parent)
    : QQuickWidget(qmlEngine, parent)
{
    setMinimumWidth(360);
    setMinimumHeight(parent->height());

    if (parent) {
        m_inspector = qobject_cast<InspectorDockWidget*>(parent);
    }

    QUrl url = QUrl(QStringLiteral("qrc:/qml/MuseScore/Inspector/InspectorForm.qml"));

    setSource(url);

    setResizeMode(QQuickWidget::SizeRootObjectToView);

    m_inspectorListModel = new InspectorListModel(this);
    connect(m_inspectorListModel, &InspectorListModel::elementsModified, this, &InspectorFormWidget::layoutUpdateRequested);

    setFocusPolicy(Qt::StrongFocus);

    if (rootObject()) {
        rootObject()->setProperty("model", QVariant::fromValue(m_inspectorListModel));
        rootObject()->setWidth(minimumWidth());
        rootObject()->setHeight(minimumHeight());
    }
}

void InspectorFormWidget::onSelectionChanged(const QList<Ms::EngravingItem*>& elementList)
{
    m_inspectorListModel->setElementList(elementList);
}

void InspectorFormWidget::focusInEvent(QFocusEvent* event)
{
    QQuickWidget::focusInEvent(event);

    if (rootObject()) {
        rootObject()->setFocus(true);
    }
}
