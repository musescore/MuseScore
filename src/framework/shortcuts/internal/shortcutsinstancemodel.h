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
#ifndef MUSE_SHORTCUTS_SHORTCUTSINSTANCEMODEL_H
#define MUSE_SHORTCUTS_SHORTCUTSINSTANCEMODEL_H

#include <QObject>
#include <QString>
#include <QList>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ishortcutsregister.h"
#include "ishortcutscontroller.h"
#include "ui/imainwindow.h"

namespace muse::shortcuts {
class ShortcutsInstanceModel : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap shortcuts READ shortcuts NOTIFY shortcutsChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    Inject<IShortcutsRegister> shortcutsRegister = { this };
    Inject<IShortcutsController> controller = { this };
    Inject<ui::IMainWindow> mainWindow = { this };

public:
    explicit ShortcutsInstanceModel(QObject* parent = nullptr);

    QVariantMap shortcuts() const;
    bool active() const;

    Q_INVOKABLE void init();
    Q_INVOKABLE void activate(const QString& seq);
    Q_INVOKABLE void activateAmbiguous(const QString& seq);

signals:
    void shortcutsChanged();
    void activeChanged();

protected:
    virtual void doLoadShortcuts();
    virtual void doActivate(std::vector<QString> sequences);

    // Key = sequence (QString), value = autoRepeat (QVariant/bool)
    QVariantMap m_shortcuts;

private:
    std::map<int, int> m_shortcutSequences;

    bool eventFilter(QObject* watched, QEvent* event);
};
}

#endif // MUSE_SHORTCUTS_SHORTCUTSINSTANCEMODEL_H
