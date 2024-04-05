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
#ifndef MUSE_SHORTCUTS_SHORTCUTSCONTROLLERSTUB_H
#define MUSE_SHORTCUTS_SHORTCUTSCONTROLLERSTUB_H

#include "shortcuts/ishortcutscontroller.h"

namespace muse::shortcuts {
class ShortcutsControllerStub : public IShortcutsController
{
public:
    void activate(const std::string& sequence) override;
    bool isRegistered(const std::string& sequence) const override;
};
}

#endif // MUSE_SHORTCUTS_SHORTCUTSCONTROLLERSTUB_H
