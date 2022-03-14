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
#ifndef MU_APPSHELL_NAVIGABLEAPPMENUMODEL_H
#define MU_APPSHELL_NAVIGABLEAPPMENUMODEL_H

#include <QObject>

#include "appmenumodel.h"

namespace mu::appshell {
class NavigableAppMenuModel : public AppMenuModel
{
    Q_OBJECT

    Q_PROPERTY(QString highlightedMenuId READ highlightedMenuId NOTIFY highlightedMenuIdChanged)

    Q_PROPERTY(QWindow * appWindow READ appWindow WRITE setAppWindow)

public:
    explicit NavigableAppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

    QWindow* appWindow() const;

public slots:
    void setAppWindow(QWindow* appWindow);
    void setHighlightedMenuId(QString highlightedMenuId);

signals:
    void highlightedMenuIdChanged(QString highlightedMenuId);

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    bool isNavigationStarted() const;
    bool isNavigateKey(int key) const;
    void navigate(const QSet<int>& activatePossibleKeys);

    bool hasItem(const QSet<int>& activatePossibleKeys);
    void navigate(int scanCode);

    void resetNavigation();
    void navigateToFirstMenu();

    void saveMUNavigationSystemState();
    void restoreMUNavigationSystemState();

    void activateHighlightedMenu();

    QString highlightedMenuId() const;

    QString menuId(const QSet<int>& activatePossibleKeys);

    QString m_highlightedMenuId;

    bool m_needActivateHighlight = true;
    ui::INavigationControl* m_lastActiveNavigationControl = nullptr;

    QWindow* m_appWindow = nullptr;
};
}

#endif // MU_APPSHELL_NAVIGABLEAPPMENUMODEL_H
