/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "timeline.h"

#include <QApplication>
#include <QGraphicsTextItem>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTextDocument>

#include "translation.h"

#include "engraving/dom/barline.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurebase.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/page.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/timesig.h"
#include "engraving/types/typesconv.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

//---------------------------------------------------------
//   TRowLabels
//---------------------------------------------------------

TRowLabels::TRowLabels(QSplitter* splitter, Timeline* time)
    : QGraphicsView(splitter)
{
    TRACEFUNC;

    setFocusPolicy(Qt::NoFocus);
    setObjectName("TRowLabels");

    _splitter = splitter;
    _timeline = time;
    setScene(new QGraphicsScene);
    scene()->setBackgroundBrush(time->activeTheme().backgroundColor);
    setSceneRect(0, 0, 50, time->height());

    setMinimumWidth(0);

    QSplitter* split = _splitter;
    QList<int> sizes;
    // TODO: Replace 70 with hard coded value
    sizes << 70 << 10000;
    split->setSizes(sizes);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContentsMargins(0, 0, 0, 0);
    setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));

    connect(verticalScrollBar(), &QScrollBar::valueChanged, time->verticalScrollBar(), &QScrollBar::setValue);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TRowLabels::restrictScroll);
    connect(this, &TRowLabels::moved, time, &Timeline::mouseOver);

    static const char* udArrow[] = {
        "10 18 2 1",
        "# c #000000",
        ". c #d3d3d3",
        "..........",
        "..........",
        "....##....",
        "...####...",
        "..##..##..",
        "..#....#..",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..#....#..",
        "..##..##..",
        "...####...",
        "....##....",
        "..........",
        ".........."
    };

    static const char* uArrow[] = {
        "10 18 2 1",
        "# c #000000",
        ". c #d3d3d3",
        "..........",
        "..........",
        "....##....",
        "...####...",
        "..##..##..",
        "..#....#..",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        ".........."
    };

    static const char* dArrow[] = {
        "10 18 2 1",
        "# c #000000",
        ". c #d3d3d3",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..#....#..",
        "..##..##..",
        "...####...",
        "....##....",
        "..........",
        ".........."
    };

    static const char* cuArrow[] = {
        "9 18 2 1",
        "# c #000000",
        ". c #d3d3d3",
        ".........",
        ".........",
        ".........",
        ".........",
        "....#....",
        "...###...",
        "..#.#.#..",
        ".#..#..#.",
        "....#....",
        "....#....",
        "....#....",
        "....#....",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "........."
    };

    static const char* cdArrow[] = {
        "9 18 2 1",
        "# c #000000",
        ". c #d3d3d3",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "....#....",
        "....#....",
        "....#....",
        "....#....",
        "....#....",
        ".#..#..#.",
        "..#.#.#..",
        "...###...",
        "....#....",
        ".........",
        ".........",
        ".........",
        "........."
    };

    static const char* openEye[] = {
        "11 18 2 1",
        "# c #000000",
        ". c #d3d3d3",
        "...........",
        "...........",
        "...........",
        "...........",
        "...####....",
        ".##....##..",
        ".#......#..",
        "#........#.",
        "#...##...#.",
        "#...##...#.",
        "#........#.",
        ".#......#..",
        ".##....##..",
        "...####....",
        "...........",
        "...........",
        "...........",
        "..........."
    };

    static const char* closedEye[] = {
        "11 18 2 1",
        "# c #000000",
        ". c #d3d3d3",
        "...........",
        "...........",
        "...........",
        "...........",
        "...........",
        "...........",
        "...........",
        "..######...",
        "##..##..##.",
        "##..##..##.",
        "..######...",
        "...........",
        "...........",
        "...........",
        "...........",
        "...........",
        "...........",
        "..........."
    };

    _mouseoverMap[MouseOverValue::COLLAPSE_DOWN_ARROW] = new QPixmap(cdArrow);
    _mouseoverMap[MouseOverValue::COLLAPSE_UP_ARROW] = new QPixmap(cuArrow);
    _mouseoverMap[MouseOverValue::MOVE_DOWN_ARROW] = new QPixmap(dArrow);
    _mouseoverMap[MouseOverValue::MOVE_UP_DOWN_ARROW] = new QPixmap(udArrow);
    _mouseoverMap[MouseOverValue::MOVE_UP_ARROW] = new QPixmap(uArrow);
    _mouseoverMap[MouseOverValue::OPEN_EYE] = new QPixmap(openEye);
    _mouseoverMap[MouseOverValue::CLOSED_EYE] = new QPixmap(closedEye);

    std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> tmp(nullptr, MouseOverValue::NONE, -1);
    _oldItemInfo = tmp;

    connect(this, &TRowLabels::requestContextMenu, _timeline, &Timeline::contextMenuEvent);
}

bool TRowLabels::handleEvent(QEvent* e)
{
    return QWidget::event(e);
}

//---------------------------------------------------------
//   TRowLabels::restrictScroll
//---------------------------------------------------------

void TRowLabels::restrictScroll(int value)
{
    TRACEFUNC;

    if (value > _timeline->verticalScrollBar()->maximum()) {
        verticalScrollBar()->setValue(_timeline->verticalScrollBar()->maximum());
    }
    for (std::vector<std::pair<QGraphicsItem*, int> >::iterator it = _metaLabels.begin();
         it != _metaLabels.end(); ++it) {
        std::pair<QGraphicsItem*, int> pairGraphicInt = *it;

        QGraphicsItem* graphicsItem = pairGraphicInt.first;

        QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
        QGraphicsLineItem* graphicsLineItem = qgraphicsitem_cast<QGraphicsLineItem*>(graphicsItem);
        QGraphicsPixmapItem* graphicsPixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphicsItem);
        int y = pairGraphicInt.second * 20;
        int scrollbarValue = verticalScrollBar()->value();

        if (graphicsRectItem) {
            QRectF rectf = graphicsRectItem->rect();
            rectf.setY(qreal(scrollbarValue + y));
            rectf.setHeight(20);
            graphicsRectItem->setRect(rectf);
        } else if (graphicsLineItem) {
            QLineF linef = graphicsLineItem->line();
            linef.setLine(linef.x1(), y + scrollbarValue + 1, linef.x2(), y + scrollbarValue + 1);
            graphicsLineItem->setLine(linef);
        } else if (graphicsPixmapItem) {
            graphicsPixmapItem->setY(qreal(scrollbarValue + y + 1));
        } else {
            graphicsItem->setY(qreal(scrollbarValue + y));
        }
    }
    viewport()->update();
}

//---------------------------------------------------------
//   TRowLabels::updateLabels
//---------------------------------------------------------

void TRowLabels::updateLabels(std::vector<std::pair<QString, bool> > labels, int height)
{
    TRACEFUNC;

    scene()->clear();
    _metaLabels.clear();
    if (labels.empty()) {
        return;
    }

    unsigned numMetas = _timeline->nmetas();
    int maxWidth = -1;
    int measureWidth = 0;
    for (unsigned row = 0; row < labels.size(); row++) {
        // Draw instrument name rectangle
        int ypos = (row < numMetas) ? row * height + verticalScrollBar()->value() : row * height + 3;
        QGraphicsRectItem* graphicsRectItem = new QGraphicsRectItem(0, ypos, width(), height);
        QGraphicsTextItem* graphicsTextItem = new QGraphicsTextItem(labels[row].first);

        if (row == numMetas - 1) {
            measureWidth = graphicsTextItem->boundingRect().width();
        }
        maxWidth = std::max(maxWidth, int(graphicsTextItem->boundingRect().width()));

        QFontMetrics f(QApplication::font());
        QString partName = f.elidedText(labels[row].first, Qt::ElideRight, width());
        graphicsTextItem->setPlainText(partName);
        graphicsTextItem->setX(0);
        graphicsTextItem->setY(ypos);
        if (labels[row].second) {
            graphicsTextItem->setDefaultTextColor(_timeline->activeTheme().labelsColor1);
        } else {
            graphicsTextItem->setDefaultTextColor(_timeline->activeTheme().labelsColor2);
        }
        graphicsRectItem->setPen(QPen(_timeline->activeTheme().labelsColor2));
        graphicsRectItem->setBrush(QBrush(_timeline->activeTheme().labelsColor3));
        graphicsTextItem->setZValue(-1);
        graphicsRectItem->setZValue(-1);

        graphicsRectItem->setData(0, QVariant::fromValue<bool>(false));
        graphicsTextItem->setData(0, QVariant::fromValue<bool>(false));

        MouseOverValue mouseOverArrow = MouseOverValue::NONE;
        if (numMetas - 1 == row && (numMetas > 2 || _timeline->collapsed())) {
            // Measures meta
            if (_timeline->collapsed()) {
                mouseOverArrow = MouseOverValue::COLLAPSE_DOWN_ARROW;
            } else {
                mouseOverArrow = MouseOverValue::COLLAPSE_UP_ARROW;
            }
        } else if (row < numMetas - 1) {
            if (row != 0 && row + 1 <= numMetas - 2) {
                mouseOverArrow = MouseOverValue::MOVE_UP_DOWN_ARROW;
            } else if (row == 0 && row + 1 < numMetas - 1) {
                mouseOverArrow = MouseOverValue::MOVE_DOWN_ARROW;
            } else if (row == numMetas - 2 && row != 0) {
                mouseOverArrow = MouseOverValue::MOVE_UP_ARROW;
            }
        } else if (numMetas <= row) {
            if (_timeline->numToStaff(row - numMetas)
                && _timeline->numToStaff(row - numMetas)->show()) {
                mouseOverArrow = MouseOverValue::OPEN_EYE;
            } else {
                mouseOverArrow = MouseOverValue::CLOSED_EYE;
            }
        }
        graphicsTextItem->setData(1, QVariant::fromValue<MouseOverValue>(mouseOverArrow));
        graphicsRectItem->setData(1, QVariant::fromValue<MouseOverValue>(mouseOverArrow));

        graphicsTextItem->setData(2, QVariant::fromValue<unsigned>(row));
        graphicsRectItem->setData(2, QVariant::fromValue<unsigned>(row));

        scene()->addItem(graphicsRectItem);
        scene()->addItem(graphicsTextItem);
        if (row < numMetas) {
            std::pair<QGraphicsItem*, int> p1 = std::make_pair(graphicsRectItem, row);
            std::pair<QGraphicsItem*, int> p2 = std::make_pair(graphicsTextItem, row);
            _metaLabels.push_back(p1);
            _metaLabels.push_back(p2);
            graphicsRectItem->setZValue(1);
            graphicsTextItem->setZValue(2);
        }
    }
    QGraphicsLineItem* graphicsLineItem = new QGraphicsLineItem(0,
                                                                height * numMetas + verticalScrollBar()->value() + 1,
                                                                std::max(maxWidth + 20, 70),
                                                                height * numMetas + verticalScrollBar()->value() + 1);
    graphicsLineItem->setPen(QPen(QColor(150, 150, 150), 4));
    graphicsLineItem->setZValue(0);
    graphicsLineItem->setData(0, QVariant::fromValue<bool>(false));
    scene()->addItem(graphicsLineItem);

    std::pair<QGraphicsItem*, int> graphicsLineItemPair = std::make_pair(graphicsLineItem, numMetas);
    _metaLabels.push_back(graphicsLineItemPair);

    setSceneRect(0, 0, maxWidth, _timeline->getHeight() + _timeline->horizontalScrollBar()->height());

    std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> tmp(nullptr, MouseOverValue::NONE, -1);
    _oldItemInfo = tmp;

    setMinimumWidth(measureWidth + 9);
    setMaximumWidth(std::max(maxWidth + 20, 70));
    mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));
}

//---------------------------------------------------------
//   TRowLabels::resizeEvent
//---------------------------------------------------------

void TRowLabels::resizeEvent(QResizeEvent*)
{
    std::vector<std::pair<QString, bool> > labels = _timeline->getLabels();
    updateLabels(labels, 20);
}

//---------------------------------------------------------
//   TRowLabels::mousePressEvent
//---------------------------------------------------------

void TRowLabels::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }

    TRACEFUNC;

    QPointF scenePt = mapToScene(event->pos());
    unsigned numMetas = _timeline->nmetas();

    // Check if mouse position in scene is on the last meta
    QPointF measureMetaTl = QPointF(0, (numMetas - 1) * 20 + verticalScrollBar()->value());
    QPointF measureMetaBr = QPointF(width(), numMetas * 20 + verticalScrollBar()->value());
    if (QRectF(measureMetaTl, measureMetaBr).contains(scenePt) && (numMetas > 2 || _timeline->collapsed())) {
        if (std::get<0>(_oldItemInfo)) {
            std::pair<QGraphicsItem*, int> p = std::make_pair(std::get<0>(_oldItemInfo), std::get<2>(_oldItemInfo));
            std::vector<std::pair<QGraphicsItem*, int> >::iterator it = std::find(_metaLabels.begin(), _metaLabels.end(), p);
            if (it != _metaLabels.end()) {
                _metaLabels.erase(it);
            }
            scene()->removeItem(std::get<0>(_oldItemInfo));
        }
        std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> tmp(nullptr, MouseOverValue::NONE, -1);
        _oldItemInfo = tmp;
        mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));

        _timeline->setCollapsed(!_timeline->collapsed());
        _timeline->updateGridView();
    } else {
        // Check if pixmap was selected
        if (QGraphicsItem* graphicsItem = scene()->itemAt(scenePt, transform())) {
            QGraphicsPixmapItem* graphicsPixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphicsItem);
            if (graphicsPixmapItem) {
                unsigned row = graphicsPixmapItem->data(2).value<unsigned>();
                if (row == numMetas - 1) {
                    return;
                } else if (row < numMetas - 1) {
                    // Find mid point between up and down arrow
                    qreal midPoint = graphicsPixmapItem->boundingRect().height() / 2 + graphicsPixmapItem->scenePos().y();
                    if (scenePt.y() > midPoint) {
                        emit swapMeta(row, false);
                    } else {
                        emit swapMeta(row, true);
                    }
                } else if (row >= numMetas) {
                    _timeline->toggleShow(row - numMetas);
                }
            } else {
                _dragging = true;
                setCursor(Qt::SizeAllCursor);
                _oldLoc = QPoint(int(scenePt.x()), int(scenePt.y()));
            }
        } else {
            _dragging = true;
            setCursor(Qt::SizeAllCursor);
            _oldLoc = QPoint(int(scenePt.x()), int(scenePt.y()));
        }
    }
}

