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

#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include <QDockWidget>
#include <QQmlEngine>

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/masterscore.h"

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
    void propertyEditStarted(Ms::EngravingItem* element);
    void selectionChanged(const QList<Ms::EngravingItem*>& elementList);
    void layoutUpdateRequested();

private:
    const QList<Ms::EngravingItem*>* selectedElementList() const;

    Ms::Score* m_score = nullptr;
    QQmlEngine* m_qmlEngine = nullptr;
    InspectorFormWidget* m_inspectorForm = nullptr;
};

#endif
