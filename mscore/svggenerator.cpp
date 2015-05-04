/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSvg module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "svggenerator.h"
#include "paintengine_p.h"

#if QT_POINTER_SIZE == 8 // 64-bit versions

static uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    quint64 t = (((quint64(x)) | ((quint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t += (((quint64(y)) | ((quint64(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
    t >>= 8;
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

static uint PREMUL(uint x) {
    uint a = x >> 24;
    quint64 t = (((quint64(x)) | ((quint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080) >> 8;
    t &= 0x000000ff00ff00ff;
    return (uint(t)) | (uint(t >> 24)) | (a << 24);
}

#else // 32-bit versions

static uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t >>= 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x &= 0xff00ff00;
    x |= t;
    return x;
}

#if defined(Q_CC_RVCT)
#  pragma push
#  pragma arm
#endif

#if defined(Q_CC_RVCT)
#  pragma pop
#endif

static uint PREMUL(uint x) {
    uint a = x >> 24;
    uint t = (x & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff) * a;
    x = (x + ((x >> 8) & 0xff) + 0x80);
    x &= 0xff00;
    x |= t | (a << 24);
    return x;
}
#endif

#define INV_PREMUL(p)                                   \
    (qAlpha(p) == 0 ? 0 :                               \
    ((qAlpha(p) << 24)                                  \
     | (((255*qRed(p))/ qAlpha(p)) << 16)               \
     | (((255*qGreen(p)) / qAlpha(p))  << 8)            \
     | ((255*qBlue(p)) / qAlpha(p))))


static void translate_color(const QColor &color, QString *color_string,
                            QString *opacity_string)
{
    Q_ASSERT(color_string);
    Q_ASSERT(opacity_string);

    *color_string =
        QString::fromLatin1("#%1%2%3")
        .arg(color.red(), 2, 16, QLatin1Char('0'))
        .arg(color.green(), 2, 16, QLatin1Char('0'))
        .arg(color.blue(), 2, 16, QLatin1Char('0'));
    *opacity_string = QString::number(color.alphaF());
}

static void translate_dashPattern(QVector<qreal> pattern, const qreal& width, QString *pattern_string)
{
    Q_ASSERT(pattern_string);

    // Note that SVG operates in absolute lengths, whereas Qt uses a length/width ratio.
    for (qreal entry : pattern)
        *pattern_string += QString::fromLatin1("%1,").arg(entry * width);

    pattern_string->chop(1);
}

class SvgPaintEnginePrivate : public QPaintEnginePrivate
{
public:
    SvgPaintEnginePrivate()
    {
        size = QSize();
        viewBox = QRectF();
        outputDevice = 0;
        resolution = 72;

        attributes.document_title = QLatin1String("Qt Svg Document");
        attributes.document_description = QLatin1String("Generated with Qt");
        attributes.font_family = QLatin1String("serif");
        attributes.font_size = QLatin1String("10pt");
        attributes.font_style = QLatin1String("normal");
        attributes.font_weight = QLatin1String("normal");

        afterFirstUpdate = false;
        numGradients = 0;
    }

    QSize size;
    QRectF viewBox;
    QIODevice *outputDevice;
    QTextStream *stream;
    int resolution;

    QString header;
    QString defs;
    QString body;
    bool    afterFirstUpdate;

    QBrush brush;
    QPen pen;
    QMatrix matrix;
    QFont font;

    QString generateGradientName() {
        ++numGradients;
        currentGradientName = QString::fromLatin1("gradient%1").arg(numGradients);
        return currentGradientName;
    }

    QString currentGradientName;
    int numGradients;

    struct _attributes {
        QString document_title;
        QString document_description;
        QString font_weight;
        QString font_size;
        QString font_family;
        QString font_style;
        QString stroke, strokeOpacity;
        QString dashPattern, dashOffset;
        QString fill, fillOpacity;
    } attributes;
};

static inline QPaintEngine::PaintEngineFeatures svgEngineFeatures()
{
    return QPaintEngine::PaintEngineFeatures(
        QPaintEngine::AllFeatures
        & ~QPaintEngine::PatternBrush
        & ~QPaintEngine::PerspectiveTransform
        & ~QPaintEngine::ConicalGradientFill
        & ~QPaintEngine::PorterDuff);
}

class SvgPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(SvgPaintEngine)
public:

    SvgPaintEngine()
        : QPaintEngine(*new SvgPaintEnginePrivate,
                       svgEngineFeatures())
    {
    }

    bool begin(QPaintDevice *device);
    bool end();

    void updateState(const QPaintEngineState &state);
    void popGroup();

    void drawPath(const QPainterPath &path);
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlag = Qt::AutoColor);

    QPaintEngine::Type type() const { return QPaintEngine::SVG; }

    QSize size() const { return d_func()->size; }
    void setSize(const QSize &size) {
        Q_ASSERT(!isActive());
        d_func()->size = size;
    }

    QRectF viewBox() const { return d_func()->viewBox; }
    void setViewBox(const QRectF &viewBox) {
        Q_ASSERT(!isActive());
        d_func()->viewBox = viewBox;
    }

    QString documentTitle() const { return d_func()->attributes.document_title; }
    void setDocumentTitle(const QString &title) {
        d_func()->attributes.document_title = title;
    }

    QString documentDescription() const { return d_func()->attributes.document_description; }
    void setDocumentDescription(const QString &description) {
        d_func()->attributes.document_description = description;
    }

    QIODevice *outputDevice() const { return d_func()->outputDevice; }
    void setOutputDevice(QIODevice *device) {
        Q_ASSERT(!isActive());
        d_func()->outputDevice = device;
    }

    int resolution() { return d_func()->resolution; }
    void setResolution(int resolution) {
        Q_ASSERT(!isActive());
        d_func()->resolution = resolution;
    }
    void saveLinearGradientBrush(const QGradient *g)
    {
        QTextStream str(&d_func()->defs, QIODevice::Append);
        const QLinearGradient *grad = static_cast<const QLinearGradient*>(g);
        str << QLatin1String("<linearGradient ");
        saveGradientUnits(str, g);
        if (grad) {
            str << QLatin1String("x1=\"") <<grad->start().x()<< QLatin1String("\" ")
                << QLatin1String("y1=\"") <<grad->start().y()<< QLatin1String("\" ")
                << QLatin1String("x2=\"") <<grad->finalStop().x() << QLatin1String("\" ")
                << QLatin1String("y2=\"") <<grad->finalStop().y() << QLatin1String("\" ");
        }

        str << QLatin1String("id=\"") << d_func()->generateGradientName() << QLatin1String("\">\n");
        saveGradientStops(str, g);
        str << QLatin1String("</linearGradient>") <<endl;
    }
    void saveRadialGradientBrush(const QGradient *g)
    {
        QTextStream str(&d_func()->defs, QIODevice::Append);
        const QRadialGradient *grad = static_cast<const QRadialGradient*>(g);
        str << QLatin1String("<radialGradient ");
        saveGradientUnits(str, g);
        if (grad) {
            str << QLatin1String("cx=\"") <<grad->center().x()<< QLatin1String("\" ")
                << QLatin1String("cy=\"") <<grad->center().y()<< QLatin1String("\" ")
                << QLatin1String("r=\"") <<grad->radius() << QLatin1String("\" ")
                << QLatin1String("fx=\"") <<grad->focalPoint().x() << QLatin1String("\" ")
                << QLatin1String("fy=\"") <<grad->focalPoint().y() << QLatin1String("\" ");
        }
        str << QLatin1String("xml:id=\"") <<d_func()->generateGradientName()<< QLatin1String("\">\n");
        saveGradientStops(str, g);
        str << QLatin1String("</radialGradient>") << endl;
    }
    void saveConicalGradientBrush(const QGradient *)
    {
        qWarning("svg's don't support conical gradients!");
    }

    void saveGradientStops(QTextStream &str, const QGradient *g) {
        QGradientStops stops = g->stops();

        if (g->interpolationMode() == QGradient::ColorInterpolation) {
            bool constantAlpha = true;
            int alpha = stops.at(0).second.alpha();
            for (int i = 1; i < stops.size(); ++i)
                constantAlpha &= (stops.at(i).second.alpha() == alpha);

            if (!constantAlpha) {
                const qreal spacing = qreal(0.02);
                QGradientStops newStops;
                QRgb fromColor = PREMUL(stops.at(0).second.rgba());
                QRgb toColor;
                for (int i = 0; i + 1 < stops.size(); ++i) {
                    int parts = qCeil((stops.at(i + 1).first - stops.at(i).first) / spacing);
                    newStops.append(stops.at(i));
                    toColor = PREMUL(stops.at(i + 1).second.rgba());

                    if (parts > 1) {
                        qreal step = (stops.at(i + 1).first - stops.at(i).first) / parts;
                        for (int j = 1; j < parts; ++j) {
                            QRgb color = INV_PREMUL(INTERPOLATE_PIXEL_256(fromColor, 256 - 256 * j / parts, toColor, 256 * j / parts));
                            newStops.append(QGradientStop(stops.at(i).first + j * step, QColor::fromRgba(color)));
                        }
                    }
                    fromColor = toColor;
                }
                newStops.append(stops.back());
                stops = newStops;
            }
        }

        for (QGradientStop stop : stops) {
            QString color =
                QString::fromLatin1("#%1%2%3")
                .arg(stop.second.red(), 2, 16, QLatin1Char('0'))
                .arg(stop.second.green(), 2, 16, QLatin1Char('0'))
                .arg(stop.second.blue(), 2, 16, QLatin1Char('0'));
            str << QLatin1String("    <stop offset=\"")<< stop.first << QLatin1String("\" ")
                << QLatin1String("stop-color=\"") << color << QLatin1String("\" ")
                << QLatin1String("stop-opacity=\"") << stop.second.alphaF() <<QLatin1String("\" />\n");
        }
    }

    void saveGradientUnits(QTextStream &str, const QGradient *gradient)
    {
        str << QLatin1String("gradientUnits=\"");
        if (gradient && gradient->coordinateMode() == QGradient::ObjectBoundingMode)
            str << QLatin1String("objectBoundingBox");
        else
            str << QLatin1String("userSpaceOnUse");
        str << QLatin1String("\" ");
    }

    void generateQtDefaults()
    {
        *d_func()->stream << QLatin1String("fill=\"none\" ");
        *d_func()->stream << QLatin1String("stroke=\"black\" ");
        *d_func()->stream << QLatin1String("stroke-width=\"1\" ");
        *d_func()->stream << QLatin1String("fill-rule=\"evenodd\" ");
        *d_func()->stream << QLatin1String("stroke-linecap=\"square\" ");
        *d_func()->stream << QLatin1String("stroke-linejoin=\"bevel\" ");
        *d_func()->stream << QLatin1String(">\n");
    }
    inline QTextStream &stream()
    {
        return *d_func()->stream;
    }


    void qpenToSvg(const QPen &spen)
    {
        QString width;

        d_func()->pen = spen;

        switch (spen.style()) {
        case Qt::NoPen:
            stream() << QLatin1String("stroke=\"none\" ");

            d_func()->attributes.stroke = QLatin1String("none");
            d_func()->attributes.strokeOpacity = QString();
            return;
            break;
        case Qt::SolidLine: {
            QString color, colorOpacity;

            translate_color(spen.color(), &color,
                            &colorOpacity);
            d_func()->attributes.stroke = color;
            d_func()->attributes.strokeOpacity = colorOpacity;

            stream() << QLatin1String("stroke=\"")<<color<< QLatin1String("\" ");
            stream() << QLatin1String("stroke-opacity=\"")<<colorOpacity<< QLatin1String("\" ");
        }
            break;
        case Qt::DashLine:
        case Qt::DotLine:
        case Qt::DashDotLine:
        case Qt::DashDotDotLine:
        case Qt::CustomDashLine: {
            QString color, colorOpacity, dashPattern, dashOffset;

            qreal penWidth = spen.width() == 0 ? qreal(1) : spen.widthF();

            translate_color(spen.color(), &color, &colorOpacity);
            translate_dashPattern(spen.dashPattern(), penWidth, &dashPattern);

            // SVG uses absolute offset
            dashOffset = QString::number(spen.dashOffset() * penWidth);

            d_func()->attributes.stroke = color;
            d_func()->attributes.strokeOpacity = colorOpacity;
            d_func()->attributes.dashPattern = dashPattern;
            d_func()->attributes.dashOffset = dashOffset;

            stream() << QLatin1String("stroke=\"")<<color<< QLatin1String("\" ");
            stream() << QLatin1String("stroke-opacity=\"")<<colorOpacity<< QLatin1String("\" ");
            stream() << QLatin1String("stroke-dasharray=\"")<<dashPattern<< QLatin1String("\" ");
            stream() << QLatin1String("stroke-dashoffset=\"")<<dashOffset<< QLatin1String("\" ");
            break;
        }
        default:
            qWarning("Unsupported pen style");
            break;
        }

        if (spen.widthF() == 0)
            stream() <<"stroke-width=\"1\" ";
        else
            stream() <<"stroke-width=\"" << spen.widthF() << "\" ";

        switch (spen.capStyle()) {
        case Qt::FlatCap:
            stream() << "stroke-linecap=\"butt\" ";
            break;
        case Qt::SquareCap:
            stream() << "stroke-linecap=\"square\" ";
            break;
        case Qt::RoundCap:
            stream() << "stroke-linecap=\"round\" ";
            break;
        default:
            qWarning("Unhandled cap style");
        }
        switch (spen.joinStyle()) {
        case Qt::MiterJoin:
            stream() << "stroke-linejoin=\"miter\" "
                        "stroke-miterlimit=\""<<spen.miterLimit()<<"\" ";
            break;
        case Qt::BevelJoin:
            stream() << "stroke-linejoin=\"bevel\" ";
            break;
        case Qt::RoundJoin:
            stream() << "stroke-linejoin=\"round\" ";
            break;
        case Qt::SvgMiterJoin:
            stream() << "stroke-linejoin=\"miter\" "
                        "stroke-miterlimit=\""<<spen.miterLimit()<<"\" ";
            break;
        default:
            qWarning("Unhandled join style");
        }
    }
    void qbrushToSvg(const QBrush &sbrush)
    {
        d_func()->brush = sbrush;
        switch (sbrush.style()) {
        case Qt::SolidPattern: {
            QString color, colorOpacity;
            translate_color(sbrush.color(), &color, &colorOpacity);
            stream() << "fill=\"" << color << "\" "
                        "fill-opacity=\""
                     << colorOpacity << "\" ";
            d_func()->attributes.fill = color;
            d_func()->attributes.fillOpacity = colorOpacity;
        }
            break;
        case Qt::LinearGradientPattern:
            saveLinearGradientBrush(sbrush.gradient());
            d_func()->attributes.fill = QString::fromLatin1("url(#%1)").arg(d_func()->currentGradientName);
            d_func()->attributes.fillOpacity = QString();
            stream() << QLatin1String("fill=\"url(#") << d_func()->currentGradientName << QLatin1String(")\" ");
            break;
        case Qt::RadialGradientPattern:
            saveRadialGradientBrush(sbrush.gradient());
            d_func()->attributes.fill = QString::fromLatin1("url(#%1)").arg(d_func()->currentGradientName);
            d_func()->attributes.fillOpacity = QString();
            stream() << QLatin1String("fill=\"url(#") << d_func()->currentGradientName << QLatin1String(")\" ");
            break;
        case Qt::ConicalGradientPattern:
            saveConicalGradientBrush(sbrush.gradient());
            d_func()->attributes.fill = QString::fromLatin1("url(#%1)").arg(d_func()->currentGradientName);
            d_func()->attributes.fillOpacity = QString();
            stream() << QLatin1String("fill=\"url(#") << d_func()->currentGradientName << QLatin1String(")\" ");
            break;
        case Qt::NoBrush:
            stream() << QLatin1String("fill=\"none\" ");
            d_func()->attributes.fill = QLatin1String("none");
            d_func()->attributes.fillOpacity = QString();
            return;
            break;
        default:
           break;
        }
    }
    void qfontToSvg(const QFont &sfont)
    {
        Q_D(SvgPaintEngine);

        d->font = sfont;

        if (d->font.pixelSize() == -1)
            d->attributes.font_size = QString::number(d->font.pointSizeF() * d->resolution / 72);
        else
            d->attributes.font_size = QString::number(d->font.pixelSize());

        int svgWeight = d->font.weight();
        switch (svgWeight) {
        case QFont::Light:
            svgWeight = 100;
            break;
        case QFont::Normal:
            svgWeight = 400;
            break;
        case QFont::Bold:
            svgWeight = 700;
            break;
        default:
            svgWeight *= 10;
        }

        d->attributes.font_weight = QString::number(svgWeight);
        d->attributes.font_family = d->font.family();
        d->attributes.font_style = d->font.italic() ? QLatin1String("italic") : QLatin1String("normal");

        *d->stream << "font-family=\"" << d->attributes.font_family << "\" "
                      "font-size=\"" << d->attributes.font_size << "\" "
                      "font-weight=\"" << d->attributes.font_weight << "\" "
                      "font-style=\"" << d->attributes.font_style << "\" "
                   << endl;
    }
};

class SvgGeneratorPrivate
{
public:
    SvgPaintEngine *engine;

    uint owns_iodevice : 1;
    QString fileName;
};

/*!
    \class SvgGenerator
    \ingroup painting
    \since 4.3
    \brief The SvgGenerator class provides a paint device that is used to create SVG drawings.
    \reentrant

    This paint device represents a Scalable Vector Graphics (SVG) drawing. Like QPrinter, it is
    designed as a write-only device that generates output in a specific format.

    To write an SVG file, you first need to configure the output by setting the \l fileName
    or \l outputDevice properties. It is usually necessary to specify the size of the drawing
    by setting the \l size property, and in some cases where the drawing will be included in
    another, the \l viewBox property also needs to be set.

    \snippet examples/painting/svggenerator/window.cpp configure SVG generator

    Other meta-data can be specified by setting the \a title, \a description and \a resolution
    properties.

    As with other QPaintDevice subclasses, a QPainter object is used to paint onto an instance
    of this class:

    \snippet examples/painting/svggenerator/window.cpp begin painting
    \dots
    \snippet examples/painting/svggenerator/window.cpp end painting

    Painting is performed in the same way as for any other paint device. However,
    it is necessary to use the QPainter::begin() and \l{QPainter::}{end()} to
    explicitly begin and end painting on the device.

    The \l{SVG Generator Example} shows how the same painting commands can be used
    for painting a widget and writing an SVG file.

    \sa SvgRenderer, SvgWidget, {About SVG}
*/

/*!
    Constructs a new generator.
*/
SvgGenerator::SvgGenerator()
    : d_ptr(new SvgGeneratorPrivate)
{
    Q_D(SvgGenerator);

    d->engine = new SvgPaintEngine;
    d->owns_iodevice = false;
}

/*!
    Destroys the generator.
*/
SvgGenerator::~SvgGenerator()
{
    Q_D(SvgGenerator);
    if (d->owns_iodevice)
        delete d->engine->outputDevice();
    delete d->engine;
}

/*!
    \property SvgGenerator::title
    \brief the title of the generated SVG drawing
    \since 4.5
    \sa description
*/
QString SvgGenerator::title() const
{
    Q_D(const SvgGenerator);

    return d->engine->documentTitle();
}

void SvgGenerator::setTitle(const QString &title)
{
    Q_D(SvgGenerator);

    d->engine->setDocumentTitle(title);
}

/*!
    \property SvgGenerator::description
    \brief the description of the generated SVG drawing
    \since 4.5
    \sa title
*/
QString SvgGenerator::description() const
{
    Q_D(const SvgGenerator);

    return d->engine->documentDescription();
}

void SvgGenerator::setDescription(const QString &description)
{
    Q_D(SvgGenerator);

    d->engine->setDocumentDescription(description);
}

/*!
    \property SvgGenerator::size
    \brief the size of the generated SVG drawing
    \since 4.5

    By default this property is set to \c{QSize(-1, -1)}, which
    indicates that the generator should not output the width and
    height attributes of the \c<svg> element.

    \note It is not possible to change this property while a
    QPainter is active on the generator.

    \sa viewBox, resolution
*/
QSize SvgGenerator::size() const
{
    Q_D(const SvgGenerator);
    return d->engine->size();
}

void SvgGenerator::setSize(const QSize &size)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setSize(), cannot set size while SVG is being generated");
        return;
    }
    d->engine->setSize(size);
}

/*!
    \property SvgGenerator::viewBox
    \brief the viewBox of the generated SVG drawing
    \since 4.5

    By default this property is set to \c{QRect(0, 0, -1, -1)}, which
    indicates that the generator should not output the viewBox attribute
    of the \c<svg> element.

    \note It is not possible to change this property while a
    QPainter is active on the generator.

    \sa viewBox(), size, resolution
*/
QRectF SvgGenerator::viewBoxF() const
{
    Q_D(const SvgGenerator);
    return d->engine->viewBox();
}

/*!
    \since 4.5

    Returns viewBoxF().toRect().

    \sa viewBoxF()
*/
QRect SvgGenerator::viewBox() const
{
    Q_D(const SvgGenerator);
    return d->engine->viewBox().toRect();
}

void SvgGenerator::setViewBox(const QRectF &viewBox)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setViewBox(), cannot set viewBox while SVG is being generated");
        return;
    }
    d->engine->setViewBox(viewBox);
}

void SvgGenerator::setViewBox(const QRect &viewBox)
{
    setViewBox(QRectF(viewBox));
}

/*!
    \property SvgGenerator::fileName
    \brief the target filename for the generated SVG drawing
    \since 4.5

    \sa outputDevice
*/
QString SvgGenerator::fileName() const
{
    Q_D(const SvgGenerator);
    return d->fileName;
}

void SvgGenerator::setFileName(const QString &fileName)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setFileName(), cannot set file name while SVG is being generated");
        return;
    }

    if (d->owns_iodevice)
        delete d->engine->outputDevice();

    d->owns_iodevice = true;

    d->fileName = fileName;
    QFile *file = new QFile(fileName);
    d->engine->setOutputDevice(file);
}

/*!
    \property SvgGenerator::outputDevice
    \brief the output device for the generated SVG drawing
    \since 4.5

    If both output device and file name are specified, the output device
    will have precedence.

    \sa fileName
*/
QIODevice *SvgGenerator::outputDevice() const
{
    Q_D(const SvgGenerator);
    return d->engine->outputDevice();
}

void SvgGenerator::setOutputDevice(QIODevice *outputDevice)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setOutputDevice(), cannot set output device while SVG is being generated");
        return;
    }
    d->owns_iodevice = false;
    d->engine->setOutputDevice(outputDevice);
    d->fileName = QString();
}

/*!
    \property SvgGenerator::resolution
    \brief the resolution of the generated output
    \since 4.5

    The resolution is specified in dots per inch, and is used to
    calculate the physical size of an SVG drawing.

    \sa size, viewBox
*/
int SvgGenerator::resolution() const
{
    Q_D(const SvgGenerator);
    return d->engine->resolution();
}

void SvgGenerator::setResolution(int dpi)
{
    Q_D(SvgGenerator);
    d->engine->setResolution(dpi);
}

/*!
    Returns the paint engine used to render graphics to be converted to SVG
    format information.
*/
QPaintEngine *SvgGenerator::paintEngine() const
{
    Q_D(const SvgGenerator);
    return d->engine;
}

/*!
    \reimp
*/
int SvgGenerator::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    Q_D(const SvgGenerator);
    switch (metric) {
    case QPaintDevice::PdmDepth:
        return 32;
    case QPaintDevice::PdmWidth:
        return d->engine->size().width();
    case QPaintDevice::PdmHeight:
        return d->engine->size().height();
    case QPaintDevice::PdmDpiX:
        return d->engine->resolution();
    case QPaintDevice::PdmDpiY:
        return d->engine->resolution();
    case QPaintDevice::PdmHeightMM:
        return qRound(d->engine->size().height() * 25.4 / d->engine->resolution());
    case QPaintDevice::PdmWidthMM:
        return qRound(d->engine->size().width() * 25.4 / d->engine->resolution());
    case QPaintDevice::PdmNumColors:
        return 0xffffffff;
    case QPaintDevice::PdmPhysicalDpiX:
        return d->engine->resolution();
    case QPaintDevice::PdmPhysicalDpiY:
        return d->engine->resolution();
    default:
        qWarning("SvgGenerator::metric(), unhandled metric %d\n", metric);
        break;
    }
    return 0;
}

