//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include <QDockWidget>
#include <QQmlEngine>

#include "libmscore/element.h"
#include "libmscore/score.h"

class InspectorFormWidget;

class InspectorDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit InspectorDockWidget(QQmlEngine* engine, QWidget* parent = nullptr);

    void update(Ms::Score* score);

public slots:
    void update();

protected:
    void changeEvent(QEvent* event) override;

signals:
    void propertyEditStarted(Ms::Element* element);
    void selectionChanged(const QList<Ms::Element*>& elementList);
    void layoutUpdateRequested();

private:
    const QList<Ms::Element*>* selectedElementList() const;

    Ms::Score* m_score = nullptr;
    QQmlEngine* m_qmlEngine = nullptr;
    InspectorFormWidget* m_inspectorForm = nullptr;
};

#endif
