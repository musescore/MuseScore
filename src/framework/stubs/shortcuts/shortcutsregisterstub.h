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
#ifndef MUSE_SHORTCUTS_SHORTCUTSREGISTERSTUB_H
#define MUSE_SHORTCUTS_SHORTCUTSREGISTERSTUB_H

#include "shortcuts/ishortcutsregister.h"

namespace muse::shortcuts {
class ShortcutsRegisterStub : public IShortcutsRegister
{
public:
    const ShortcutList& shortcuts() const override;
    Ret setShortcuts(const ShortcutList& shortcuts) override;
    void resetShortcuts() override;
    async::Notification shortcutsChanged() const override;

    Ret setAdditionalShortcuts(const std::string& context, const ShortcutList& shortcuts) override;

    const Shortcut& shortcut(const std::string& actionCode) const override;
    const Shortcut& defaultShortcut(const std::string& actionCode) const override;

    bool isRegistered(const std::string& sequence) const override;
    ShortcutList shortcutsForSequence(const std::string& sequence) const override;

    Ret importFromFile(const io::path_t& filePath) override;
    Ret exportToFile(const io::path_t& filePath) const override;

    bool active() override;
    void setActive(bool active) override;
    async::Notification activeChanged() const override;

    // for autobot tests
    void reload(bool onlyDef = false) override;
};
}

#endif // MUSE_SHORTCUTS_SHORTCUTSREGISTERSTUB_H
