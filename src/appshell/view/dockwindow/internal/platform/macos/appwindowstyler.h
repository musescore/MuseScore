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
#ifndef MU_APPSHELL_APPWINDOWSTYLER_H
#define MU_APPSHELL_APPWINDOWSTYLER_H

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"

#include <QObject>

class QWindow;

namespace mu::appshell {
class AppWindowStyler : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, ui::IUiConfiguration, uiConfiguration)

    Q_PROPERTY(QWindow * targetWindow READ targetWindow WRITE setTargetWindow NOTIFY targetWindowChanged)

public:
    explicit AppWindowStyler(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    QWindow* targetWindow() const;
    void setTargetWindow(QWindow* window);

signals:
    void targetWindowChanged();

private:
    QWindow* m_targetWindow;
};
}

#endif // MU_APPSHELL_APPWINDOWSTYLER_H
