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
#include "drawlogger.h"

#include "log.h"

using namespace muse::draw;

static const std::string DRAW_OBJ_TAG("DRAW_OBJ");

void DrawObjectsLogger::beginObject(const std::string& name)
{
    m_objects.push(name);
    std::string gap;
    gap.resize(m_objects.size());
#ifdef LOG_STREAM
    LOG_STREAM(muse::logger::Logger::DEBG, DRAW_OBJ_TAG, muse::logger::Color::None)() << "Begin: " << gap << name;
#else
    UNUSED(pagePos);
#endif
}

void DrawObjectsLogger::endObject()
{
    IF_ASSERT_FAILED(!m_objects.empty()) {
        return;
    }

    std::string gap;
    gap.resize(m_objects.size());
#ifdef LOG_STREAM
    LOG_STREAM(muse::logger::Logger::DEBG, DRAW_OBJ_TAG, muse::logger::Color::None)() << "End:   " << gap << m_objects.top();
#endif

    m_objects.pop();
}
