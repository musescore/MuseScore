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
#ifndef MUSE_API_KEYBOARDAPI_H
#define MUSE_API_KEYBOARDAPI_H

#include "api/apiobject.h"

#include "modularity/ioc.h"
#include "shortcuts/ishortcutscontroller.h"
#include "ui/imainwindow.h"

class QWindow;
namespace muse::api {
class KeyboardApi : public api::ApiObject
{
    Q_OBJECT

    Inject<shortcuts::IShortcutsController> shortcutsController = { this };
    Inject<ui::IMainWindow> mainWindow = { this };

public:
    explicit KeyboardApi(api::IApiEngine* e);

    Q_INVOKABLE void key(const QString& key, const QString& mod = QString());
    Q_INVOKABLE void pressKey(const QString& key, const QString& mod = QString());
    Q_INVOKABLE void releaseKey(const QString& key, const QString& mod = QString());
    Q_INVOKABLE void repeatKey(const QString& key, int count);
    Q_INVOKABLE void text(const QString& text);

private:

    QWindow* window() const;
    void doPressKey(QWindow* w, int code, Qt::KeyboardModifier mod, const QString& text);
    void doReleaseKey(QWindow* w, int key, Qt::KeyboardModifier mod, const QString& text);
};
}

#endif // MUSE_API_KEYBOARDAPI_H
