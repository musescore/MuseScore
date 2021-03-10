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
#ifndef MU_DRAW_BUFFEREDPAINTPROVIDER_H
#define MU_DRAW_BUFFEREDPAINTPROVIDER_H

#include <vector>

#include <QPen>
#include <QBrush>
#include <QFont>

#include "ipaintprovider.h"
#include "drawtypes.h"
#include "drawobjectslogger.h"

namespace mu::draw {
struct DrawBuffer
{
    struct State {
        QPen pen;
        QBrush brush;
        QFont font;
        bool isAntialiasing = false;
        QTransform transform;
        CompositionMode compositionMode = CompositionMode::SourceOver;
    };

    struct Data {
        State state;

        std::vector<DrawPath> paths;
        std::vector<DrawPolygon> polygons;
        std::vector<DrawText> texts;
        std::vector<DrawRectText> rectTexts;
        std::vector<DrawGlyphRun> glyphs;
        std::vector<DrawPixmap> pixmaps;
        std::vector<DrawTiledPixmap> tiledPixmap;
    };

    QRect window;
    QRect viewport;
    std::vector<Data> datas;
};

class BufferedPaintProvider : public IPaintProvider
{
public:
    BufferedPaintProvider();

    QPaintDevice* device() const override;
    QPainter* qpainter() const override;

    bool isActive() const override;
    void beginTarget(const std::string& name) override;
    bool endTarget(const std::string& name, bool endDraw = false) override;

    void beginObject(const std::string& name, const QPointF& pagePos) override;
    void endObject(const std::string& name, const QPointF& pagePos) override;

    void setAntialiasing(bool arg) override;
    void setCompositionMode(CompositionMode mode) override;

    void setFont(const QFont& font) override;
    const QFont& font() const override;

    void setPen(const QPen& pen) override;
    void setNoPen() override;
    const QPen& pen() const override;

    void setBrush(const QBrush& brush) override;
    const QBrush& brush() const override;

    void save() override;
    void restore() override;

    void setTransform(const QTransform& transform) override;
    const QTransform& transform() const override;

    // drawing functions
    void drawPath(const QPainterPath& path) override;
    void drawPolygon(const QPointF* points, int pointCount, PolygonMode mode) override;

    void drawText(const QPointF& point, const QString& text) override;
    void drawText(const QRectF& rect, int flags, const QString& text) override;
    void drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun) override;

    void drawPixmap(const QPointF& p, const QPixmap& pm) override;
    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF()) override;

    // ---

    const DrawBuffer& buffer() const;
    void clear();

private:

    const DrawBuffer::State& currentState() const;
    DrawBuffer::State& editableState();
    DrawBuffer::Data& editableData();

    DrawBuffer m_buf;
    bool m_isCurDataEmpty = true;
    bool m_isActive = false;
    DrawObjectsLogger m_drawObjectsLogger;
};
}

#endif // MU_DRAW_BUFFEREDPAINTPROVIDER_H
