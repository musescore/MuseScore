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
#ifndef MUSE_SHORTCUTS_SHORTCUTSCONFIGURATIONSTUB_H
#define MUSE_SHORTCUTS_SHORTCUTSCONFIGURATIONSTUB_H

#include "shortcuts/ishortcutsconfiguration.h"

namespace muse::shortcuts {
class ShortcutsConfigurationStub : public IShortcutsConfiguration
{
public:
    QString currentKeyboardLayout() const override;
    void setCurrentKeyboardLayout(const QString& layout) override;

    io::path_t shortcutsUserAppDataPath() const override;
    io::path_t shortcutsAppDataPath() const override;

    io::path_t midiMappingUserAppDataPath() const override;

    bool advanceToNextNoteOnKeyRelease() const override;
    void setAdvanceToNextNoteOnKeyRelease(bool value) override;
};
}

#endif // MUSE_SHORTCUTS_SHORTCUTSCONFIGURATIONSTUB_H
