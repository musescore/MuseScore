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

#include "widgetnavigationfix.h"

#include <QWidget>
#include <QTableWidget>

using namespace mu::ui;

bool WidgetNavigationFix::fixNavigationForTableWidget(const WidgetNavigationFix::NavigationChain* chain, int key)
{
    QTableWidget* tableWidget = qobject_cast<QTableWidget*>(chain->widget);

    if (!tableWidget || !tableWidget->hasFocus()) {
        return false;
    }

    switch (key) {
    case Qt::Key_Tab: {
        if (!chain->nextWidget) {
            return false;
        }

        auto lastItem = tableWidget->item(tableWidget->rowCount() - 1, tableWidget->columnCount() - 1);
        if (lastItem == tableWidget->currentItem()) {
            chain->nextWidget->setFocus();
            return true;
        }
        break;
    }
    case Qt::Key_Backtab: {
        if (!chain->prevWidget) {
            return false;
        }

        auto firstItem = tableWidget->item(0, 0);
        if (firstItem == tableWidget->currentItem()) {
            chain->prevWidget->setFocus();
            return true;
        }
        break;
    }
    default:
        break;
    }

    return false;
}
