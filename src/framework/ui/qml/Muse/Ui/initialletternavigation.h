/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include <qqmlintegration.h>

#include "abstractnavigation.h"

Q_MOC_INCLUDE("ui/qml/Muse/Ui/navigationpanel.h")

namespace muse::ui {
class NavigationPanel;
class InitialLetterNavigation : public QObject, public muse::Injectable
{
    Q_OBJECT
    Q_PROPERTY(QStringList stringList READ stringList WRITE setStringList NOTIFY stringListChanged)
    Q_PROPERTY(muse::ui::NavigationPanel * panel READ panel WRITE setPanel NOTIFY panelChanged)
    Q_PROPERTY(int controlColumn READ controlColumn WRITE setControlColumn NOTIFY controlColumnChanged)
    QML_ELEMENT

    Inject<INavigationController> navigationController = { this };

public:
    explicit InitialLetterNavigation(QObject* parent = nullptr);

    QStringList stringList() const;
    void setStringList(const QStringList& stringList);

    NavigationPanel* panel() const;
    void setPanel(NavigationPanel* panel);

    int controlColumn() const;
    void setControlColumn(int controlColumn);

signals:
    void stringListChanged();
    void panelChanged();
    void controlColumnChanged();

    void requestVisible(int index);

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    int indexForInput(const QString& eventText) const;
    void navigateToIndex(const INavigation::Index& index);

    void clearBuffer();

    QStringList m_stringList;
    NavigationPanel* m_panel = nullptr;
    int m_controlColumn = -1;

    QString m_inputString;
    QString m_prevEventText;
    bool m_cycling = true;

    QTimer m_inputBufferTimer;
};
}
