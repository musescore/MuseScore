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
#ifndef MU_SHORTCUTS_SHORTCUTSCONFIGURATION_H
#define MU_SHORTCUTS_SHORTCUTSCONFIGURATION_H

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "ishortcutsconfiguration.h"
#include "async/asyncable.h"

namespace mu::shortcuts {
class ShortcutsConfiguration : public IShortcutsConfiguration, public async::Asyncable
{
    INJECT(shortcuts, framework::IGlobalConfiguration, globalConfiguration)

public:
    void init();

    ValCh<io::path> shortcutsUserPath() const override;
    void setShortcutsUserPath(const io::path& path) override;

    io::path shortcutsDefaultPath() const override;

    io::path midiMappingsPath() const override;

private:
    async::Channel<io::path> m_userPathChanged;
};
}

#endif // MU_SHORTCUTS_SHORTCUTSCONTROLLER_H