/*****************************************************************************
 * class SvgPaintEngine
 */

bool SvgPaintEngine::begin(QPaintDevice *)
{
    Q_D(SvgPaintEngine);
    if (!d->outputDevice) {
        qWarning("SvgPaintEngine::begin(), no output device");
        return false;
    }

    if (!d->outputDevice->isOpen()) {
        if (!d->outputDevice->open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("SvgPaintEngine::begin(), could not open output device: '%s'",
                     qPrintable(d->outputDevice->errorString()));
            return false;
        }
    } else if (!d->outputDevice->isWritable()) {
        qWarning("SvgPaintEngine::begin(), could not write to read-only output device: '%s'",
                 qPrintable(d->outputDevice->errorString()));
        return false;
    }

    d->stream = new QTextStream(&d->header);

    // stream out the header...
    *d->stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl << "<svg";

    if (d->size.isValid()) {
        qreal wmm = d->size.width() * 25.4 / d->resolution;
        qreal hmm = d->size.height() * 25.4 / d->resolution;
        *d->stream << " width=\"" << wmm << "mm\" height=\"" << hmm << "mm\"" << endl;
    }

    if (d->viewBox.isValid()) {
        *d->stream << " viewBox=\"" << d->viewBox.left() << ' ' << d->viewBox.top();
        *d->stream << ' ' << d->viewBox.width() << ' ' << d->viewBox.height() << '\"' << endl;
    }

    *d->stream << " xmlns=\"http://www.w3.org/2000/svg\""
                  " xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                  " version=\"1.2\" baseProfile=\"tiny\">" << endl;

    if (!d->attributes.document_title.isEmpty()) {
        *d->stream << "<title>" << d->attributes.document_title << "</title>" << endl;
    }

    if (!d->attributes.document_description.isEmpty()) {
        *d->stream << "<desc>" << d->attributes.document_description << "</desc>" << endl;
    }

    d->stream->setString(&d->defs);
    *d->stream << "<defs>\n";

    d->stream->setString(&d->body);
    // Start the initial graphics state...
    *d->stream << "<g ";
    generateQtDefaults();
    *d->stream << endl;

    return true;
}

