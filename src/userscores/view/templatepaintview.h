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
#ifndef MU_USERSCORES_TEMPLATEPAINTVIEW_H
#define MU_USERSCORES_TEMPLATEPAINTVIEW_H

#include <QQuickPaintedItem>

#include "modularity/ioc.h"
#include "notation/inotationcreator.h"
#include "notation/imasternotation.h"
#include "iuserscoresconfiguration.h"
#include "async/asyncable.h"

namespace mu {
namespace userscores {
class TemplatePaintView : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(userscores, notation::INotationCreator, notationCreator)
    INJECT(userscores, IUserScoresConfiguration, configuration)

    Q_PROPERTY(qreal startHorizontalScrollPosition READ startHorizontalScrollPosition NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal horizontalScrollSize READ horizontalScrollSize NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal startVerticalScrollPosition READ startVerticalScrollPosition NOTIFY verticalScrollChanged)
    Q_PROPERTY(qreal verticalScrollSize READ verticalScrollSize NOTIFY verticalScrollChanged)

public:
    explicit TemplatePaintView(QQuickItem* parent = nullptr);

    qreal startHorizontalScrollPosition() const;
    qreal horizontalScrollSize() const;
    qreal startVerticalScrollPosition() const;
    qreal verticalScrollSize() const;

    Q_INVOKABLE void load(const QString& templatePath);

    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();

    Q_INVOKABLE void scrollHorizontal(qreal position);
    Q_INVOKABLE void scrollVertical(qreal position);

signals:
    void horizontalScrollChanged();
    void verticalScrollChanged();

private:
    qreal horizontalScrollableAreaSize() const;
    qreal verticalScrollableAreaSize() const;

    void paint(QPainter* painter) override;

    qreal canvasWidth() const;
    qreal canvasHeight() const;

    void moveCanvasToCenter();

    void scaleCanvas(qreal scaleFactor);
    bool canvasScaled() const;

private slots:
    void onViewSizeChanged();

private:
    QString m_templatePath;
    notation::IMasterNotationPtr m_notation;
    QColor m_backgroundColor;

    qreal m_previousVerticalScrollPosition = 0;
    qreal m_previousHorizontalScrollPosition = 0;

    qreal m_currentScaleFactor = 0;
    qreal m_dx = 0;
    qreal m_dy = 0;
};
}
}

#endif // MU_USERSCORES_TEMPLATEPAINTVIEW_H
