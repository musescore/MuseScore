//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_NOTATION_PLAYBACKCURSOR_H
#define MU_NOTATION_PLAYBACKCURSOR_H

#include <QRect>
#include <QColor>

class QPainter;

namespace mu {
namespace notation {
class PlaybackCursor
{
public:

    PlaybackCursor() = default;

    void paint(QPainter* painter);
    void move(const QRect& rect);

    const QRect& rect() const;
    void setVisible(bool arg);
    void setColor(const QColor& c);

private:

    bool m_visible = false;
    QRect m_rect;
    QColor m_color;
};
}
}

#endif // MU_NOTATION_PLAYBACKCURSOR_H
