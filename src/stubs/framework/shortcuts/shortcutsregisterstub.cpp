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
#include "shortcutsregisterstub.h"

using namespace mu::shortcuts;

const ShortcutList& ShortcutsRegisterStub::shortcuts() const
{
    static const ShortcutList list;
    return list;
}

mu::Ret ShortcutsRegisterStub::setShortcuts(const ShortcutList&)
{
    return make_ret(Ret::Code::NotImplemented);
}

mu::async::Notification ShortcutsRegisterStub::shortcutsChanged() const
{
    return async::Notification();
}

const Shortcut& ShortcutsRegisterStub::shortcut(const std::string&) const
{
    static Shortcut s;
    return s;
}

const Shortcut& ShortcutsRegisterStub::defaultShortcut(const std::string&) const
{
    static Shortcut s;
    return s;
}

ShortcutList ShortcutsRegisterStub::shortcutsForSequence(const std::string&) const
{
    return {};
}

mu::Ret ShortcutsRegisterStub::importFromFile(const io::path&)
{
    return make_ret(Ret::Code::NotImplemented);
}

mu::Ret ShortcutsRegisterStub::exportToFile(const io::path&) const
{
    return make_ret(Ret::Code::NotImplemented);
}
