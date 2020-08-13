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
#include "io/path.h"

using namespace mu::userscores;
using namespace mu::notation;

//! NOTE: experimental values
static constexpr qreal INITIAL_SCALE_FACTOR = 0.05;
static constexpr qreal SCALE_FACTOR_STEP = 0.01;

static constexpr qreal MIN_SCROLL_SIZE = 0.1;
static constexpr qreal MAX_SCROLL_SIZE = 1.0;
static constexpr qreal MID_SCROLL_POSITION = (MAX_SCROLL_SIZE - MIN_SCROLL_SIZE) / 2.0;

TemplatePaintView::TemplatePaintView(QQuickItem* parent)
    : QQuickPaintedItem(parent), m_currentScaleFactor(INITIAL_SCALE_FACTOR)
{
    setAntialiasing(true);

    m_backgroundColor = configuration()->templatePreviewBackgroundColor();
    configuration()->templatePreviewBackgroundColorChanged().onReceive(this, [this](const QColor& color) {
        m_backgroundColor = color;
        update();
    });

    connect(this, &QQuickPaintedItem::widthChanged, this, &TemplatePaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &TemplatePaintView::onViewSizeChanged);
}

qreal TemplatePaintView::startHorizontalScrollPosition() const
{
    if (horizontalScrollSize() == MIN_SCROLL_SIZE) {
        return MID_SCROLL_POSITION;
    }

    return MAX_SCROLL_SIZE - horizontalScrollableAreaSize();
}

qreal TemplatePaintView::horizontalScrollSize() const
{
    qreal area = horizontalScrollableAreaSize();
    if (qFuzzyIsNull(area)) {
        return 0;
    }

    qreal size = area - (MAX_SCROLL_SIZE - area);
    size = std::max(size, MIN_SCROLL_SIZE);

    return size;
}

qreal TemplatePaintView::horizontalScrollableAreaSize() const
{
    if (canvasWidth() <= width() || !canvasScaled()) {
        return 0;
    }

    return width() / canvasWidth();
}

qreal TemplatePaintView::startVerticalScrollPosition() const
{
    if (verticalScrollSize() == MIN_SCROLL_SIZE) {
        return MID_SCROLL_POSITION;
    }

    return MAX_SCROLL_SIZE - verticalScrollableAreaSize();
}

qreal TemplatePaintView::verticalScrollSize() const
{
    qreal area = verticalScrollableAreaSize();
    if (qFuzzyIsNull(area)) {
        return 0;
    }

    qreal size = area - (MAX_SCROLL_SIZE - area);
    size = std::max(size, MIN_SCROLL_SIZE);

    return size;
}

qreal TemplatePaintView::verticalScrollableAreaSize() const
{
    if (canvasHeight() <= height() || !canvasScaled()) {
        return 0;
    }

    return height() / canvasHeight();
}

void TemplatePaintView::load(const QString& templatePath)
{
    if (m_templatePath == templatePath) {
        return;
    }

    m_templatePath = templatePath;
    m_notation = notationCreator()->newMasterNotation();

    Ret ret = m_notation->load(m_templatePath);
    if (!ret) {
        LOGE() << ret.toString();
    }

    scaleCanvas(INITIAL_SCALE_FACTOR);
}

void TemplatePaintView::moveCanvasToCenter()
{
    m_dx = (width() - canvasWidth()) / 2.;
    m_dy = (height() - canvasHeight()) / 2.;

    m_previousHorizontalScrollPosition = 0;
    m_previousVerticalScrollPosition = 0;

    update();
}

void TemplatePaintView::onViewSizeChanged()
{
    moveCanvasToCenter();
}

void TemplatePaintView::zoomIn()
{
    scaleCanvas(m_currentScaleFactor + SCALE_FACTOR_STEP);
}

void TemplatePaintView::zoomOut()
{
    scaleCanvas(m_currentScaleFactor - SCALE_FACTOR_STEP);
}

void TemplatePaintView::scaleCanvas(qreal scaleFactor)
{
    if (scaleFactor < INITIAL_SCALE_FACTOR) {
        scaleFactor = INITIAL_SCALE_FACTOR;
    }

    m_currentScaleFactor = scaleFactor;
    moveCanvasToCenter();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
}

bool TemplatePaintView::canvasScaled() const
{
    return !qFuzzyCompare(m_currentScaleFactor, INITIAL_SCALE_FACTOR);
}

void TemplatePaintView::scrollHorizontal(qreal position)
{
    if (position == m_previousHorizontalScrollPosition) {
        return;
    }

    if (qFuzzyIsNull(m_previousHorizontalScrollPosition)) {
        m_previousHorizontalScrollPosition = position;
        return;
    }

    qreal scrollStep = position - m_previousHorizontalScrollPosition;
    m_dx -= canvasWidth() * scrollStep;

    m_previousHorizontalScrollPosition = position;
    update();
}

void TemplatePaintView::scrollVertical(qreal position)
{
    if (position == m_previousVerticalScrollPosition) {
        return;
    }

    if (qFuzzyIsNull(m_previousVerticalScrollPosition)) {
        m_previousVerticalScrollPosition = position;
        return;
    }

    qreal scrollStep = position - m_previousVerticalScrollPosition;
    m_dy -= canvasHeight() * scrollStep;

    m_previousVerticalScrollPosition = position;
    update();
}

qreal TemplatePaintView::canvasWidth() const
{
    if (m_notation) {
        return m_notation->previewRect().width() * m_currentScaleFactor;
    }

    return 0;
}

qreal TemplatePaintView::canvasHeight() const
{
    if (m_notation) {
        return m_notation->previewRect().height() * m_currentScaleFactor;
    }

    return 0;
}

void TemplatePaintView::paint(QPainter* painter)
{
    QRect rect(0, 0, width(), height());

    painter->fillRect(rect, m_backgroundColor);
    painter->translate(m_dx, m_dy);
    painter->scale(m_currentScaleFactor, m_currentScaleFactor);

    if (m_notation) {
        m_notation->paint(painter);
    }
}