bool SvgPaintEngine::end()
{
    Q_D(SvgPaintEngine);

    d->stream->setString(&d->defs);
    *d->stream << "</defs>\n";

    d->stream->setDevice(d->outputDevice);
#ifndef QT_NO_TEXTCODEC
    d->stream->setCodec(QTextCodec::codecForName("UTF-8"));
#endif

    *d->stream << d->header;
    *d->stream << d->defs;
    *d->stream << d->body;
    if (d->afterFirstUpdate)
        *d->stream << "</g>" << endl; // close the updateState

    *d->stream << "</g>" << endl // close the Qt defaults
               << "</svg>" << endl;

    delete d->stream;

    return true;
}

void SvgPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm,
                                 const QRectF &sr)
{
    drawImage(r, pm.toImage(), sr);
}

void SvgPaintEngine::drawImage(const QRectF &r, const QImage &image,
                                const QRectF &sr,
                                Qt::ImageConversionFlag flags)
{
    //Q_D(SvgPaintEngine);

    Q_UNUSED(sr);
    Q_UNUSED(flags);
    stream() << "<image ";
    stream() << "x=\""<<r.x()<<"\" "
                "y=\""<<r.y()<<"\" "
                "width=\""<<r.width()<<"\" "
                "height=\""<<r.height()<<"\" "
                "preserveAspectRatio=\"none\" ";

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QBuffer::ReadWrite);
    image.save(&buffer, "PNG");
    buffer.close();
    stream() << "xlink:href=\"data:image/png;base64,"
             << data.toBase64()
             <<"\" />\n";
}

void SvgPaintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(SvgPaintEngine);
    QPaintEngine::DirtyFlags flags = state.state();

    // always stream full gstate, which is not required, but...
    flags |= QPaintEngine::AllDirty;

    // close old state and start a new one...
    if (d->afterFirstUpdate)
        *d->stream << "</g>\n\n";

    *d->stream << "<g ";

    if (flags & QPaintEngine::DirtyBrush) {
        qbrushToSvg(state.brush());
    }

    if (flags & QPaintEngine::DirtyPen) {
        qpenToSvg(state.pen());
    }

    if (flags & QPaintEngine::DirtyTransform) {
        d->matrix = state.matrix();
        *d->stream << "transform=\"matrix(" << d->matrix.m11() << ','
                   << d->matrix.m12() << ','
                   << d->matrix.m21() << ',' << d->matrix.m22() << ','
                   << d->matrix.dx() << ',' << d->matrix.dy()
                   << ")\""
                   << endl;
    }

    if (flags & QPaintEngine::DirtyFont) {
        qfontToSvg(state.font());
    }

    if (flags & QPaintEngine::DirtyOpacity) {
        if (!qFuzzyIsNull(state.opacity() - 1))
            stream() << "opacity=\""<<state.opacity()<<"\" ";
    }

    *d->stream << '>' << endl;

    d->afterFirstUpdate = true;
}

