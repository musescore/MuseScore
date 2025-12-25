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

#ifndef MU_NOTATION_TIMELINE_H
#define MU_NOTATION_TIMELINE_H

#include "engraving/dom/select.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "notation/inotation.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"

#include <vector>
#include <QGraphicsView>
#include <QSplitter>

namespace mu::engraving {
class Score;
class Page;
}

namespace mu::notation {
class Timeline;

class TRowLabels : public QGraphicsView
{
    Q_OBJECT

    friend class TimelineAdapter;

public:
    enum class MouseOverValue {
        NONE,
        MOVE_UP_ARROW,
        MOVE_DOWN_ARROW,
        MOVE_UP_DOWN_ARROW,
        COLLAPSE_UP_ARROW,
        COLLAPSE_DOWN_ARROW,
        OPEN_EYE,
        CLOSED_EYE
    };

private:
    QSplitter* _splitter { nullptr };
    Timeline* _timeline { nullptr };

    QPoint _oldLoc;

    bool _dragging = false;

    std::vector<std::pair<QGraphicsItem*, int> > _metaLabels;
    std::map<MouseOverValue, QPixmap*> _mouseoverMap;
    std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> _oldItemInfo;

    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    void leaveEvent(QEvent*) override;

private slots:
    void restrictScroll(int value);

public slots:
    void mouseOver(QPointF scenePt);

signals:
    void moved(QPointF p);
    void swapMeta(unsigned r, bool up);
    void requestContextMenu(QContextMenuEvent*);

public:
    TRowLabels(QSplitter* splitter, Timeline* time);

    bool handleEvent(QEvent* event);

    void updateLabels(std::vector<std::pair<QString, bool> > labels, int height);
    QString cursorIsOn();
};

struct TimelineTheme {
    QColor backgroundColor, labelsColor1, labelsColor2, labelsColor3, gridColor1, gridColor2;
    QColor measureMetaColor, selectionColor, nonVisiblePenColor, nonVisibleBrushColor, colorBoxColor;
    QColor metaValuePenColor, metaValueBrushColor;
};

class Timeline : public QGraphicsView, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    enum class ItemType {
        TYPE_UNKNOWN = 0,
        TYPE_MEASURE,
        TYPE_META,
    };
    Q_ENUM(ItemType)

    Timeline(QSplitter* splitter);

    bool handleEvent(QEvent* event);

    void updateGridView() { updateGrid(-1, -1); }
    void updateGridFromCmdState();
    void setNotation(INotationPtr notation);

    TRowLabels* labelsColumn() const;

private:
    friend class TRowLabels;

    enum class ViewState {
        NORMAL,
        LASSO,
        DRAG
    };

    ViewState state = ViewState::NORMAL;

    static constexpr int keyItemType = 15;

    int _gridWidth = 20;
    int _gridHeight = 20;
    int _maxZoom = 50;
    int _minZoom = 5;
    int _spacing = 5;

    TimelineTheme _lightTheme, _darkTheme;

    std::tuple<int, qreal, EngravingItem*, EngravingItem*, bool> _repeatInfo;
    std::tuple<QGraphicsItem*, int, QColor> _oldHoverInfo;

    std::map<engraving::BarLineType, QPixmap*> _barlines;
    bool _isBarline { false };

    QSplitter* _splitter { nullptr };
    TRowLabels* _rowNames { nullptr };

    INotationPtr m_notation;

    int gridRows = 0;
    int gridCols = 0;

    QGraphicsPathItem* nonVisiblePathItem = nullptr;
    QGraphicsPathItem* visiblePathItem = nullptr;
    QGraphicsPathItem* selectionItem = nullptr;

    QGraphicsRectItem* _selectionBox { nullptr };
    std::vector<std::pair<QGraphicsItem*, int> > _metaRows;

    QPainterPath _selectionPath;
    QRectF _oldSelectionRect;
    bool _mousePressed { false };
    QPoint _oldLoc;

    bool _collapsedMeta { false };

    std::vector<std::tuple<QString, void (Timeline::*)(engraving::Segment*, int*, int), bool> > _metas;
    void tempoMeta(engraving::Segment* seg, int* stagger, int pos);
    void timeMeta(engraving::Segment* seg, int* stagger, int pos);
    void measureMeta(engraving::Segment*, int*, int pos);
    void rehearsalMeta(engraving::Segment* seg, int* stagger, int pos);
    void keyMeta(engraving::Segment* seg, int* stagger, int pos);
    void barlineMeta(engraving::Segment* seg, int* stagger, int pos);
    void jumpMarkerMeta(engraving::Segment* seg, int* stagger, int pos);

    bool addMetaValue(int x, int pos, QString metaText, int row, engraving::ElementType elementType, engraving::EngravingItem* element,
                      engraving::Segment* seg, engraving::Measure* measure, QString tooltip = "");
    void setMetaData(QGraphicsItem* gi, int staff, engraving::ElementType et, Measure* m, bool full_measure, engraving::EngravingItem* e,
                     QGraphicsItem* pairItem = nullptr, engraving::Segment* seg = nullptr);
    unsigned getMetaRow(QString targetText);

    int _globalMeasureNumber { 0 };
    int _globalZValue        { 0 };

    // True if meta value was last clicked
    bool _metaValue = false;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent*) override;
    void showEvent(QShowEvent*) override;
    void changeEvent(QEvent*) override;

    unsigned correctMetaRow(unsigned row);
    engraving::staff_idx_t correctStave(engraving::staff_idx_t stave);

    QList<engraving::Part*> getParts();

    QRectF getMeasureRect(int measureIndex, int row, int numMetas)
    {
        return QRectF(measureIndex * _gridWidth, _gridHeight * (row + numMetas) + 3, _gridWidth, _gridHeight);
    }

    void clearScene();

    void updateGrid(int startMeasure = -1, int endMeasure = -1);

    INotationInteractionPtr interaction() const;
    engraving::Score* score() const;

private slots:
    void handleScroll(int value);

    void changeSelection(engraving::SelState);
    void mouseOver(QPointF pos);
    void swapMeta(unsigned row, bool switchUp);
    void requestInstrumentDialog();
    void toggleMetaRow();
    void updateTimelineTheme();

    void contextMenuEvent(QContextMenuEvent* event) override;

signals:
    void moved(QPointF);

private:
    int correctPart(engraving::staff_idx_t stave);

    void updateView();
    void drawSelection();
    void drawGrid(int globalRows, int globalCols, int startMeasure = 0, int endMeasure = -1);

    int nstaves() const;

    int getWidth() const;
    int getHeight() const;
    const TimelineTheme& activeTheme() const;

    void updateGridFull() { updateGrid(0, -1); }

    QColor colorBox(QGraphicsRectItem* item);

    std::vector<std::pair<QString, bool> > getLabels();

    unsigned nmetas() const;

    bool collapsed() const { return _collapsedMeta; }
    void setCollapsed(bool st) { _collapsedMeta = st; }

    engraving::Staff* numToStaff(int staff);
    void toggleShow(int staff);
    QString cursorIsOn(const QPoint& cursorPos);
};
}

#endif // MU_NOTATION_TIMELINE_H
