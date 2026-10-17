// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPainter>
#include <QPainterPathStroker>

namespace mu::notationscene::drawing {
// Used only by the explicit Fuse command. Convert painted ink to editable
// filled contours without changing the interactive renderer or source assets.
class PathCollector : public QPaintEngine
{
public:
    explicit PathCollector(const QTransform& placement)
        : QPaintEngine(AllFeatures), m_placement(placement) {}
    bool begin(QPaintDevice*) override { return true; }
    bool end() override { return true; }
    Type type() const override { return User; }
    void updateState(const QPaintEngineState&) override {}
    void drawPath(const QPainterPath& path) override { collect(path, true); }
    void drawPixmap(const QRectF&, const QPixmap&, const QRectF&) override { supported = false; }
    void drawImage(const QRectF&, const QImage&, const QRectF&, Qt::ImageConversionFlags) override { supported = false; }
    void drawTextItem(const QPointF& point, const QTextItem& item) override
    {
        QPainterPath path;
        path.addText(point, item.font(), item.text());
        appendMapped(path, state->pen().brush());
    }

    void drawRects(const QRectF* rects, int count) override
    {
        for (int i = 0; i < count; ++i) {
            QPainterPath path;
            path.addRect(rects[i]);
            collect(path, true);
        }
    }

    void drawRects(const QRect* rects, int count) override
    {
        for (int i = 0; i < count; ++i) {
            const QRectF rect(rects[i]);
            drawRects(&rect, 1);
        }
    }

    void drawEllipse(const QRectF& rect) override { QPainterPath path; path.addEllipse(rect); collect(path, true); }
    void drawEllipse(const QRect& rect) override { drawEllipse(QRectF(rect)); }
    void drawLines(const QLineF* lines, int count) override
    {
        for (int i = 0; i < count; ++i) {
            QPainterPath path;
            path.moveTo(lines[i].p1());
            path.lineTo(lines[i].p2());
            collect(path, false);
        }
    }

    void drawLines(const QLine* lines, int count) override
    {
        for (int i = 0; i < count; ++i) {
            const QLineF line(lines[i]);
            drawLines(&line, 1);
        }
    }

    void drawPolygon(const QPointF* points, int count, PolygonDrawMode mode) override
    {
        if (!count) {
            return;
        }
        QPainterPath path;
        path.moveTo(points[0]);
        for (int i = 1; i < count; ++i) {
            path.lineTo(points[i]);
        }
        if (mode != PolylineMode) {
            path.closeSubpath();
        }
        path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
        collect(path, mode != PolylineMode);
    }

    void drawPolygon(const QPoint* points, int count, PolygonDrawMode mode) override
    {
        QVector<QPointF> converted;
        converted.reserve(count);
        for (int i = 0; i < count; ++i) {
            converted.append(points[i]);
        }
        drawPolygon(converted.constData(), count, mode);
    }

    bool supported = true;
    QJsonArray paths;

private:
    void collect(const QPainterPath& path, bool fill)
    {
        if (fill) {
            appendMapped(path, state->brush());
        }
        const QPen pen = state->pen();
        if (pen.style() == Qt::NoPen) {
            return;
        }
        QPainterPathStroker stroker;
        stroker.setCapStyle(pen.capStyle());
        stroker.setJoinStyle(pen.joinStyle());
        stroker.setMiterLimit(pen.miterLimit());
        if (pen.style() == Qt::CustomDashLine) {
            stroker.setDashPattern(pen.dashPattern());
        } else if (pen.style() != Qt::SolidLine) {
            stroker.setDashPattern(pen.style());
        }
        stroker.setDashOffset(pen.dashOffset());
        if (pen.isCosmetic()) {
            stroker.setWidth(pen.widthF() > 0 ? pen.widthF() : 1.0);
            appendDevice(stroker.createStroke(state->transform().map(path)), pen.brush());
        } else {
            stroker.setWidth(pen.widthF());
            appendMapped(stroker.createStroke(path), pen.brush());
        }
    }

    void appendMapped(const QPainterPath& path, const QBrush& brush) { appendDevice(state->transform().map(path), brush); }
    void appendDevice(QPainterPath path, const QBrush& brush)
    {
        if (brush.style() == Qt::NoBrush) {
            return;
        }
        if (brush.style() != Qt::SolidPattern || state->compositionMode() != QPainter::CompositionMode_SourceOver) {
            supported = false;
            return;
        }
        if (brush.color().alpha() == 0) {
            return;
        }
        if (painter()->hasClipping()) {
            path = path.intersected(state->transform().map(painter()->clipPath()));
        }
        path = m_placement.map(QTransform::fromScale(1.0 / 1024, 1.0 / 1024).map(path));
        QJsonArray commands;
        for (int i = 0; i < path.elementCount(); ++i) {
            const auto element = path.elementAt(i);
            if (element.isMoveTo()) {
                commands.append(QJsonArray { "M", element.x, element.y });
            } else if (element.isLineTo()) {
                commands.append(QJsonArray { "L", element.x, element.y });
            } else if (element.isCurveTo() && i + 2 < path.elementCount()) {
                const auto control = path.elementAt(i + 1), end = path.elementAt(i + 2);
                commands.append(QJsonArray { "C", element.x, element.y, control.x, control.y, end.x, end.y });
                i += 2;
            }
            if (commands.size() > 100000) {
                supported = false;
                return;
            }
        }
        if (commands.isEmpty()) {
            return;
        }
        paths.append(QJsonObject { { "commands", commands }, { "fillRule", path.fillRule() == Qt::OddEvenFill ? "evenodd" : "nonzero" },
                         { "style", QJsonObject { { "stroke", "none" }, { "fill", brush.color().name() }, { "width", 0.001 },
                               { "opacity", state->opacity() * brush.color().alphaF() } } } });
    }

    QTransform m_placement;
};

class PathDevice : public QPaintDevice
{
public:
    explicit PathDevice(const QTransform& placement)
        : engine(placement) {}
    QPaintEngine* paintEngine() const override { return const_cast<PathCollector*>(&engine); }
    PathCollector engine;
protected:
    int metric(PaintDeviceMetric kind) const override
    {
        switch (kind) {
        case PdmWidth: case PdmHeight: return 1024;
        case PdmWidthMM: case PdmHeightMM: return 271;
        case PdmDpiX: case PdmDpiY: case PdmPhysicalDpiX: case PdmPhysicalDpiY: return 96;
        case PdmDepth: return 32;
        case PdmDevicePixelRatio: return 1;
        case PdmDevicePixelRatioScaled: return devicePixelRatioFScale();
        default: return 0;
        }
    }
};
}