//---------------------------------------------------------
//   TRowLabels::mouseMoveEvent
//---------------------------------------------------------

void TRowLabels::mouseMoveEvent(QMouseEvent* event)
{
    QPointF scenePt = mapToScene(event->pos());
    if (_dragging) {
        setCursor(Qt::SizeAllCursor);
        int yOffset = int(_oldLoc.y()) - int(scenePt.y());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + yOffset);
    } else {
        mouseOver(scenePt);
    }
    emit moved(QPointF(-1, -1));
}

//---------------------------------------------------------
//   TRowLabels::mouseReleaseEvent
//---------------------------------------------------------

void TRowLabels::mouseReleaseEvent(QMouseEvent* event)
{
    if (QGraphicsItem* graphicsItem = scene()->itemAt(mapToScene(event->pos()), transform())) {
        QGraphicsPixmapItem* graphicsPixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphicsItem);
        if (graphicsPixmapItem) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
    _dragging = false;
}

//---------------------------------------------------------
//   TRowLabels::leaveEvent
//---------------------------------------------------------

void TRowLabels::leaveEvent(QEvent*)
{
    if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
        mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));
    }
}

//---------------------------------------------------------
//   TRowLabels::contextMenuEvent
//---------------------------------------------------------

void TRowLabels::contextMenuEvent(QContextMenuEvent* event)
{
    emit requestContextMenu(event);
}

//---------------------------------------------------------
//   TRowLabels::mouseOver
//---------------------------------------------------------

void TRowLabels::mouseOver(QPointF scenePt)
{
    TRACEFUNC;

    // Handle drawing of arrows
    if (QGraphicsItem* graphicsItem = scene()->itemAt(scenePt, transform())) {
        QGraphicsPixmapItem* graphicsPixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphicsItem);
        if (graphicsPixmapItem) {
            setCursor(Qt::PointingHandCursor);
            return;
        }

        MouseOverValue mouseOverArrow = graphicsItem->data(1).value<MouseOverValue>();
        if (mouseOverArrow != MouseOverValue::NONE) {
            QPixmap* pixmapArrow = _mouseoverMap[mouseOverArrow];
            QGraphicsPixmapItem* graphicsPixmapItemArrow = new QGraphicsPixmapItem(*pixmapArrow);
            unsigned row = graphicsItem->data(2).value<unsigned>();

            QString tooltip;
            switch (mouseOverArrow) {
            case MouseOverValue::COLLAPSE_DOWN_ARROW:
                tooltip = muse::qtrc("notation/timeline", "Expand meta rows");
                break;
            case MouseOverValue::COLLAPSE_UP_ARROW:
                tooltip = muse::qtrc("notation/timeline", "Collapse meta rows");
                break;
            case MouseOverValue::MOVE_DOWN_ARROW:
                tooltip = muse::qtrc("notation/timeline", "Move meta row down one");
                break;
            case MouseOverValue::MOVE_UP_ARROW:
                tooltip = muse::qtrc("notation/timeline", "Move meta row up one");
                break;
            case MouseOverValue::MOVE_UP_DOWN_ARROW:
                tooltip = muse::qtrc("notation/timeline", "Move meta row up/down one");
                break;
            case MouseOverValue::OPEN_EYE:
                tooltip = muse::qtrc("notation/timeline", "Hide instrument in score");
                break;
            case MouseOverValue::CLOSED_EYE:
                tooltip = muse::qtrc("notation/timeline", "Show instrument in score");
                break;
            default:
                tooltip = "";
                break;
            }

            graphicsPixmapItemArrow->setToolTip(tooltip);
            if (mouseOverArrow == MouseOverValue::OPEN_EYE || mouseOverArrow == MouseOverValue::CLOSED_EYE) {
                graphicsPixmapItemArrow->setData(0, QVariant::fromValue<bool>(false));
            } else {
                graphicsPixmapItemArrow->setData(0, QVariant::fromValue<bool>(true));
            }
            graphicsPixmapItemArrow->setData(1, QVariant::fromValue<MouseOverValue>(mouseOverArrow));
            graphicsPixmapItemArrow->setData(2, QVariant::fromValue<unsigned>(row));

            // Draw arrow at correct location
            if (row < _timeline->nmetas()) {
                graphicsPixmapItemArrow->setPos(width() - 12, verticalScrollBar()->value() + 1 + row * 20);
                graphicsPixmapItemArrow->setZValue(3);
            } else {
                graphicsPixmapItemArrow->setPos(width() - 13, row * 20 + 5);
                graphicsPixmapItemArrow->setZValue(-1);
            }

            if (std::get<2>(_oldItemInfo) == row && std::get<1>(_oldItemInfo) == mouseOverArrow) {
                // DO NOTHING
            } else {
                if (std::get<0>(_oldItemInfo)) {
                    std::pair<QGraphicsItem*, int> p = std::make_pair(std::get<0>(_oldItemInfo), std::get<2>(_oldItemInfo));
                    std::vector<std::pair<QGraphicsItem*, int> >::iterator it = std::find(_metaLabels.begin(), _metaLabels.end(), p);
                    if (it != _metaLabels.end()) {
                        _metaLabels.erase(it);
                    }
                    scene()->removeItem(std::get<0>(_oldItemInfo));
                }
                std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> tmp(graphicsPixmapItemArrow, mouseOverArrow, row);
                _oldItemInfo = tmp;
                if (mouseOverArrow != MouseOverValue::OPEN_EYE && mouseOverArrow != MouseOverValue::CLOSED_EYE) {
                    std::pair<QGraphicsItem*, int> p = std::make_pair(graphicsPixmapItemArrow, row);
                    _metaLabels.push_back(p);
                }
                scene()->addItem(graphicsPixmapItemArrow);
            }
        } else {
            if (std::get<0>(_oldItemInfo)) {
                scene()->removeItem(std::get<0>(_oldItemInfo));
            }
            std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> tmp(nullptr, MouseOverValue::NONE, -1);
            _oldItemInfo = tmp;
        }
    } else {
        if (std::get<0>(_oldItemInfo)) {
            std::pair<QGraphicsItem*, int> p = std::make_pair(std::get<0>(_oldItemInfo), std::get<2>(_oldItemInfo));
            std::vector<std::pair<QGraphicsItem*, int> >::iterator it = std::find(_metaLabels.begin(), _metaLabels.end(), p);
            if (it != _metaLabels.end()) {
                _metaLabels.erase(it);
            }
            scene()->removeItem(std::get<0>(_oldItemInfo));
        }
        std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> tmp(nullptr, MouseOverValue::NONE, -1);
        _oldItemInfo = tmp;
    }
    if (QGraphicsItem* graphicsItem = scene()->itemAt(scenePt, transform())) {
        QGraphicsPixmapItem* graphicsPixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphicsItem);
        if (graphicsPixmapItem) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

//---------------------------------------------------------
//   TRiwLabels::cursorIsOn
//---------------------------------------------------------

QString TRowLabels::cursorIsOn()
{
    QPointF scenePos = mapToScene(mapFromGlobal(QCursor::pos()));
    QGraphicsItem* graphicsItem = scene()->itemAt(scenePos, transform());
    if (graphicsItem) {
        auto it = _metaLabels.begin();
        for (; it != _metaLabels.end(); ++it) {
            if ((*it).first == graphicsItem) {
                break;
            }
        }
        if (it != _metaLabels.end()) {
            return "meta";
        } else {
            return "instrument";
        }
    } else {
        return "";
    }
}

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

Timeline::Timeline(QSplitter* splitter)
    : QGraphicsView(splitter), muse::Injectable(muse::iocCtxForQWidget(this))
{
    TRACEFUNC;

    setFocusPolicy(Qt::NoFocus);
    setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));
    setAttribute(Qt::WA_OpaquePaintEvent);
    setObjectName("Timeline");

    // theming
    _lightTheme.backgroundColor      = QColor(Qt::lightGray);
    _lightTheme.labelsColor1         = QColor(Qt::black);
    _lightTheme.labelsColor2         = QColor(150, 150, 150);
    _lightTheme.labelsColor3         = QColor(211, 211, 211);
    _lightTheme.gridColor1           = QColor(150, 150, 150);
    _lightTheme.gridColor2           = QColor(211, 211, 211);
    _lightTheme.measureMetaColor     = QColor(0, 0, 0);
    _lightTheme.selectionColor       = QColor(173, 216, 230);
    _lightTheme.nonVisiblePenColor   = QColor(100, 150, 250);
    _lightTheme.nonVisibleBrushColor = QColor(192, 192, 192, 180);
    _lightTheme.colorBoxColor        = QColor(Qt::gray);
    _lightTheme.metaValuePenColor    = QColor(Qt::black);
    _lightTheme.metaValueBrushColor  = QColor(Qt::gray);

    _darkTheme.backgroundColor       = QColor(35, 35, 35);
    _darkTheme.labelsColor1          = QColor(225, 225, 225);
    _darkTheme.labelsColor2          = QColor(55, 55, 55);
    _darkTheme.labelsColor3          = QColor(70, 70, 70);
    _darkTheme.gridColor1            = QColor(50, 50, 50);
    _darkTheme.gridColor2            = QColor(75, 75, 75);
    _darkTheme.measureMetaColor      = QColor(200, 200, 200);
    _darkTheme.selectionColor        = QColor(55, 70, 75);
    _darkTheme.nonVisiblePenColor    = QColor(40, 60, 80);
    _darkTheme.nonVisibleBrushColor  = QColor(55, 55, 55, 180);
    _darkTheme.colorBoxColor         = QColor(Qt::gray);
    _darkTheme.metaValuePenColor     = QColor(Qt::lightGray);
    _darkTheme.metaValueBrushColor   = QColor(Qt::darkGray);

    _splitter = splitter;
    _rowNames = new TRowLabels(splitter, this);
    _splitter->addWidget(_rowNames);
    _splitter->addWidget(this);
    _splitter->setChildrenCollapsible(false);
    _splitter->setStretchFactor(0, 0);
    _splitter->setStretchFactor(1, 0);

    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    setScene(new QGraphicsScene);
    setSceneRect(0, 0, 100, 100);
    scene()->setBackgroundBrush(QBrush(activeTheme().backgroundColor));

    connect(verticalScrollBar(), &QScrollBar::valueChanged, _rowNames->verticalScrollBar(), &QScrollBar::setValue);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &Timeline::handleScroll);
    connect(_rowNames, &TRowLabels::swapMeta, this, &Timeline::swapMeta);
    connect(this, &Timeline::moved, _rowNames, &TRowLabels::mouseOver);

    _metas.push_back({ muse::qtrc("notation/timeline", "Tempo"), &Timeline::tempoMeta, true });
    _metas.push_back({ muse::qtrc("notation/timeline", "Time signature"), &Timeline::timeMeta, true });
    _metas.push_back({ muse::qtrc("notation/timeline", "Rehearsal mark"), &Timeline::rehearsalMeta, true });
    _metas.push_back({ muse::qtrc("notation/timeline", "Key signature"), &Timeline::keyMeta, true });
    _metas.push_back({ muse::qtrc("notation/timeline", "Barlines"), &Timeline::barlineMeta, true });
    _metas.push_back({ muse::qtrc("notation/timeline", "Jumps and markers"), &Timeline::jumpMarkerMeta, true });
    _metas.push_back({ muse::qtrc("notation/timeline", "Measures"), &Timeline::measureMeta, true });

    std::tuple<QGraphicsItem*, int, QColor> ohi(nullptr, -1, QColor());
    _oldHoverInfo = ohi;
    std::tuple<int, qreal, EngravingItem*, EngravingItem*, bool> ri(0, 0, nullptr, nullptr, false);
    _repeatInfo = ri;

    static const char* startRepeat[] = {
        "7 14 2 1",
        "# c #000000",
        ". c None",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#.##",
        "##.#.##",
        "##.#...",
        "##.#...",
        "##.#.##",
        "##.#.##",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#..."
    };

    static const char* endRepeat[] = {
        "7 14 2 1",
        "# c #000000",
        ". c None",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "##.#.##",
        "##.#.##",
        "...#.##",
        "...#.##",
        "##.#.##",
        "##.#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##"
    };

    static const char* endBarline[] = {
        "7 14 2 1",
        "# c #000000",
        ". c None",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##",
        "...#.##"
    };

    static const char* doubleBarline[] = {
        "7 14 2 1",
        "# c #000000",
        ". c None",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#..",
        "..#.#.."
    };

    static const char* reverseEndBarline[] = {
        "7 14 2 1",
        "# c #000000",
        ". c None",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#...",
        "##.#..."
    };

    static const char* heavyBarline[] = {
        "6 14 2 1",
        "# c #000000",
        ". c None",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##..",
        "..##.."
    };

    static const char* doubleHeavyBarline[] = {
        "7 14 2 1",
        "# c #000000",
        ". c None",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##.",
        ".##.##."
    };

    QPixmap* startRepeatPixmap = new QPixmap(startRepeat);
    QPixmap* endRepeatPixmap = new QPixmap(endRepeat);
    QPixmap* endBarlinePixmap = new QPixmap(endBarline);
    QPixmap* doubleBarlinePixmap = new QPixmap(doubleBarline);
    QPixmap* reverseEndBarlinePixmap = new QPixmap(reverseEndBarline);
    QPixmap* heavyBarlinePixmap = new QPixmap(heavyBarline);
    QPixmap* doubleHeavyBarlinePixmap = new QPixmap(doubleHeavyBarline);

    _barlines[BarLineType::START_REPEAT] = startRepeatPixmap;
    _barlines[BarLineType::END_REPEAT] = endRepeatPixmap;
    _barlines[BarLineType::END] = endBarlinePixmap;
    _barlines[BarLineType::DOUBLE] = doubleBarlinePixmap;
    _barlines[BarLineType::REVERSE_END] = reverseEndBarlinePixmap;
    _barlines[BarLineType::HEAVY] = heavyBarlinePixmap;
    _barlines[BarLineType::DOUBLE_HEAVY] = doubleHeavyBarlinePixmap;

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        updateTimelineTheme();
    });
}

