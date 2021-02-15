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

#ifndef MU_NOTATION_LOOPMARKER_H
#define MU_NOTATION_LOOPMARKER_H

#include "notation/inotationconfiguration.h"
#include "notation/inotationstyle.h"
#include "modularity/ioc.h"

namespace mu::notation {
class LoopMarker
{
    INJECT(notation, INotationConfiguration, configuration)

public:
    LoopMarker(LoopBoundaryType type);

    void setRect(const QRect& rect);
    void setVisible(bool visible);
    void setStyle(INotationStylePtr style);

    void paint(QPainter* painter);

private:
    LoopBoundaryType m_type = LoopBoundaryType::Unknown;
    QRect m_rect;
    bool m_visible = false;
    INotationStylePtr m_style;
};
}

#endif // MU_NOTATION_LOOPMARKER_H
