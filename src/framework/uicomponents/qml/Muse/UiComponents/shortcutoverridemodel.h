/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "shortcuts/ishortcutsregister.h"
#include "actions/iactionsdispatcher.h"

namespace muse::uicomponents {
class ShortcutOverrideModel : public QObject, public muse::Contextable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(DirectionKeys directionKeysForOverride READ directionKeysForOverride
               WRITE setDirectionKeysForOverride NOTIFY directionKeysForOverrideChanged)

    QML_ELEMENT;

    muse::ContextInject<shortcuts::IShortcutsRegister> shortcutsRegister = { this };
    muse::ContextInject<actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit ShortcutOverrideModel(QObject* parent = nullptr);

    enum class DirectionKey {
        None      = 0x0,
        LeftRight = 0x1,
        UpDown    = 0x2,
        All       = LeftRight | UpDown,
    };
    Q_DECLARE_FLAGS(DirectionKeys, DirectionKey)
    Q_FLAG(DirectionKeys)

    Q_INVOKABLE void init();
    Q_INVOKABLE bool isShortcutOverrideAllowed(Qt::Key key, Qt::KeyboardModifiers modifiers) const;
    Q_INVOKABLE bool handleShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers);

    DirectionKeys directionKeysForOverride() const;
    void setDirectionKeysForOverride(const DirectionKeys& keys);

signals:
    void directionKeysForOverrideChanged();

private:
    void loadDisallowedOverrides();
    shortcuts::Shortcut disallowedOverride(Qt::Key key, Qt::KeyboardModifiers modifiers) const;

    shortcuts::ShortcutList m_notAllowedForOverrideShortcuts;
    DirectionKeys m_directionKeysForOverride = DirectionKey::None;
};
}