bool Timeline::handleEvent(QEvent* e)
{
    return QWidget::event(e);
}

//---------------------------------------------------------
//   Timeline::drawGrid
//---------------------------------------------------------

void Timeline::drawGrid(int globalRows, int globalCols, int startMeasure, int endMeasure)
{
    TRACEFUNC;

    if (endMeasure < 0) {
        endMeasure = globalCols;
    }
    if (startMeasure < 0) {
        endMeasure = startMeasure;
    }

    const bool rebuildAll = (
        gridRows != globalRows || gridCols != globalCols
        || (startMeasure == 0 && 2 * (endMeasure - startMeasure) > globalCols)  // rebuild all if more than half of score has changed
        );
    const bool rebuildPartial = !rebuildAll && (startMeasure >= 0);

    const unsigned numMetas = nmetas();

    if (rebuildAll) {
        clearScene();
        startMeasure = 0;
        endMeasure = globalCols;
    } else {
        if (rebuildPartial) {
            const QRectF replacedRect
                = getMeasureRect(startMeasure, 0, numMetas) | getMeasureRect(endMeasure - 1, globalRows - 1, numMetas);
            const QList<QGraphicsItem*> replacedItems = scene()->items(replacedRect, Qt::ContainsItemShape);
            for (QGraphicsItem* item : replacedItems) {
                if (item->data(keyItemType).value<ItemType>() != ItemType::TYPE_MEASURE) {
                    continue;
                }
                scene()->removeItem(item);
                delete item;
            }
        }

        // Meta rows are still rebuilt from scratch, remove old meta rows manually
        const QList<QGraphicsItem*> items = scene()->items();
        for (QGraphicsItem* item : items) {
            if (item->data(keyItemType).value<ItemType>() != ItemType::TYPE_META) {
                continue;
            }
            scene()->removeItem(item);
            delete item;
        }
    }

    _metaRows.clear();

    if (globalRows == 0 || globalCols == 0) {
        return;
    }

    int stagger = 0;
    setMinimumHeight(_gridHeight * (numMetas + 1) + 5 + horizontalScrollBar()->height());
    setMinimumWidth(_gridWidth * 3);
    _globalZValue = 1;

    // Draw grid
    Measure* currMeasure = score()->firstMeasure();
    for (int i = 0; i < startMeasure; ++i) {
        currMeasure = currMeasure->nextMeasure();
    }

    QList<Part*> partList = getParts();

    for (int col = startMeasure; col < endMeasure; col++) {
        for (int row = 0; row < globalRows; row++) {
            QGraphicsRectItem* graphicsRectItem = new QGraphicsRectItem(getMeasureRect(col, row, numMetas));
            graphicsRectItem->setData(keyItemType, QVariant::fromValue(ItemType::TYPE_MEASURE));

            setMetaData(graphicsRectItem, row, ElementType::INVALID, currMeasure, false, 0);

            QString translateMeasure = muse::qtrc("notation/timeline", "Measure");
            QChar initialLetter = translateMeasure[0];
            QTextDocument doc;
            QString partName = "";
            if (partList.size() > row) {
                doc.setHtml(partList.at(row)->longName());
                partName = doc.toPlainText();
                if (partName.isEmpty()) {         // No Long instrument name? Fall back to Part name
                    doc.setHtml(partList.at(row)->partName());
                    partName = doc.toPlainText();
                }
                if (partName.isEmpty()) {       // No Part name? Fall back to Instrument name
                    partName = partList.at(row)->instrumentName();
                }
            }

            graphicsRectItem->setToolTip(initialLetter + QString(" ") + QString::number(currMeasure->no() + 1) + QString(", ") + partName);
            graphicsRectItem->setPen(QPen(activeTheme().backgroundColor));
            graphicsRectItem->setBrush(QBrush(colorBox(graphicsRectItem)));
            graphicsRectItem->setZValue(-3);
            scene()->addItem(graphicsRectItem);
        }

        currMeasure = currMeasure->nextMeasure();
    }
    setSceneRect(0, 0, getWidth(), getHeight());

    // Draw meta rows and separator
    QGraphicsLineItem* graphicsLineItemSeparator = new QGraphicsLineItem(0,
                                                                         _gridHeight * numMetas + verticalScrollBar()->value() + 1,
                                                                         getWidth() - 1,
                                                                         _gridHeight * numMetas + verticalScrollBar()->value() + 1);
    graphicsLineItemSeparator->setData(keyItemType, QVariant::fromValue(ItemType::TYPE_META));
    graphicsLineItemSeparator->setPen(QPen(activeTheme().gridColor1, 4));
    graphicsLineItemSeparator->setZValue(-2);
    scene()->addItem(graphicsLineItemSeparator);
    std::pair<QGraphicsItem*, int> pairGraphicsIntSeparator(graphicsLineItemSeparator, numMetas);
    _metaRows.push_back(pairGraphicsIntSeparator);

    for (unsigned row = 0; row < numMetas; row++) {
        QGraphicsRectItem* metaRow = new QGraphicsRectItem(0,
                                                           _gridHeight * row + verticalScrollBar()->value(),
                                                           getWidth(),
                                                           _gridHeight);
        metaRow->setData(keyItemType, QVariant::fromValue(ItemType::TYPE_META));
        metaRow->setBrush(QBrush(activeTheme().gridColor2));
        metaRow->setPen(QPen(activeTheme().gridColor1));
        metaRow->setData(0, QVariant::fromValue<int>(-1));

        scene()->addItem(metaRow);

        std::pair<QGraphicsItem*, int> pairGraphicsIntMeta(metaRow, row);
        _metaRows.push_back(pairGraphicsIntMeta);
    }

    int xPos = 0;

    // Create stagger array if _collapsedMeta is false
    std::vector<int> staggerArr(numMetas, 0);    // Default initialized, loop not required

    bool noKey = true;
    std::get<4>(_repeatInfo) = false;

    for (Measure* cm = score()->firstMeasure(); cm; cm = cm->nextMeasure()) {
        for (Segment* currSeg = cm->first(); currSeg; currSeg = currSeg->next()) {
            // Toggle noKey if initial key signature is found
            if (currSeg->isKeySigType() && cm == score()->firstMeasure()) {
                if (noKey && currSeg->tick().isZero()) {
                    noKey = false;
                }
            }

            // If no initial key signature is found, add key signature
            if (cm == score()->firstMeasure() && noKey
                && (currSeg->isTimeSigType() || currSeg->isChordRestType())) {
                if (getMetaRow(muse::qtrc("notation/timeline", "Key signature")) != numMetas) {
                    if (_collapsedMeta) {
                        keyMeta(0, &stagger, xPos);
                    } else {
                        keyMeta(0, &staggerArr[getMetaRow(muse::qtrc("notation/timeline", "Key signature"))], xPos);
                    }
                }
                noKey = false;
            }
            int row = 0;
            for (auto it = _metas.begin(); it != _metas.end(); ++it) {
                std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
                if (!std::get<2>(meta)) {
                    continue;
                }
                void (Timeline::* func)(Segment*, int*, int) = std::get<1>(meta);
                if (_collapsedMeta) {
                    (this->*func)(currSeg, &stagger, xPos);
                } else {
                    (this->*func)(currSeg, &staggerArr[row], xPos);
                }
                row++;
            }
        }
        // Handle all jumps here
        if (getMetaRow(muse::qtrc("notation/timeline", "Jumps and markers")) != numMetas) {
            ElementList measureElementsList = cm->el();
            for (EngravingItem* element : measureElementsList) {
                std::get<3>(_repeatInfo) = element;
                if (element->isMarker()) {
                    jumpMarkerMeta(0, &stagger, xPos);
                }
            }
            for (EngravingItem* element : measureElementsList) {
                if (element->isJump()) {
                    std::get<2>(_repeatInfo) = element;
                    if (_collapsedMeta) {
                        jumpMarkerMeta(0, &stagger, xPos);
                    } else {
                        jumpMarkerMeta(0, &std::get<0>(_repeatInfo), xPos);
                    }
                }
            }
        }
        stagger = 0;
        std::get<0>(_repeatInfo) = 0;

        for (unsigned row = 0; row < numMetas; row++) {
            staggerArr[row] = 0;
        }
        xPos += _gridWidth;
        std::get<4>(_repeatInfo) = false;
    }

    gridRows = globalRows;
    gridCols = globalCols;
}

//---------------------------------------------------------
//   Timeline::tempoMeta
//---------------------------------------------------------

void Timeline::tempoMeta(Segment* seg, int* stagger, int pos)
{
    // Find position of tempoMeta in metas
    int row = getMetaRow(muse::qtrc("notation/timeline", "Tempo"));

    // Add all tempo texts in this segment
    const std::vector<EngravingItem*> annotations = seg->annotations();
    for (EngravingItem* element : annotations) {
        if (element->isTempoText()) {
            TempoText* text = toTempoText(element);
            qreal x = pos + (*stagger) * _spacing;
            if (addMetaValue(x, pos, text->plainText(), row, ElementType::TEMPO_TEXT, element, 0, seg->measure())) {
                (*stagger)++;
                _globalZValue++;
            }
        }
    }
}

//---------------------------------------------------------
//   Timeline::timeMeta
//---------------------------------------------------------

void Timeline::timeMeta(Segment* seg, int* stagger, int pos)
{
    if (!seg->isTimeSigType()) {
        return;
    }

    TRACEFUNC;

    int x = pos + (*stagger) * _spacing;

    // Find position of timeMeta in metas
    int row = getMetaRow(muse::qtrc("notation/timeline", "Time signature"));

    TimeSig* originalTimeSig = toTimeSig(seg->element(0));
    if (!originalTimeSig) {
        return;
    }

    // Check if same across all staves
    const size_t nrows = score()->staves().size();
    bool same = true;
    for (size_t track = 0; track < nrows; track++) {
        const TimeSig* currTimeSig = toTimeSig(seg->element(track * VOICES));
        if (!currTimeSig) {
            same = false;
            break;
        }
        if (*currTimeSig == *originalTimeSig) {
            continue;
        }
        same = false;
        break;
    }
    if (!same) {
        return;
    }
    QString text = QString::number(originalTimeSig->numerator()) + QString("/") + QString::number(originalTimeSig->denominator());

    if (addMetaValue(x, pos, text, row, ElementType::TIMESIG, 0, seg, seg->measure())) {
        (*stagger)++;
        _globalZValue++;
    }
}

//---------------------------------------------------------
//   Timeline::rehearsalMeta
//---------------------------------------------------------

void Timeline::rehearsalMeta(Segment* seg, int* stagger, int pos)
{
    int row = getMetaRow(muse::qtrc("notation/timeline", "Rehearsal mark"));

    for (EngravingItem* element : seg->annotations()) {
        int x = pos + (*stagger) * _spacing;
        if (element->isRehearsalMark()) {
            RehearsalMark* rehearsal_mark = toRehearsalMark(element);
            if (!rehearsal_mark) {
                continue;
            }

            if (addMetaValue(x, pos, rehearsal_mark->plainText(), row, ElementType::REHEARSAL_MARK, element, 0, seg->measure())) {
                (*stagger)++;
                _globalZValue++;
            }
        }
    }
}

//---------------------------------------------------------
//   Timeline::keyMeta
//---------------------------------------------------------

