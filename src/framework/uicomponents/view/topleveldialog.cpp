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

#include "topleveldialog.h"

#include <QWindow>

using namespace muse::uicomponents;

TopLevelDialog::TopLevelDialog(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

#ifdef Q_OS_MAC
    setWindowFlags(Qt::Tool);
#endif
}

bool TopLevelDialog::event(QEvent* e)
{
#ifndef Q_OS_MAC
    if (e->type() == QEvent::Show) {
        windowHandle()->setTransientParent(mainWindow()->qWindow());
    }
#endif

    return QDialog::event(e);
}
