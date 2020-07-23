//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

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

const QList<Ms::Element*>* InspectorDockWidget::selectedElementList() const
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