void Timeline::keyMeta(Segment* seg, int* stagger, int pos)
{
    // If seg is nullptr, handle initial key signature
    if (seg && !seg->isKeySigType()) {
        return;
    }

    TRACEFUNC;

    int row = getMetaRow(muse::qtrc("notation/timeline", "Key signature"));
    std::map<Key, int> keyFrequencies;
    const std::vector<Staff*>& staves = score()->staves();

    int track = 0;
    for (Staff* stave : staves) {
        if (!stave->show()) {
            track += VOICES;
            continue;
        }

        // Ignore unpitched staves
        if ((seg && !stave->isPitchedStaff(seg->tick())) || (!seg && !stave->isPitchedStaff(Fraction(0, 1)))) {
            track += VOICES;
            continue;
        }

        // Add corrected key signature to map
        // Atonal -> Key::INVALID
        // Custom -> Key::NUM_OF
        const KeySig* currKeySig = nullptr;
        if (seg) {
            currKeySig = toKeySig(seg->element(track));
        }

        Key globalKey;
        if (seg) {
            globalKey = stave->concertKey(seg->tick());
        } else {
            globalKey = stave->concertKey(Fraction(0, 1));
        }
        if (currKeySig) {
            if (currKeySig->generated()) {
                return;
            }
            globalKey = currKeySig->concertKey();
        }

        if (currKeySig && currKeySig->isAtonal()) {
            globalKey = Key::INVALID;
        } else if (currKeySig && currKeySig->isCustom()) {
            globalKey = Key::NUM_OF;
        }

        std::map<Key, int>::iterator it = keyFrequencies.find(globalKey);
        if (it != keyFrequencies.end()) {
            keyFrequencies[globalKey]++;
        } else {
            keyFrequencies[globalKey] = 1;
        }

        track += VOICES;
    }

    // Change key into QString
    Key newKey = Key::C;
    int maxKeyFreq = 0;
    for (std::map<Key, int>::iterator iter = keyFrequencies.begin(); iter != keyFrequencies.end(); ++iter) {
        if (iter->second > maxKeyFreq) {
            newKey = iter->first;
            maxKeyFreq = iter->second;
        }
    }
    QString keyText;
    QString tooltip;
    if (newKey == Key::INVALID) {
        keyText = "X";
        tooltip = TConv::translatedUserName(Key::INVALID, true);
    } else if (newKey == Key::NUM_OF) {
        keyText = "?";
        tooltip = muse::qtrc("notation/timeline", "Custom key signature");
    } else if (int(newKey) == 0) {
        keyText = "\u266E";
        tooltip = TConv::translatedUserName(Key::C);
    } else if (int(newKey) < 0) {
        keyText = QString::number(std::abs(int(newKey))) + "\u266D";
        tooltip = TConv::translatedUserName(newKey);
    } else {
        keyText = QString::number(std::abs(int(newKey))) + "\u266F";
        tooltip = TConv::translatedUserName(newKey);
    }

    int x = pos + (*stagger) * _spacing;
    Measure* measure = seg ? seg->measure() : 0;
    if (addMetaValue(x, pos, keyText, row, ElementType::KEYSIG, 0, seg, measure, tooltip)) {
        (*stagger)++;
        _globalZValue++;
    }
}

//---------------------------------------------------------
//   Timeline::barLineMeta
//---------------------------------------------------------

void Timeline::barlineMeta(Segment* seg, int* stagger, int pos)
{
    if (!seg->isBeginBarLineType() && !seg->isEndBarLineType() && !seg->isBarLine() && !seg->isStartRepeatBarLineType()) {
        return;
    }

    TRACEFUNC;

    // Find position of repeat_meta in metas
    int row = getMetaRow(muse::qtrc("notation/timeline", "Barlines"));

    QString repeatText = "";
    BarLine* barline = toBarLine(seg->element(0));

    if (barline) {
        switch (barline->barLineType()) {
        case BarLineType::START_REPEAT:
        case BarLineType::END_REPEAT:
        case BarLineType::END:
        case BarLineType::DOUBLE:
        case BarLineType::REVERSE_END:
        case BarLineType::HEAVY:
        case BarLineType::DOUBLE_HEAVY:
            repeatText = BarLine::translatedUserTypeName(barline->barLineType());
            break;
        case BarLineType::END_START_REPEAT:
        // actually an end repeat followed by a start repeat, so nothing needs to be done here
        default:
            break;
        }
        _isBarline = true;
    } else {
        return;
    }

    Measure* measure = seg->measure();
    ElementType elementType = ElementType::BAR_LINE;
    EngravingItem* element = nullptr;

    if (repeatText == "") {
        _isBarline = false;
        return;
    }

    int x = pos + (*stagger) * _spacing;
    if (addMetaValue(x, pos, repeatText, row, elementType, element, seg, measure)) {
        (*stagger)++;
        _globalZValue++;
    }
    _isBarline = false;
}

//---------------------------------------------------------
//   Timeline::jumpMarkerMeta
//---------------------------------------------------------

void Timeline::jumpMarkerMeta(Segment* seg, int* stagger, int pos)
{
    if (seg) {
        return;
    }

    TRACEFUNC;

    // Find position of repeat_meta in metas
    int row = getMetaRow(muse::qtrc("notation/timeline", "Jumps and markers"));

    QString text = "";
    EngravingItem* element = nullptr;
    if (std::get<2>(_repeatInfo)) {
        element = std::get<2>(_repeatInfo);
    } else if (std::get<3>(_repeatInfo)) {
        element = std::get<3>(_repeatInfo);
    }

    Measure* measure;
    ElementType elementType;
    if (std::get<2>(_repeatInfo)) {
        Jump* jump = toJump(std::get<2>(_repeatInfo));
        text = jump->plainText();
        measure = jump->measure();
        elementType = ElementType::JUMP;
    } else {
        Marker* marker = toMarker(std::get<3>(_repeatInfo));
        std::list<TextFragment> tf_list = marker->fragmentList();
        for (TextFragment tf : tf_list) {
            text.push_back(tf.text);
        }
        measure = marker->measure();
        if (marker->markerType() == MarkerType::FINE
            || marker->markerType() == MarkerType::TOCODA
            || marker->markerType() == MarkerType::TOCODASYM
            || marker->markerType() == MarkerType::DA_CODA
            || marker->markerType() == MarkerType::DA_DBLCODA
            ) {
            elementType = ElementType::MARKER;
            std::get<2>(_repeatInfo) = std::get<3>(_repeatInfo);
            std::get<3>(_repeatInfo) = nullptr;
        } else {
            elementType = ElementType::MARKER;
        }
    }

    if (text == "") {
        std::get<2>(_repeatInfo) = nullptr;
        std::get<3>(_repeatInfo) = nullptr;
        return;
    }

    int x = pos + (*stagger) * _spacing;
    if (addMetaValue(x, pos, text, row, elementType, element, seg, measure)) {
        (*stagger)++;
        _globalZValue++;
    }
    std::get<2>(_repeatInfo) = nullptr;
    std::get<3>(_repeatInfo) = nullptr;
}

//---------------------------------------------------------
//   Timeline::measureMeta
//---------------------------------------------------------

void Timeline::measureMeta(Segment*, int*, int pos)
{
    TRACEFUNC;

    // Increment decided by zoom level
    int incrementValue = 1;
    int halfway = (_maxZoom + _minZoom) / 2;
    if (_gridWidth <= _maxZoom && _gridWidth > halfway) {
        incrementValue = 1;
    } else if (_gridWidth <= halfway && _gridWidth > _minZoom) {
        incrementValue = 5;
    } else {
        incrementValue = 10;
    }

    int currMeasureNumber = pos / _gridWidth;
    if (currMeasureNumber == _globalMeasureNumber) {
        return;
    }

    _globalMeasureNumber = currMeasureNumber;
    // Check if 1 or 5*n
    if (currMeasureNumber + 1 != 1 && (currMeasureNumber + 1) % incrementValue != 0) {
        return;
    }

    // Find position of measureMeta in metas
    int row = getMetaRow(muse::qtrc("notation/timeline", "Measures"));

    // Adjust number
    Measure* currMeasure;
    for (currMeasure = score()->firstMeasure(); currMeasureNumber != 0; currMeasureNumber--, currMeasure = currMeasure->nextMeasure()) {
    }

    // Add measure number
    QString measureNumber = (currMeasure->irregular()) ? "( )" : QString::number(currMeasure->no() + 1);
    QGraphicsTextItem* graphicsTextItem = new QGraphicsTextItem(measureNumber);
    graphicsTextItem->setData(keyItemType, QVariant::fromValue(ItemType::TYPE_META));
    graphicsTextItem->setDefaultTextColor(activeTheme().measureMetaColor);
    graphicsTextItem->setX(pos);
    graphicsTextItem->setY(_gridHeight * row + verticalScrollBar()->value());

    QFont f = graphicsTextItem->font();
    f.setPointSizeF(7.0);
    graphicsTextItem->setFont(f);

    // Center text
    qreal remainingWidth  = _gridWidth - graphicsTextItem->boundingRect().width();
    qreal remainingHeight = _gridHeight - graphicsTextItem->boundingRect().height();
    graphicsTextItem->setX(graphicsTextItem->x() + remainingWidth / 2);
    graphicsTextItem->setY(graphicsTextItem->y() + remainingHeight / 2);

    int endOfText = graphicsTextItem->x() + graphicsTextItem->boundingRect().width();
    int endOfGrid = getWidth();
    if (endOfText <= endOfGrid) {
        scene()->addItem(graphicsTextItem);

        std::pair<QGraphicsItem*, int> pairMeasureText = std::make_pair(graphicsTextItem, row);
        _metaRows.push_back(pairMeasureText);
    }
}

//---------------------------------------------------------
//   Timeline::getMetaRow
//---------------------------------------------------------

unsigned Timeline::getMetaRow(QString targetText)
{
    if (_collapsedMeta) {
        if (targetText == muse::qtrc("notation/timeline", "Measures")) {
            return 1;
        } else {
            return 0;
        }
    }
    int row = 0;
    for (auto it = _metas.begin(); it != _metas.end(); ++it) {
        std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
        QString metaText = std::get<0>(meta);
        bool visible = std::get<2>(meta);
        if (metaText == targetText && visible) {
            break;
        } else if (!visible) {
            continue;
        }
        row++;
    }
    return row;
}

//---------------------------------------------------------
//   Timeline::addMetaValue
//---------------------------------------------------------

bool Timeline::addMetaValue(int x, int pos, QString metaText, int row, ElementType elementType, EngravingItem* element, Segment* seg,
                            Measure* measure, QString tooltip)
{
    TRACEFUNC;

    QGraphicsTextItem* graphicsTextItem = new QGraphicsTextItem(metaText);
    qreal textWidth = graphicsTextItem->boundingRect().width();

    QGraphicsPixmapItem* graphicsPixmapItem = nullptr;

    std::map<QString, BarLineType> barLineTypes = {
        { BarLine::translatedUserTypeName(BarLineType::START_REPEAT), BarLineType::START_REPEAT },
        { BarLine::translatedUserTypeName(BarLineType::END_REPEAT), BarLineType::END_REPEAT },
        { BarLine::translatedUserTypeName(BarLineType::END), BarLineType::END },
        { BarLine::translatedUserTypeName(BarLineType::DOUBLE), BarLineType::DOUBLE },
        { BarLine::translatedUserTypeName(BarLineType::REVERSE_END), BarLineType::REVERSE_END },
        { BarLine::translatedUserTypeName(BarLineType::HEAVY), BarLineType::HEAVY },
        { BarLine::translatedUserTypeName(BarLineType::DOUBLE_HEAVY), BarLineType::DOUBLE_HEAVY },
    };

    BarLineType barLineType = barLineTypes[metaText];

    if (_isBarline) {
        graphicsPixmapItem = new QGraphicsPixmapItem(*_barlines[barLineType]);
    }

    if (graphicsPixmapItem) {
        textWidth = 10;
        if (textWidth > _gridWidth) {
            textWidth = _gridWidth;
            if (barLineType == BarLineType::END_REPEAT && std::get<4>(_repeatInfo)) {
                textWidth /= 2;
            }
        }
    }

    if (textWidth + x > getWidth()) {
        textWidth = getWidth() - x;
    }

    // Adjust x for end repeats
    if ((barLineType == BarLineType::END_REPEAT
         || barLineType == BarLineType::END
         || barLineType == BarLineType::DOUBLE
         || barLineType == BarLineType::REVERSE_END
         || barLineType == BarLineType::HEAVY
         || barLineType == BarLineType::DOUBLE_HEAVY
         || std::get<2>(_repeatInfo))
        && !_collapsedMeta) {
        if (std::get<0>(_repeatInfo) > 0) {
            x = pos + _gridWidth - std::get<1>(_repeatInfo) + std::get<0>(_repeatInfo) * _spacing;
        } else {
            x = pos + _gridWidth - textWidth;
            std::get<1>(_repeatInfo) = textWidth;
        }
        // Check if extending past left side
        if (x < 0) {
            textWidth = textWidth + x;
            x = 0;
        }
    }

    // Return if past width
    if (x >= getWidth()) {
        return false;
    }

    QGraphicsItem* itemToAdd;
    if (graphicsPixmapItem) {
        // Exact values required for repeat pixmap to work visually
        if (textWidth != 10) {
            graphicsPixmapItem = new QGraphicsPixmapItem();
        }
        if (barLineType == BarLineType::START_REPEAT) {
            std::get<4>(_repeatInfo) = true;
        }
        graphicsPixmapItem->setX(x + 2);
        graphicsPixmapItem->setY(_gridHeight * row + verticalScrollBar()->value() + 3);
        itemToAdd = graphicsPixmapItem;
    } else if (metaText == "\uE047" || metaText == "\uE048") {
        graphicsTextItem->setX(x);
        graphicsTextItem->setY(_gridHeight * row + verticalScrollBar()->value() - 2);
        itemToAdd = graphicsTextItem;
    } else if (row == 0) {
        graphicsTextItem->setX(x);
        graphicsTextItem->setY(_gridHeight * row + verticalScrollBar()->value() - 6);
        itemToAdd = graphicsTextItem;
    } else {
        graphicsTextItem->setX(x);
        graphicsTextItem->setY(_gridHeight * row + verticalScrollBar()->value() - 1);
        itemToAdd = graphicsTextItem;
    }

    QFontMetrics f(QApplication::font());
    QString partName = f.elidedText(graphicsTextItem->toPlainText(),
                                    Qt::ElideRight,
                                    textWidth);

    // Set tool tip if elided
    if (tooltip != "") {
        graphicsTextItem->setToolTip(tooltip);
    } else if (partName != metaText) {
        graphicsTextItem->setToolTip(graphicsTextItem->toPlainText());
    }
    graphicsTextItem->setPlainText(partName);

    // Make text fit within rectangle
    while (graphicsTextItem->boundingRect().width() > textWidth
           && graphicsTextItem->toPlainText() != "") {
        QString text = graphicsTextItem->toPlainText();
        text.chop(1);
        graphicsTextItem->setPlainText(text);
    }

    QGraphicsRectItem* graphicsRectItem = new QGraphicsRectItem(x,
                                                                _gridHeight * row + verticalScrollBar()->value(),
                                                                textWidth,
                                                                _gridHeight);
    if (tooltip != "") {
        graphicsRectItem->setToolTip(tooltip);
    } else if (partName != metaText || graphicsPixmapItem) {
        graphicsRectItem->setToolTip(metaText);
    }

    setMetaData(graphicsRectItem, -1, elementType, measure, true, element, itemToAdd, seg);
    setMetaData(itemToAdd, -1, elementType, measure, true, element, graphicsRectItem, seg);

    graphicsRectItem->setData(keyItemType, QVariant::fromValue(ItemType::TYPE_META));
    itemToAdd->setData(keyItemType, QVariant::fromValue(ItemType::TYPE_META));

    graphicsRectItem->setZValue(_globalZValue);
    itemToAdd->setZValue(_globalZValue);

    graphicsRectItem->setPen(QPen(activeTheme().metaValuePenColor));
    graphicsRectItem->setBrush(QBrush(activeTheme().metaValueBrushColor));

    scene()->addItem(graphicsRectItem);
    scene()->addItem(itemToAdd);

    std::pair<QGraphicsItem*, int> pairTimeRect = std::make_pair(graphicsRectItem, row);
    std::pair<QGraphicsItem*, int> pairTimeText = std::make_pair(itemToAdd, row);
    _metaRows.push_back(pairTimeRect);
    _metaRows.push_back(pairTimeText);

    if (barLineType == BarLineType::END_REPEAT) {
        std::get<0>(_repeatInfo)++;
    }

    return true;
}

