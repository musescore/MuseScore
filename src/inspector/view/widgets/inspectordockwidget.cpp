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

#include "inspectordockwidget.h"

#include "view/widgets/inspectorformwidget.h"

InspectorDockWidget::InspectorDockWidget(QQmlEngine* engine, QWidget* parent)
    : QDockWidget(parent), m_qmlEngine(engine)
{
    setObjectName("inspector");
    setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));

    m_inspectorForm = new InspectorFormWidget(engine, this);
    setWidget(m_inspectorForm);

    connect(m_inspectorForm, &InspectorFormWidget::layoutUpdateRequested, this, &InspectorDockWidget::layoutUpdateRequested);
}

void InspectorDockWidget::update()
{
    update(m_score);
}

const QList<Ms::EngravingItem*>* InspectorDockWidget::selectedElementList() const
{
    return m_score ? &m_score->selection().elements() : nullptr;
}

void InspectorDockWidget::update(Ms::Score* score)
{
    m_score = score;

    if (selectedElementList()) {
        m_inspectorForm->onSelectionChanged(*selectedElementList());
        emit selectionChanged(*selectedElementList());
    }
}

void InspectorDockWidget::changeEvent(QEvent* event)
{
    QDockWidget::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        update();
    }
}
