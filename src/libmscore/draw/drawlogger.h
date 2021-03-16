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
#ifndef MU_DRAW_DRAWLOGGER_H
#define MU_DRAW_DRAWLOGGER_H

#include <stack>
#include <string>

class QPointF;

namespace mu::draw {
class DrawObjectsLogger
{
public:
    DrawObjectsLogger() = default;

    void beginObject(const std::string& name, const QPointF& pagePos);
    void endObject();

private:

    std::stack<std::string> m_objects;
};
}

#endif // MU_DRAW_DRAWLOGGER_H