//---------------------------------------------------------
//   Timeline::setMetaData
//---------------------------------------------------------

void Timeline::setMetaData(QGraphicsItem* gi, int staff, ElementType et, Measure* m, bool full_measure, EngravingItem* e,
                           QGraphicsItem* pairItem,
                           Segment* seg)
{
    // full_measure true for meta values
    // pr is null for grid items, set for meta values
    // seg is set if key meta
    gi->setData(0, QVariant::fromValue<int>(staff));
    gi->setData(1, QVariant::fromValue<ElementType>(et));
    gi->setData(2, QVariant::fromValue<void*>(m));
    gi->setData(3, QVariant::fromValue<bool>(full_measure));
    gi->setData(4, QVariant::fromValue<void*>(e));
    gi->setData(5, QVariant::fromValue<void*>(pairItem));
    gi->setData(6, QVariant::fromValue<void*>(seg));
}

//---------------------------------------------------------
//   Timeline::getWidth
//---------------------------------------------------------

int Timeline::getWidth() const
{
    if (score()) {
        return static_cast<int>(score()->nmeasures()) * _gridWidth;
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   Timeline::getHeight
//---------------------------------------------------------

int Timeline::getHeight() const
{
    if (score()) {
        return (nstaves() + static_cast<int>(nmetas())) * _gridHeight + 3;
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   Timeline::correctStave
//---------------------------------------------------------

staff_idx_t Timeline::correctStave(staff_idx_t stave)
{
    // Find correct stave (skipping hidden staves)
    const std::vector<Staff*>& list = score()->staves();
    size_t count = 0;
    while (stave >= count) {
        if (count >= list.size()) {
            count = list.size() - 1;
            return count;
        }
        if (!list.at(count)->show()) {
            stave++;
        }
        count++;
    }
    return stave;
}

//---------------------------------------------------------
//   Timeline::correctPart
//---------------------------------------------------------

int Timeline::correctPart(staff_idx_t stave)
{
    // Find correct stave (skipping hidden staves)
    const std::vector<Staff*>& list = score()->staves();
    staff_idx_t count = correctStave(stave);
    return getParts().indexOf(list.at(count)->part());
}

//---------------------------------------------------------
//   Timeline::getParts
//---------------------------------------------------------

QList<Part*> Timeline::getParts()
{
    const std::vector<Part*>& realPartList = score()->parts();
    QList<Part*> partList;
    for (Part* p : realPartList) {
        for (size_t i = 0; i < p->nstaves(); i++) {
            partList.append(p);
        }
    }

    return partList;
}

//---------------------------------------------------------
//   clearScene
//---------------------------------------------------------

void Timeline::clearScene()
{
    scene()->clear();

    // clear pointers to scene items, they have been deleted by clear()
    nonVisiblePathItem = nullptr;
    visiblePathItem = nullptr;
    selectionItem = nullptr;
}

//---------------------------------------------------------
//   Timeline::changeSelection
//---------------------------------------------------------

void Timeline::changeSelection(SelState)
{
    TRACEFUNC;

    scene()->blockSignals(true);
    scene()->clearSelection();

    QRectF selectionRect = _selectionPath.boundingRect();
    if (selectionRect == QRectF()) {
        return;
    }

    int nmeta = nmetas();

    // Get borders of the current viewport
    int leftBorder = horizontalScrollBar()->value();
    int rightBorder = horizontalScrollBar()->value() + viewport()->width();
    int topBorder = verticalScrollBar()->value() + nmeta * _gridHeight;
    int bottomBorder = verticalScrollBar()->value() + viewport()->height();

    bool selectionExtendsUp = false,    selectionExtendsLeft = false;
    bool selectionExtendsRight = false, selectionExtendsDown = false;

    // Figure out which directions the selection extends
    if (selectionRect.top() < topBorder) {
        selectionExtendsUp = true;
    }
    if (selectionRect.left() < leftBorder - 1) {
        selectionExtendsLeft = true;
    }
    if (selectionRect.right() > rightBorder) {
        selectionExtendsRight = true;
    }
    if (selectionRect.bottom() > bottomBorder) {
        selectionExtendsDown = true;
    }

    if (selectionExtendsDown
        && _oldSelectionRect.bottom() != selectionRect.bottom()
        && !_metaValue) {
        int newScrollbarValue = int(verticalScrollBar()->value() + selectionRect.bottom() - bottomBorder);
        verticalScrollBar()->setValue(newScrollbarValue);
    } else if (selectionExtendsUp
               && !selectionExtendsDown
               && _oldSelectionRect.bottom() != selectionRect.bottom()
               && !_metaValue
               && _oldSelectionRect.contains(selectionRect)) {
        int newScrollbarValue = int(verticalScrollBar()->value() + selectionRect.bottom() - bottomBorder);
        verticalScrollBar()->setValue(newScrollbarValue);
    }

    if (selectionExtendsRight
        && _oldSelectionRect.right() != selectionRect.right()) {
        int newScrollbarValue = int(horizontalScrollBar()->value() + selectionRect.right() - rightBorder);
        horizontalScrollBar()->setValue(newScrollbarValue);
    }
    if (selectionExtendsUp
        && _oldSelectionRect.top() != selectionRect.top()
        && !_metaValue) {
        int newScrollbarValue = int(selectionRect.top()) - nmeta * _gridHeight;
        verticalScrollBar()->setValue(newScrollbarValue);
    }
    if (selectionExtendsLeft
        && _oldSelectionRect.left() != selectionRect.left()) {
        int newScrollbarValue = int(selectionRect.left());
        horizontalScrollBar()->setValue(newScrollbarValue);
    }

    if (selectionExtendsLeft
        && !selectionExtendsRight
        && _oldSelectionRect.right() != selectionRect.right()
        && _oldSelectionRect.contains(selectionRect)) {
        int newScrollbarValue = int(horizontalScrollBar()->value() + selectionRect.right() - rightBorder);
        horizontalScrollBar()->setValue(newScrollbarValue);
    }
    if (selectionExtendsRight
        && !selectionExtendsLeft
        && _oldSelectionRect.left() != selectionRect.left()
        && _oldSelectionRect.contains(selectionRect)) {
        int newScrollbarValue = int(selectionRect.left());
        horizontalScrollBar()->setValue(newScrollbarValue);
    }

    if (selectionExtendsDown
        && !selectionExtendsUp
        && _oldSelectionRect.top() != selectionRect.top()
        && !_metaValue
        && _oldSelectionRect.contains(selectionRect)) {
        int newScrollbarValue = int(selectionRect.top()) - nmeta * _gridHeight;
        verticalScrollBar()->setValue(newScrollbarValue);
    }

    _oldSelectionRect = selectionRect;

    _metaValue = false;
    scene()->blockSignals(false);
}

//---------------------------------------------------------
//   Timeline::drawSelection
//---------------------------------------------------------

void Timeline::drawSelection()
{
    if (!score()) {
        return;
    }

    TRACEFUNC;

    _selectionPath = QPainterPath();
    _selectionPath.setFillRule(Qt::WindingFill);

    std::set<std::tuple<Measure*, int, ElementType> > metaLabelsSet;

    INotationSelectionPtr selection = interaction()->selection();

    for (EngravingItem* element : selection->elements()) {
        if (element->tick() == Fraction(-1, 1)) {
            continue;
        } else {
            switch (element->type()) {
            case ElementType::INSTRUMENT_NAME:
            case ElementType::VBOX:
            case ElementType::HBOX:
            case ElementType::TEXT:
            case ElementType::TIE_SEGMENT:
            case ElementType::SLUR_SEGMENT:
            case ElementType::TIE:
            case ElementType::SLUR:
            case ElementType::HAMMER_ON_PULL_OFF:
                continue;
                break;
            default: break;
            }
        }

        int staffIdx;
        Fraction tick = element->tick();
        Measure* measure = score()->tick2measure(tick);
        staffIdx = static_cast<int>(element->staffIdx());
        if (numToStaff(staffIdx) && !numToStaff(staffIdx)->show()) {
            continue;
        }

        if ((element->isTempoText()
             || element->isKeySig()
             || element->isTimeSig()
             || element->isRehearsalMark()
             || element->isJump()
             || element->isMarker())
            && !element->generated()) {
            staffIdx = -1;
        }

        if (element->isBarLine()) {
            staffIdx = -1;
            BarLine* barline = toBarLine(element);
            if (barline
                && (barline->barLineType() == BarLineType::END_REPEAT
                    || barline->barLineType() == BarLineType::END
                    || barline->barLineType() == BarLineType::DOUBLE
                    || barline->barLineType() == BarLineType::REVERSE_END
                    || barline->barLineType() == BarLineType::HEAVY
                    || barline->barLineType() == BarLineType::DOUBLE_HEAVY)
                && measure != score()->lastMeasure()) {
                if (measure->prevMeasure()) {
                    measure = measure->prevMeasure();
                }
            }
        }

        // element->type() for meta rows, invalid for everything else
        ElementType elementType = (staffIdx == -1) ? element->type() : ElementType::INVALID;

        // If has a multi measure rest, find the count and add each measure to it
        // ws: If style flag Sid::createMultiMeasureRests is not set, then
        // measure->mmRest() is not valid

        if (measure->mmRest() && measure->score()->style().styleB(Sid::createMultiMeasureRests)) {
            int mmrestCount = measure->mmRest()->mmRestCount();
            Measure* tmpMeasure = measure;
            for (int mmrestMeasure = 0; mmrestMeasure < mmrestCount; mmrestMeasure++) {
                std::tuple<Measure*, int, ElementType> tmp(tmpMeasure, staffIdx, elementType);
                metaLabelsSet.insert(tmp);
                tmpMeasure = tmpMeasure->nextMeasure();
            }
        } else {
            std::tuple<Measure*, int, ElementType> tmp(measure, staffIdx, elementType);
            metaLabelsSet.insert(tmp);
        }
    }

    const QList<QGraphicsItem*> graphicsItemList = scene()->items();
    for (QGraphicsItem* graphicsItem : graphicsItemList) {
        int stave = graphicsItem->data(0).value<int>();
        ElementType elementType = graphicsItem->data(1).value<ElementType>();
        Measure* measure = static_cast<Measure*>(graphicsItem->data(2).value<void*>());

        std::tuple<Measure*, int, ElementType> targetTuple(measure, stave, elementType);
        std::set<std::tuple<Measure*, int, ElementType> >::iterator it;
        it = metaLabelsSet.find(targetTuple);

        if (stave == -1 && it != metaLabelsSet.end()) {
            //Make sure the element is correct
            const std::vector<EngravingItem*>& elementList = interaction()->selection()->elements();
            EngravingItem* targetElement = static_cast<EngravingItem*>(graphicsItem->data(4).value<void*>());
            Segment* seg = static_cast<Segment*>(graphicsItem->data(6).value<void*>());

            if (targetElement) {
                for (EngravingItem* element : elementList) {
                    if (element == targetElement) {
                        QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
                        if (graphicsRectItem) {
                            graphicsRectItem->setBrush(QBrush(activeTheme().selectionColor));
                        }
                    }
                }
            } else if (seg) {
                for (EngravingItem* element : elementList) {
                    QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
                    if (graphicsRectItem) {
                        for (size_t track = 0; track < score()->nstaves() * VOICES; track++) {
                            if (element == seg->element(track)) {
                                graphicsRectItem->setBrush(QBrush(activeTheme().selectionColor));
                            }
                        }
                    }
                }
            } else {
                QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
                if (graphicsRectItem) {
                    graphicsRectItem->setBrush(QBrush(activeTheme().selectionColor));
                }
            }
        }
        // Change color from gray to only blue
        else if (it != metaLabelsSet.end()) {
            QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
            graphicsRectItem->setBrush(QBrush(QColor(graphicsRectItem->brush().color().red(),
                                                     graphicsRectItem->brush().color().green(),
                                                     255)));
            _selectionPath.addRect(graphicsRectItem->rect());
        } else {
            // Ensure unselected measures are not marked selected
            QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
            if (graphicsRectItem && graphicsRectItem->data(keyItemType).value<ItemType>() == ItemType::TYPE_MEASURE) {
                graphicsRectItem->setBrush(QBrush(colorBox(graphicsRectItem)));
            }
        }
    }

    if (selectionItem) {
        scene()->removeItem(selectionItem);
        delete selectionItem;
        selectionItem = nullptr;
    }

    selectionItem = new QGraphicsPathItem(_selectionPath.simplified());
    if (selection->isRange()) {
        selectionItem->setPen(QPen(QColor(0, 0, 255), 3));
    } else {
        selectionItem->setPen(QPen(QColor(0, 0, 0), 1));
    }

    selectionItem->setBrush(Qt::NoBrush);
    selectionItem->setZValue(-1);
    scene()->addItem(selectionItem);

    if (std::get<0>(_oldHoverInfo)) {
        std::get<0>(_oldHoverInfo) = nullptr;
        std::get<1>(_oldHoverInfo) = -1;
    }
}

//---------------------------------------------------------
//   Timeline::mousePressEvent
//---------------------------------------------------------

void Timeline::mousePressEvent(QMouseEvent* event)
{
    if (!score()) {
        return;
    }

    if (event->button() == Qt::RightButton) {
        return;
    }

    TRACEFUNC;

    // Set as clicked
    _mousePressed = true;
    scene()->clearSelection();

    QPointF scenePt = mapToScene(event->pos());
    // Set as old location
    _oldLoc = QPoint(int(scenePt.x()), int(scenePt.y()));
    QList<QGraphicsItem*> graphicsItemList = scene()->items(scenePt);
    // Find highest z value for rect
    int maxZValue = -4;
    QGraphicsItem* currGraphicsItem = nullptr;
    for (QGraphicsItem* graphicsItem : graphicsItemList) {
        QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
        if (graphicsRectItem && graphicsItem->zValue() > maxZValue) {
            currGraphicsItem = graphicsItem;
            maxZValue = graphicsItem->zValue();
        }
    }
    if (currGraphicsItem) {
        int stave = currGraphicsItem->data(0).value<int>();
        Measure* currMeasure = static_cast<Measure*>(currGraphicsItem->data(2).value<void*>());
        if (numToStaff(stave) && !numToStaff(stave)->show()) {
            return;
        }

        if (!currMeasure) {
            int nmeta = nmetas();
            int bottomOfMeta = nmeta * _gridHeight + verticalScrollBar()->value();

            // Handle measure box clicks
            if (scenePt.y() > (nmeta - 1) * _gridHeight + verticalScrollBar()->value()
                && scenePt.y() < bottomOfMeta) {
                QRectF tmp(scenePt.x(), 0, 3, nmeta * _gridHeight + nstaves() * _gridHeight);
                QList<QGraphicsItem*> gl = scene()->items(tmp);
                Measure* measure = nullptr;

                for (QGraphicsItem* graphicsItem : gl) {
                    measure = static_cast<Measure*>(graphicsItem->data(2).value<void*>());
                    //-3 z value is the grid square values
                    if (graphicsItem->zValue() == -3 && measure) {
                        break;
                    }
                }

                if (measure) {
                    interaction()->showItem(measure);
                }
            }
            if (scenePt.y() < bottomOfMeta) {
                return;
            }

            QList<QGraphicsItem*> gl = items(event->pos());
            for (QGraphicsItem* graphicsItem : gl) {
                currMeasure = static_cast<Measure*>(graphicsItem->data(2).value<void*>());
                stave = graphicsItem->data(0).value<int>();
                if (currMeasure) {
                    break;
                }
            }
            if (!currMeasure) {
                interaction()->clearSelection();
                return;
            }
        }

        bool metaValueClicked = currGraphicsItem->data(3).value<bool>();

        scene()->clearSelection();
        if (metaValueClicked) {
            _metaValue = true;
            _oldSelectionRect = QRect();

            interaction()->showItem(currMeasure, 0);
            verticalScrollBar()->setValue(0);

            Segment* seg = static_cast<Segment*>(currGraphicsItem->data(6).value<void*>());

            if (seg) {
                std::vector<EngravingItem*> elements;

                for (size_t track = 0; track < score()->nstaves() * VOICES; track++) {
                    EngravingItem* element = seg->element(track);
                    if (element) {
                        elements.push_back(element);
                    }
                }

                if (elements.empty()) {
                    interaction()->clearSelection();
                } else {
                    interaction()->select(elements);
                }
            } else {
                // Also select the elements that they correspond to
                ElementType elementType = currGraphicsItem->data(1).value<ElementType>();
                SegmentType segmentType = SegmentType::Invalid;
                if (elementType == ElementType::KEYSIG) {
                    segmentType = SegmentType::KeySig;
                } else if (elementType == ElementType::TIMESIG) {
                    segmentType = SegmentType::TimeSig;
                }

                if (segmentType != SegmentType::Invalid) {
                    Segment* currSeg = currMeasure->first();
                    for (; currSeg && currSeg->segmentType() != segmentType; currSeg = currSeg->next()) {
                    }
                    if (currSeg) {
                        std::vector<EngravingItem*> elements;

                        for (size_t j = 0; j < score()->nstaves(); j++) {
                            EngravingItem* element = currSeg->firstElementForNavigation(j);
                            if (element) {
                                elements.push_back(element);
                            }
                        }

                        if (elements.empty()) {
                            interaction()->clearSelection();
                        } else {
                            interaction()->select(elements);
                        }
                    }
                } else {
                    // Select just the element for tempo_text
                    EngravingItem* element = static_cast<EngravingItem*>(currGraphicsItem->data(4).value<void*>());
                    if (element) {
                        interaction()->select({ element });
                    } else if (currMeasure) {
                        interaction()->select({ currMeasure });
                    } else {
                        interaction()->clearSelection();
                    }
                }
            }
        } else {
            // Handle cell clicks
            if (event->modifiers() == Qt::ShiftModifier) {
                if (currMeasure->mmRest()) {
                    currMeasure = currMeasure->mmRest();
                } else if (currMeasure->mmRestCount() == -1) {
                    currMeasure = currMeasure->prevMeasureMM();
                }

                if (currMeasure) {
                    interaction()->select({ currMeasure }, SelectType::RANGE, stave);
                }
            } else if (event->modifiers() == Qt::ControlModifier) {
                if (interaction()->selection()->isNone()) {
                    if (currMeasure->mmRest()) {
                        currMeasure = currMeasure->mmRest();
                    } else if (currMeasure->mmRestCount() == -1) {
                        currMeasure = currMeasure->prevMeasureMM();
                    }

                    if (currMeasure) {
                        interaction()->select({ currMeasure }, SelectType::RANGE, 0);
                        interaction()->select({ currMeasure }, SelectType::RANGE, score()->nstaves() - 1);
                    }
                } else {
                    interaction()->clearSelection();
                }
            } else {
                if (currMeasure->mmRest()) {
                    currMeasure = currMeasure->mmRest();
                } else if (currMeasure->mmRestCount() == -1) {
                    currMeasure = currMeasure->prevMeasureMM();
                }

                if (currMeasure) {
                    interaction()->select({ currMeasure }, SelectType::SINGLE, stave);
                }
            }

            if (currMeasure) {
                interaction()->showItem(currMeasure, stave);
            }
        }
    } else {
        interaction()->clearSelection();
    }
}

//---------------------------------------------------------
//   Timeline::mouseMoveEvent
//---------------------------------------------------------

void Timeline::mouseMoveEvent(QMouseEvent* event)
{
    QPointF newLoc = mapToScene(event->pos());
    if (!_mousePressed) {
        if (cursorIsOn(event->pos()) == "meta") {
            setCursor(Qt::ArrowCursor);
            mouseOver(newLoc);
        } else if (cursorIsOn(event->pos()) == "invalid") {
            setCursor(Qt::ForbiddenCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }

        emit moved(QPointF(-1, -1));
        return;
    }

    if (state == ViewState::NORMAL) {
        if (event->modifiers() == Qt::ShiftModifier) {
            // Slight wiggle room for selection (Same as score)
            if (std::abs(newLoc.x() - _oldLoc.x()) > 2
                || std::abs(newLoc.y() - _oldLoc.y()) > 2) {
                interaction()->clearSelection();
                updateGrid();
                state = ViewState::LASSO;
                _selectionBox = new QGraphicsRectItem();
                _selectionBox->setRect(_oldLoc.x(), _oldLoc.y(), 0, 0);
                _selectionBox->setPen(QPen(QColor(0, 0, 255), 2));
                _selectionBox->setBrush(QBrush(QColor(0, 0, 255, 50)));
                scene()->addItem(_selectionBox);
            }
        } else {
            state = ViewState::DRAG;
            setCursor(Qt::SizeAllCursor);
        }
    }

    if (state == ViewState::LASSO) {
        QRect tmp = QRect((_oldLoc.x() < newLoc.x()) ? _oldLoc.x() : newLoc.x(),
                          (_oldLoc.y() < newLoc.y()) ? _oldLoc.y() : newLoc.y(),
                          std::abs(newLoc.x() - _oldLoc.x()),
                          std::abs(newLoc.y() - _oldLoc.y()));
        _selectionBox->setRect(tmp);
    } else if (state == ViewState::DRAG) {
        int x_offset = int(_oldLoc.x()) - int(newLoc.x());
        int yOffset = int(_oldLoc.y()) - int(newLoc.y());
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + x_offset);
        verticalScrollBar()->setValue(verticalScrollBar()->value() + yOffset);
    }

    emit moved(QPointF(-1, -1));
}

//---------------------------------------------------------
//   Timeline::mouseReleaseEvent
//---------------------------------------------------------

void Timeline::mouseReleaseEvent(QMouseEvent*)
{
    _mousePressed = false;

    if (state == ViewState::LASSO) {
        scene()->removeItem(_selectionBox);
        interaction()->clearSelection();

        int width, height;
        QPoint loc = mapFromScene(_selectionBox->rect().topLeft());
        width = int(_selectionBox->rect().width());
        height = int(_selectionBox->rect().height());

        QList<QGraphicsItem*> graphicsItemList = items(QRect(loc.x(), loc.y(), width, height));
        // Find top left and bottom right to create selection
        QGraphicsItem* tlGraphicsItem = nullptr;
        QGraphicsItem* brGraphicsItem = nullptr;
        for (QGraphicsItem* graphicsItem : graphicsItemList) {
            Measure* currMeasure = static_cast<Measure*>(graphicsItem->data(2).value<void*>());
            if (!currMeasure) {
                continue;
            }
            int stave = graphicsItem->data(0).value<int>();
            if (stave == -1) {
                continue;
            }

            if (!tlGraphicsItem && !brGraphicsItem) {
                tlGraphicsItem = graphicsItem;
                brGraphicsItem = graphicsItem;
                continue;
            }

            if (graphicsItem->boundingRect().top() < tlGraphicsItem->boundingRect().top()) {
                tlGraphicsItem = graphicsItem;
            }
            if (graphicsItem->boundingRect().left() < tlGraphicsItem->boundingRect().left()) {
                tlGraphicsItem = graphicsItem;
            }

            if (graphicsItem->boundingRect().bottom() > brGraphicsItem->boundingRect().bottom()) {
                brGraphicsItem = graphicsItem;
            }
            if (graphicsItem->boundingRect().right() > brGraphicsItem->boundingRect().right()) {
                brGraphicsItem = graphicsItem;
            }
        }

        // Select single tlGraphicsItem and then range brGraphicsItem
        if (tlGraphicsItem && brGraphicsItem) {
            Measure* tlMeasure = static_cast<Measure*>(tlGraphicsItem->data(2).value<void*>());
            int tlStave = tlGraphicsItem->data(0).value<int>();
            Measure* brMeasure = static_cast<Measure*>(brGraphicsItem->data(2).value<void*>());
            int brStave = brGraphicsItem->data(0).value<int>();
            if (tlMeasure && brMeasure) {
                // Focus selection of mmRests here
                if (tlMeasure->mmRest()) {
                    tlMeasure = tlMeasure->mmRest();
                } else if (tlMeasure->mmRestCount() == -1) {
                    tlMeasure = tlMeasure->prevMeasureMM();
                }
                if (brMeasure->mmRest()) {
                    brMeasure = brMeasure->mmRest();
                } else if (brMeasure->mmRestCount() == -1) {
                    brMeasure = brMeasure->prevMeasureMM();
                }

                if (tlMeasure) {
                    interaction()->select({ tlMeasure }, SelectType::SINGLE, tlStave);
                }

                if (brMeasure) {
                    interaction()->select({ brMeasure }, SelectType::RANGE, brStave);
                }
            }

            if (tlMeasure) {
                interaction()->showItem(tlMeasure, tlStave);
            }
        }
    } else if (state == ViewState::DRAG) {
        setCursor(Qt::ArrowCursor);
    }
    state = ViewState::NORMAL;
}

//---------------------------------------------------------
//   Timeline::leaveEvent
//---------------------------------------------------------

void Timeline::leaveEvent(QEvent*)
{
    if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
        QPointF p = mapToScene(mapFromGlobal(QCursor::pos()));
        mouseOver(p);
    }
}

//---------------------------------------------------------
//   Timeline::wheelEvent
//---------------------------------------------------------

void Timeline::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        qreal originalCursorPos = mapToScene(mapFromGlobal(QCursor::pos())).x();
        int originalScrollValue = horizontalScrollBar()->value();
        qreal ratio = originalCursorPos / qreal(getWidth());

        if (event->angleDelta().y() > 0 && _gridWidth < _maxZoom) {
            _gridWidth++;
            updateGridFull();
        } else if (event->angleDelta().y() < 0 && _gridWidth > _minZoom) {
            _gridWidth--;
            updateGridFull();
        }

        // Attempt to keep mouse in original spot
        qreal newPos = qreal(getWidth()) * ratio;
        int offset = newPos - originalCursorPos;
        horizontalScrollBar()->setValue(originalScrollValue + offset);
    } else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
        qreal numOfSteps = qreal(event->angleDelta().y()) / 2;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - int(numOfSteps));
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void Timeline::showEvent(QShowEvent* evt)
{
    QGraphicsView::showEvent(evt);
    if (!evt->spontaneous()) {
        setNotation(m_notation);
    }
}

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void Timeline::changeEvent(QEvent* event)
{
    QGraphicsView::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        _metas.clear();
        _metas.push_back({ muse::qtrc("notation/timeline", "Tempo"), &Timeline::tempoMeta, true });
        _metas.push_back({ muse::qtrc("notation/timeline", "Time signature"), &Timeline::timeMeta, true });
        _metas.push_back({ muse::qtrc("notation/timeline", "Rehearsal mark"), &Timeline::rehearsalMeta, true });
        _metas.push_back({ muse::qtrc("notation/timeline", "Key signature"), &Timeline::keyMeta, true });
        _metas.push_back({ muse::qtrc("notation/timeline", "Barlines"), &Timeline::barlineMeta, true });
        _metas.push_back({ muse::qtrc("notation/timeline", "Jumps and markers"), &Timeline::jumpMarkerMeta, true });
        _metas.push_back({ muse::qtrc("notation/timeline", "Measures"), &Timeline::measureMeta, true });

        updateGridFull();
    }
}

