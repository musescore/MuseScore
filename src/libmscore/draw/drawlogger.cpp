//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "drawlogger.h"

#include "log.h"

#include <QPointF>

using namespace mu::draw;

static const std::string DRAW_OBJ_TAG("DRAW_OBJ");

void DrawObjectsLogger::beginObject(const std::string& name, const QPointF& pagePos)
{
    m_objects.push(name);
    std::string gap;
    gap.resize(m_objects.size());
    LOG_STREAM(haw::logger::Logger::DEBG, DRAW_OBJ_TAG) << "Begin: " << gap << name << "{" << pagePos.x() << "," << pagePos.y() << "}";
}

void DrawObjectsLogger::endObject()
{
    IF_ASSERT_FAILED(!m_objects.empty()) {
        return;
    }

    std::string gap;
    gap.resize(m_objects.size());
    LOG_STREAM(haw::logger::Logger::DEBG, DRAW_OBJ_TAG) << "End:   " << gap << m_objects.top();

    m_objects.pop();
}
