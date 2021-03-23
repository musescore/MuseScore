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

#include "templatepaintview.h"

#include "log.h"

#include "notation/imasternotation.h"

using namespace mu::userscores;
using namespace mu::notation;

TemplatePaintView::TemplatePaintView(QQuickItem* parent)
    : NotationPaintView(parent)
{
    setReadonly(true);
}

void TemplatePaintView::load(const QString& templatePath)
{
    if (templatePath.isEmpty()) {
        return;
    }

    if (m_templatePath == templatePath) {
        return;
    }

    m_templatePath = templatePath;
    load();
}

void TemplatePaintView::load()
{
    IMasterNotationPtr masterNotation = notationCreator()->newMasterNotation();
    Ret ret = masterNotation->load(m_templatePath);

    if (!ret) {
        LOGE() << ret.toString();
    }

    setNotation(masterNotation->notation());

    if (masterNotation) {
        adjustCanvas();
    }
}

void TemplatePaintView::adjustCanvas()
{
    qreal scaling = resolveDefaultScaling();

    if (qFuzzyIsNull(scaling) || scaling < 0) {
        return;
    }

    scale(scaling, QPoint());
    moveCanvasToCenter();
}

qreal TemplatePaintView::resolveDefaultScaling() const
{
    //! NOTE: this value was found experimentally
    constexpr qreal PROPORTION_FACTOR = 1.2;

    QRectF notationRect = notationContentRect();

    qreal widthScaling = width() * guiScaling() / notationRect.width() / PROPORTION_FACTOR;
    qreal heightScaling = height() * guiScaling() / notationRect.height() / PROPORTION_FACTOR;

    return std::min(widthScaling, heightScaling);
}

void TemplatePaintView::onViewSizeChanged()
{
    adjustCanvas();
}

void TemplatePaintView::zoomIn()
{
    handleAction("zoomin");
}

void TemplatePaintView::zoomOut()
{
    handleAction("zoomout");
}