//---------------------------------------------------------
//   Timeline::updateGrid
//---------------------------------------------------------

void Timeline::updateGrid(int startMeasure, int endMeasure)
{
    TRACEFUNC;

    if (score() && score()->firstMeasure()) {
        drawGrid(static_cast<int>(nstaves()), static_cast<int>(score()->nmeasures()), startMeasure, endMeasure);
        updateView();
        drawSelection();
        mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));
        _rowNames->updateLabels(getLabels(), _gridHeight);
    }
    viewport()->update();
}

//---------------------------------------------------------
//   updateGridFromCmdState
//---------------------------------------------------------

void Timeline::updateGridFromCmdState()
{
    if (!score()) {
        updateGridFull();
        return;
    }

    const CmdState& cState = score()->cmdState();

    const bool layoutChanged = cState.layoutRange();

    if (!layoutChanged) {
        updateGridView();
        return;
    }

    const bool layoutAll = layoutChanged && (cState.startTick() < Fraction(0, 1) || cState.endTick() < Fraction(0, 1));

    const Measure* startMeasure = layoutAll ? nullptr : score()->tick2measure(cState.startTick());
    const int startMeasureIndex = startMeasure ? startMeasure->measureIndex() : 0;

    const Measure* endMeasure = layoutAll ? nullptr : score()->tick2measure(cState.endTick());
    const int endMeasureIndex = endMeasure ? (endMeasure->measureIndex() + 1) : static_cast<int>(score()->nmeasures());

    updateGrid(startMeasureIndex, endMeasureIndex);
}

