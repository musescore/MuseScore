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
#ifndef MU_USERSCORES_SCORETHUMBNAIL_H
#define MU_USERSCORES_SCORETHUMBNAIL_H

#include <QQuickPaintedItem>
#include <QPainter>

namespace mu {
namespace userscores {
class ScoreThumbnail : public QQuickPaintedItem
{
    Q_OBJECT

public:
    ScoreThumbnail(QQuickItem* parent = nullptr);

    Q_INVOKABLE void setThumbnail(QVariant pixmap);

protected:
    virtual void paint(QPainter* painter) override;

private:
    QPixmap m_thumbnail;
};
}
}

#endif // MU_USERSCORES_SCORETHUMBNAIL_H
