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
#ifndef MU_DRAW_DRAWCOMP_H
#define MU_DRAW_DRAWCOMP_H

#include <functional>

#include "drawtypes.h"

namespace mu::draw {
class DrawComp
{
public:

    struct Tolerance {
        double base = -1.0;
        Tolerance() {}
    };

    static Diff compare(const DrawDataPtr& data, const DrawDataPtr& origin, Tolerance tolerance = Tolerance());
};
}

#endif // MU_DRAW_DRAWCOMP_H