//---------------------------------------------------------
//   Timeline::setNotation
//---------------------------------------------------------

void Timeline::setNotation(INotationPtr notation)
{
    m_notation = notation;

    clearScene();

    if (m_notation) {
        drawGrid(nstaves(), static_cast<int>(score()->nmeasures()));
        drawSelection();
        changeSelection(SelState::NONE);
        _rowNames->updateLabels(getLabels(), _gridHeight);
    } else {
        // Clear timeline if no score is present
        if (_splitter && _splitter->count() > 0) {
            TRowLabels* tRowLabels = static_cast<TRowLabels*>(_splitter->widget(0));
            std::vector<std::pair<QString, bool> > noLabels;
            tRowLabels->updateLabels(noLabels, 0);
        }
        _metaRows.clear();
        setSceneRect(0, 0, 0, 0);
    }
}

//---------------------------------------------------------
//   Timeline::updateView
//---------------------------------------------------------

void Timeline::updateView()
{
    if (!score()) {
        return;
    }

    TRACEFUNC;

    //! FIXME
    RectF canvas;        // = QRectF(_cv->matrix().inverted().mapRect(_cv->geometry()));

    // Find visible elements in timeline
    QPainterPath visiblePainterPath = QPainterPath();
    visiblePainterPath.setFillRule(Qt::WindingFill);

    // Find visible measures of score
    int measureIndex = 0;
    const int numMetas = nmetas();

    for (Measure* currMeasure = score()->firstMeasure(); currMeasure; currMeasure = currMeasure->nextMeasure(), ++measureIndex) {
        System* system = currMeasure->system();

        if (currMeasure->mmRest() && score()->style().styleB(Sid::createMultiMeasureRests)) {
            // Handle mmRests
            Measure* mmrestMeasure = currMeasure->mmRest();
            system = mmrestMeasure->system();
            if (!system) {
                measureIndex += currMeasure->mmRestCount();
                continue;
            }

            // Add all measures within mmRest to visibleItemsSet if mmRest_visible
            for (; currMeasure != mmrestMeasure->mmRestLast(); currMeasure = currMeasure->nextMeasure(), ++measureIndex) {
                for (size_t staff = 0; staff < score()->staves().size(); staff++) {
                    if (!score()->staff(staff)->show()) {
                        continue;
                    }
                    RectF staveRect = RectF(system->canvasBoundingRect().left(),
                                            system->staffCanvasYpage(staff),
                                            system->width(),
                                            system->staff(staff)->bbox().height());
                    RectF showRect = mmrestMeasure->canvasBoundingRect().intersected(staveRect);

                    if (canvas.intersects(showRect)) {
                        visiblePainterPath.addRect(getMeasureRect(measureIndex, static_cast<int>(staff), numMetas));
                    }
                }
            }

            // Handle last measure in mmRest
            for (size_t staff = 0; staff < score()->staves().size(); staff++) {
                if (!score()->staff(staff)->show()) {
                    continue;
                }
                RectF staveRect = RectF(system->canvasBoundingRect().left(),
                                        system->staffCanvasYpage(staff),
                                        system->width(),
                                        system->staff(staff)->bbox().height());
                RectF showRect = mmrestMeasure->canvasBoundingRect().intersected(staveRect);

                if (canvas.intersects(showRect)) {
                    visiblePainterPath.addRect(getMeasureRect(measureIndex, static_cast<int>(staff), numMetas));
                }
            }
            continue;
        }

        if (!system) {
            continue;
        }

        for (size_t staff = 0; staff < score()->staves().size(); staff++) {
            if (!score()->staff(staff)->show()) {
                continue;
            }
            RectF staveRect = RectF(system->canvasBoundingRect().left(),
                                    system->staffCanvasYpage(staff),
                                    system->width(),
                                    system->staff(staff)->bbox().height());
            RectF showRect = currMeasure->canvasBoundingRect().intersected(staveRect);

            if (canvas.intersects(showRect)) {
                visiblePainterPath.addRect(getMeasureRect(measureIndex, static_cast<int>(staff), numMetas));
            }
        }
    }

    if (nonVisiblePathItem) {
        scene()->removeItem(nonVisiblePathItem);
        delete nonVisiblePathItem;
        nonVisiblePathItem = nullptr;
    }
    if (visiblePathItem) {
        scene()->removeItem(visiblePathItem);
        delete visiblePathItem;
        visiblePathItem = nullptr;
    }

    QPainterPath nonVisiblePainterPath = QPainterPath();
    nonVisiblePainterPath.setFillRule(Qt::WindingFill);

    QRectF timelineRect = QRectF(0, 0, getWidth(), getHeight());
    nonVisiblePainterPath.addRect(timelineRect);

    nonVisiblePainterPath = nonVisiblePainterPath.subtracted(visiblePainterPath);

    nonVisiblePathItem = new QGraphicsPathItem(nonVisiblePainterPath.simplified());

    QPen nonVisiblePen = QPen(activeTheme().nonVisiblePenColor);
    QBrush nonVisibleBrush = QBrush(activeTheme().nonVisibleBrushColor);
    nonVisiblePathItem->setPen(QPen(nonVisibleBrush.color()));
    nonVisiblePathItem->setBrush(nonVisibleBrush);
    nonVisiblePathItem->setZValue(-3);

    visiblePathItem = new QGraphicsPathItem(visiblePainterPath.simplified());
    visiblePathItem->setPen(nonVisiblePen);
    visiblePathItem->setBrush(Qt::NoBrush);
    visiblePathItem->setZValue(-2);

    scene()->addItem(nonVisiblePathItem);
    scene()->addItem(visiblePathItem);
}

//---------------------------------------------------------
//   Timeline::nstaves
//---------------------------------------------------------

int Timeline::nstaves() const
{
    return static_cast<int>(score()->staves().size());
}

//---------------------------------------------------------
//   Timeline::colorBox
//---------------------------------------------------------

QColor Timeline::colorBox(QGraphicsRectItem* item)
{
    Measure* measure = static_cast<Measure*>(item->data(2).value<void*>());
    staff_idx_t stave = static_cast<staff_idx_t>(item->data(0).value<int>());
    for (Segment* seg = measure->first(); seg; seg = seg->next()) {
        if (!seg->isChordRestType()) {
            continue;
        }
        for (track_idx_t track = stave * VOICES; track < stave * VOICES + VOICES; track++) {
            ChordRest* chordRest = seg->cr(track);
            if (chordRest) {
                ElementType crt = chordRest->type();
                if (crt == ElementType::CHORD || crt == ElementType::MEASURE_REPEAT) {
                    return activeTheme().colorBoxColor;
                }
            }
        }
    }
    return QColor(224, 224, 224);
}

//---------------------------------------------------------
//   Timeline::getLabels
//---------------------------------------------------------

std::vector<std::pair<QString, bool> > Timeline::getLabels()
{
    if (!score()) {
        std::vector<std::pair<QString, bool> > noLabels;
        return noLabels;
    }

    QList<Part*> partList = getParts();
    // Transfer them into a vector of qstrings and then add the meta row names
    std::vector<std::pair<QString, bool> > rowLabels;
    if (_collapsedMeta) {
        std::pair<QString, bool> first = std::make_pair("", true);
        std::pair<QString, bool> second = std::make_pair(muse::qtrc("notation/timeline", "Measures"), true);
        rowLabels.push_back(first);
        rowLabels.push_back(second);
    } else {
        for (auto it = _metas.begin(); it != _metas.end(); ++it) {
            std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
            if (!std::get<2>(meta)) {
                continue;
            }
            std::pair<QString, bool> metaLabel = std::make_pair(std::get<0>(meta), true);
            rowLabels.push_back(metaLabel);
        }
    }

    for (int stave = 0; stave < partList.size(); stave++) {
        QTextDocument doc;
        QString partName = "";
        doc.setHtml(partList.at(stave)->longName());
        partName = doc.toPlainText();
        if (partName.isEmpty()) {     // No Long instrument name? Fall back to Part name
            doc.setHtml(partList.at(stave)->partName());
            partName = doc.toPlainText();
        }
        if (partName.isEmpty()) {   // No Part name? Fall back to Instrument name
            partName = partList.at(stave)->instrumentName();
        }

        std::pair<QString, bool> instrumentLabel = std::make_pair(partName, partList.at(stave)->show());
        rowLabels.push_back(instrumentLabel);
    }
    return rowLabels;
}

//---------------------------------------------------------
//   Timeline::handleScroll
//---------------------------------------------------------

void Timeline::handleScroll(int value)
{
    if (!score()) {
        return;
    }

    for (auto it = _metaRows.begin(); it != _metaRows.end(); ++it) {
        std::pair<QGraphicsItem*, int> pairGraphicsInt = *it;

        QGraphicsItem* graphicsItem = pairGraphicsInt.first;
        QGraphicsRectItem* graphicsRectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItem);
        QGraphicsLineItem* graphicsLineItem = qgraphicsitem_cast<QGraphicsLineItem*>(graphicsItem);
        QGraphicsPixmapItem* graphicsPixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphicsItem);

        int rowY = pairGraphicsInt.second * _gridHeight;
        int scrollbarValue = value;

        if (graphicsRectItem) {
            QRectF rectf = graphicsRectItem->rect();
            rectf.setY(qreal(scrollbarValue + rowY));
            rectf.setHeight(_gridHeight);
            graphicsRectItem->setRect(rectf);
        } else if (graphicsLineItem) {
            QLineF linef = graphicsLineItem->line();
            linef.setLine(linef.x1(), rowY + scrollbarValue + 1, linef.x2(), rowY + scrollbarValue + 1);
            graphicsLineItem->setLine(linef);
        } else if (graphicsPixmapItem) {
            graphicsPixmapItem->setY(qreal(scrollbarValue + rowY + 3));
        } else {
            graphicsItem->setY(qreal(scrollbarValue + rowY));
        }
    }
    viewport()->update();
}

//---------------------------------------------------------
//   Timeline::mouseOver
//---------------------------------------------------------

