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
#ifndef MUSE_DRAW_DRAWLOGGER_H
#define MUSE_DRAW_DRAWLOGGER_H

#include <stack>
#include <string>

namespace muse::draw {
class DrawObjectsLogger
{
public:
    DrawObjectsLogger() = default;

    void beginObject(const std::string& name);
    void endObject();

private:

    std::stack<std::string> m_objects;
};
}

#endif // MUSE_DRAW_DRAWLOGGER_H
