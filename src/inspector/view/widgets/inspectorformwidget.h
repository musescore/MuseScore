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
#ifndef INSPECTORELEMENT_H
#define INSPECTORELEMENT_H

#include "inspectordockwidget.h"
#include <QQuickWidget>

#include "models/abstractinspectormodel.h"
#include "models/inspectorlistmodel.h"

class InspectorFormWidget : public QQuickWidget
{
    Q_OBJECT

public:
    explicit InspectorFormWidget(QQmlEngine* qmlEngine, QWidget* parent = nullptr);

public slots:
    void onSelectionChanged(const QList<Ms::EngravingItem*>& elementList);

signals:
    void layoutUpdateRequested();

private:
    InspectorDockWidget* m_inspector = nullptr;

    InspectorListModel* m_inspectorListModel = nullptr;

protected:
    void focusInEvent(QFocusEvent* event) override;
};

#endif // INSPECTORELEMENT_H