void Timeline::mouseOver(QPointF pos)
{
    TRACEFUNC;

    // Choose item with the largest original Z value...
    QList<QGraphicsItem*> graphicsList = scene()->items(pos);
    QGraphicsItem* hoveredGraphicsItem = 0;
    int maxZValue = -1;
    for (QGraphicsItem* currGraphicsItem : graphicsList) {
        if (qgraphicsitem_cast<QGraphicsTextItem*>(currGraphicsItem)) {
            continue;
        }
        if (currGraphicsItem->zValue() >= maxZValue && currGraphicsItem->zValue() < _globalZValue) {
            hoveredGraphicsItem = currGraphicsItem;
            maxZValue = hoveredGraphicsItem->zValue();
        } else if (currGraphicsItem->zValue() > _globalZValue && std::get<1>(_oldHoverInfo) >= maxZValue) {
            hoveredGraphicsItem = currGraphicsItem;
            maxZValue = std::get<1>(_oldHoverInfo);
        }
    }

    if (!hoveredGraphicsItem) {
        if (std::get<0>(_oldHoverInfo)) {
            std::get<0>(_oldHoverInfo)->setZValue(std::get<1>(_oldHoverInfo));
            static_cast<QGraphicsItem*>(std::get<0>(_oldHoverInfo)->data(5).value<void*>())->setZValue(std::get<1>(_oldHoverInfo));
            QGraphicsRectItem* graphicsRectItem1 = qgraphicsitem_cast<QGraphicsRectItem*>(std::get<0>(_oldHoverInfo));
            QGraphicsRectItem* graphicsRectItem2
                = qgraphicsitem_cast<QGraphicsRectItem*>(static_cast<QGraphicsItem*>(std::get<0>(_oldHoverInfo)->data(5).value<void*>()));
            if (graphicsRectItem1) {
                graphicsRectItem1->setBrush(QBrush(std::get<2>(_oldHoverInfo)));
            }
            if (graphicsRectItem2) {
                graphicsRectItem2->setBrush(QBrush(std::get<2>(_oldHoverInfo)));
            }
            std::get<0>(_oldHoverInfo) = nullptr;
            std::get<1>(_oldHoverInfo) = -1;
        }
        return;
    }
    QGraphicsItem* pairItem = static_cast<QGraphicsItem*>(hoveredGraphicsItem->data(5).value<void*>());
    if (!pairItem) {
        if (std::get<0>(_oldHoverInfo)) {
            std::get<0>(_oldHoverInfo)->setZValue(std::get<1>(_oldHoverInfo));
            static_cast<QGraphicsItem*>(std::get<0>(_oldHoverInfo)->data(5).value<void*>())->setZValue(std::get<1>(_oldHoverInfo));
            QGraphicsRectItem* graphicsRectItem1 = qgraphicsitem_cast<QGraphicsRectItem*>(std::get<0>(_oldHoverInfo));
            QGraphicsRectItem* graphicsRectItem2
                = qgraphicsitem_cast<QGraphicsRectItem*>(static_cast<QGraphicsItem*>(std::get<0>(_oldHoverInfo)->data(5).value<void*>()));
            if (graphicsRectItem1) {
                graphicsRectItem1->setBrush(QBrush(std::get<2>(_oldHoverInfo)));
            }
            if (graphicsRectItem2) {
                graphicsRectItem2->setBrush(QBrush(std::get<2>(_oldHoverInfo)));
            }
            std::get<0>(_oldHoverInfo) = nullptr;
            std::get<1>(_oldHoverInfo) = -1;
        }
        return;
    }

    if (std::get<0>(_oldHoverInfo) == hoveredGraphicsItem) {
        return;
    }

    if (std::get<0>(_oldHoverInfo)) {
        std::get<0>(_oldHoverInfo)->setZValue(std::get<1>(_oldHoverInfo));
        static_cast<QGraphicsItem*>(std::get<0>(_oldHoverInfo)->data(5).value<void*>())->setZValue(std::get<1>(_oldHoverInfo));
        QGraphicsRectItem* graphicsRectItem1 = qgraphicsitem_cast<QGraphicsRectItem*>(std::get<0>(_oldHoverInfo));
        QGraphicsRectItem* graphicsRectItem2
            = qgraphicsitem_cast<QGraphicsRectItem*>(static_cast<QGraphicsItem*>(std::get<0>(_oldHoverInfo)->data(5).value<void*>()));
        if (graphicsRectItem1) {
            graphicsRectItem1->setBrush(QBrush(std::get<2>(_oldHoverInfo)));
        }
        if (graphicsRectItem2) {
            graphicsRectItem2->setBrush(QBrush(std::get<2>(_oldHoverInfo)));
        }

        std::get<0>(_oldHoverInfo) = nullptr;
        std::get<1>(_oldHoverInfo) = -1;
    }

    std::get<1>(_oldHoverInfo) = hoveredGraphicsItem->zValue();
    std::get<0>(_oldHoverInfo) = hoveredGraphicsItem;

    // Give items the top z value
    hoveredGraphicsItem->setZValue(_globalZValue + 1);
    pairItem->setZValue(_globalZValue + 1);

    QGraphicsRectItem* graphicsRectItem1 = qgraphicsitem_cast<QGraphicsRectItem*>(hoveredGraphicsItem);
    QGraphicsRectItem* graphicsRectItem2 = qgraphicsitem_cast<QGraphicsRectItem*>(pairItem);
    if (graphicsRectItem1) {
        std::get<2>(_oldHoverInfo) = graphicsRectItem1->brush().color();
        if (std::get<2>(_oldHoverInfo) != activeTheme().selectionColor) {
            graphicsRectItem1->setBrush(QBrush(activeTheme().backgroundColor));
        }
    }
    if (graphicsRectItem2) {
        std::get<2>(_oldHoverInfo) = graphicsRectItem2->brush().color();
        if (std::get<2>(_oldHoverInfo) != activeTheme().selectionColor) {
            graphicsRectItem2->setBrush(QBrush(activeTheme().backgroundColor));
        }
    }
}

//---------------------------------------------------------
//   Timeline::swapMeta
//---------------------------------------------------------

void Timeline::swapMeta(unsigned row, bool switchUp)
{
    // Attempt to switch row up or down, skipping non visible rows
    if (switchUp && row != 0) {
        // traverse backwards until visible one is found
        auto swap = _metas.begin() + correctMetaRow(row) - 1;
        while (!std::get<2>(*swap)) {
            swap--;
        }
        iter_swap(_metas.begin() + correctMetaRow(row), swap);
    } else if (!switchUp && row != nmetas() - 2) {
        // traverse forwards until visible one is found
        auto swap = _metas.begin() + correctMetaRow(row) + 1;
        while (!std::get<2>(*swap)) {
            swap++;
        }
        iter_swap(_metas.begin() + correctMetaRow(row), swap);
    }

    updateGrid();
}

//---------------------------------------------------------
//   Timeline::numToStaff
//---------------------------------------------------------

Staff* Timeline::numToStaff(int staff)
{
    if (!score()) {
        return nullptr;
    }

    size_t staffIdx = static_cast<size_t>(staff);

    const std::vector<Staff*>& staves = score()->staves();
    if (staffIdx < staves.size()) {
        return staves.at(staffIdx);
    } else {
        return nullptr;
    }
}

//---------------------------------------------------------
//   Timeline::toggleShow
//---------------------------------------------------------

void Timeline::toggleShow(int staff)
{
    if (!score()) {
        return;
    }

    QList<Part*> parts = getParts();
    if (staff < 0 || staff >= parts.size()) {
        return;
    }

    Part* part = parts.at(staff);

    bool newShow = !part->show();
    TranslatableString actionName = newShow
                                    ? TranslatableString("undoableAction", "Show instrument")
                                    : TranslatableString("undoableAction", "Hide instrument");

    m_notation->undoStack()->prepareChanges(actionName);
    part->undoChangeProperty(Pid::VISIBLE, newShow);
    m_notation->undoStack()->commitChanges();
    m_notation->notationChanged().notify();
}

//---------------------------------------------------------
//   Timeline::showContextMenu
//---------------------------------------------------------

void Timeline::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu* contextMenu = new QMenu(muse::qtrc("notation/timeline", "Context menu"), this);
    if (_rowNames->cursorIsOn() == "instrument") {
        QAction* edit_instruments = new QAction(muse::qtrc("notation/timeline", "Edit instruments"), this);
        connect(edit_instruments, &QAction::triggered, this, &Timeline::requestInstrumentDialog);
        contextMenu->addAction(edit_instruments);
        contextMenu->exec(QCursor::pos());
    } else if (_rowNames->cursorIsOn() == "meta" || cursorIsOn(event->pos()) == "meta") {
        for (auto it = _metas.begin(); it != _metas.end(); ++it) {
            std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
            QString row_name = std::get<0>(meta);
            if (row_name != muse::qtrc("notation/timeline", "Measures")) {
                QAction* action = new QAction(row_name, this);
                action->setCheckable(true);
                action->setChecked(std::get<2>(meta));
                connect(action, &QAction::triggered, this, &Timeline::toggleMetaRow);
                contextMenu->addAction(action);
            }
        }
        contextMenu->addSeparator();
        QAction* hide_all = new QAction(muse::qtrc("notation/timeline", "Hide all"), this);
        connect(hide_all, &QAction::triggered, this, &Timeline::toggleMetaRow);
        contextMenu->addAction(hide_all);
        QAction* show_all = new QAction(muse::qtrc("notation/timeline", "Show all"), this);
        connect(show_all, &QAction::triggered, this, &Timeline::toggleMetaRow);
        contextMenu->addAction(show_all);
        contextMenu->exec(QCursor::pos());
    }
}

//---------------------------------------------------------
//   Timeline::toggleMetaRow
//---------------------------------------------------------

void Timeline::toggleMetaRow()
{
    QAction* action = qobject_cast<QAction*>(QObject::sender());
    if (!action) {
        return;
    }

    QString targetText = action->text();

    if (targetText == muse::qtrc("notation/timeline", "Hide all")) {
        for (auto it = _metas.begin(); it != _metas.end(); ++it) {
            QString metaText = std::get<0>(*it);
            if (metaText != muse::qtrc("notation/timeline", "Measures")) {
                std::get<2>(*it) = false;
            }
        }
        updateGrid();
        return;
    } else if (targetText == muse::qtrc("notation/timeline", "Show all")) {
        for (auto it = _metas.begin(); it != _metas.end(); ++it) {
            std::get<2>(*it) = true;
        }
        updateGrid();
        return;
    }

    bool checked = action->isChecked();
    // Find target text in metas and toggle visibility to the checked status of action
    for (auto it = _metas.begin(); it != _metas.end(); ++it) {
        QString metaText = std::get<0>(*it);
        if (metaText == targetText) {
            std::get<2>(*it) = checked;
            updateGrid();
            break;
        }
    }
}

//---------------------------------------------------------
//   Timeline::nmetas
//---------------------------------------------------------

unsigned Timeline::nmetas() const
{
    unsigned total = 0;
    if (_collapsedMeta) {
        return 2;
    }
    for (auto it = _metas.begin(); it != _metas.end(); ++it) {
        std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
        if (std::get<2>(meta)) {
            total++;
        }
    }
    return total;
}

//---------------------------------------------------------
//   Timeline::correctMetaRow
//---------------------------------------------------------

unsigned Timeline::correctMetaRow(unsigned row)
{
    unsigned count = 0;
    auto it = _metas.begin();
    while (row >= count) {
        if (!std::get<2>(*it)) {
            row++;
        }
        count++;
        ++it;
    }
    return row;
}

//---------------------------------------------------------
//   Timeline::correctMetaRow
//---------------------------------------------------------

QString Timeline::cursorIsOn(const QPoint& cursorPos)
{
    QGraphicsItem* graphicsItem = scene()->itemAt(cursorPos, transform());
    if (!graphicsItem) {
        return "";
    }

    auto it = _metaRows.begin();
    for (; it != _metaRows.end(); ++it) {
        if ((*it).first == graphicsItem) {
            break;
        }
    }
    if (it != _metaRows.end()) {
        return "meta";
    }
    QList<QGraphicsItem*> graphicsItemList = scene()->items(cursorPos);
    for (QGraphicsItem* currGraphicsItem : graphicsItemList) {
        Measure* currMeasure = static_cast<Measure*>(currGraphicsItem->data(2).value<void*>());
        int stave = currGraphicsItem->data(0).value<int>();
        const Staff* st = numToStaff(stave);
        if (currMeasure && !(st && st->show())) {
            return "invalid";
        }
    }
    return "instrument";
}

//---------------------------------------------------------
//   Timeline::activeTheme
//---------------------------------------------------------

const TimelineTheme& Timeline::activeTheme() const
{
    if (uiConfiguration()->currentTheme().codeKey == muse::ui::DARK_THEME_CODE) {
        return _darkTheme;
    }

    return _lightTheme;
}

//---------------------------------------------------------
//   Timeline::updateTimelineTheme
//---------------------------------------------------------

void Timeline::updateTimelineTheme()
{
    const QBrush backgroundBrush = QBrush(activeTheme().backgroundColor);
    scene()->setBackgroundBrush(backgroundBrush);
    _rowNames->scene()->setBackgroundBrush(backgroundBrush);
    updateGrid();
}

//---------------------------------------------------------
//   Timeline::requestInstrumentDialog
//---------------------------------------------------------

void Timeline::requestInstrumentDialog()
{
    dispatcher()->dispatch("instruments");
}

INotationInteractionPtr Timeline::interaction() const
{
    return m_notation ? m_notation->interaction() : nullptr;
}

Score* Timeline::score() const
{
    return m_notation ? m_notation->elements()->msScore() : nullptr;
}

TRowLabels* Timeline::labelsColumn() const
{
    return _rowNames;
}