void SvgPaintEngine::drawPath(const QPainterPath &p)
{
    Q_D(SvgPaintEngine);

    *d->stream << "<path vector-effect=\""
               << (state->pen().isCosmetic() ? "non-scaling-stroke" : "none")
               << "\" fill-rule=\""
               << (p.fillRule() == Qt::OddEvenFill ? "evenodd" : "nonzero")
               << "\" d=\"";

    for (int i=0; i<p.elementCount(); ++i) {
        const QPainterPath::Element &e = p.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            *d->stream << 'M' << e.x << ',' << e.y;
            break;
        case QPainterPath::LineToElement:
            *d->stream << 'L' << e.x << ',' << e.y;
            break;
        case QPainterPath::CurveToElement:
            *d->stream << 'C' << e.x << ',' << e.y;
            ++i;
            while (i < p.elementCount()) {
                const QPainterPath::Element &e = p.elementAt(i);
                if (e.type != QPainterPath::CurveToDataElement) {
                    --i;
                    break;
                } else
                    *d->stream << ' ';
                *d->stream << e.x << ',' << e.y;
                ++i;
            }
            break;
        default:
            break;
        }
        if (i != p.elementCount() - 1) {
            *d->stream << ' ';
        }
    }

    *d->stream << "\"/>" << endl;
}

void SvgPaintEngine::drawPolygon(const QPointF *points, int pointCount,
                                  PolygonDrawMode mode)
{
    Q_ASSERT(pointCount >= 2);

    //Q_D(SvgPaintEngine);

    QPainterPath path(points[0]);
    for (int i=1; i<pointCount; ++i)
        path.lineTo(points[i]);

    if (mode == PolylineMode) {
        stream() << "<polyline fill=\"none\" vector-effect=\""
                 << (state->pen().isCosmetic() ? "non-scaling-stroke" : "none")
                 << "\" points=\"";
        for (int i = 0; i < pointCount; ++i) {
            const QPointF &pt = points[i];
            stream() << pt.x() << ',' << pt.y() << ' ';
        }
        stream() << "\" />" <<endl;
    } else {
        path.closeSubpath();
        drawPath(path);
    }
}
