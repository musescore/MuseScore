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
#include "playbackcursor.h"

#include <QPainter>

using namespace mu::notation;

void PlaybackCursor::paint(QPainter* painter)
{
    if (!m_visible) {
        return;
    }

    painter->fillRect(m_rect, color());
}

void PlaybackCursor::move(const QRect& rect)
{
    m_rect = rect;
}

const QRect& PlaybackCursor::rect() const
{
    return m_rect;
}

void PlaybackCursor::setVisible(bool arg)
{
    m_visible = arg;
}

QColor PlaybackCursor::color()
{
    QColor color = configuration()->playbackCursorColor();
    color.setAlpha(configuration()->cursorOpacity());
    return color;
}
