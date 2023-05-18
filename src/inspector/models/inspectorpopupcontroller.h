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

#ifndef MU_INSPECTOR_INSPECTORPOPUPCONTROLLER_H
#define MU_INSPECTOR_INSPECTORPOPUPCONTROLLER_H

#include <QObject>

#include "modularity/ioc.h"
#include "ui/imainwindow.h"

namespace mu::uicomponents {
class PopupView;
}

class QQuickItem;

namespace mu::inspector {
class InspectorPopupController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem * visualControl READ visualControl WRITE setVisualControl NOTIFY visualControlChanged)
    Q_PROPERTY(mu::uicomponents::PopupView * popup READ popup WRITE setPopup NOTIFY popupChanged)

    Q_PROPERTY(QQuickItem * notationView READ notationView WRITE setNotationView NOTIFY notationViewChanged)

    INJECT(ui::IMainWindow, mainWindow)

public:
    explicit InspectorPopupController(QObject* parent = nullptr);
    ~InspectorPopupController() override;

    Q_INVOKABLE void load();

    QQuickItem* visualControl() const;
    uicomponents::PopupView* popup() const;

    QQuickItem* notationView() const;

public slots:
    void setVisualControl(QQuickItem* control);
    void setPopup(uicomponents::PopupView* popup);
    void setNotationView(QQuickItem* notationView);

signals:
    void visualControlChanged();
    void popupChanged();
    void notationViewChanged(QQuickItem* notationView);

private slots:
    void closePopup();
    void doClosePopup();

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void closePopupIfNeed(const QPoint& mouseGlobalPos);

    QQuickItem* m_visualControl = nullptr;
    uicomponents::PopupView* m_popup = nullptr;
    QQuickItem* m_notationView = nullptr;
};
}

#endif // MU_INSPECTOR_INSPECTORPOPUPCONTROLLER_H
