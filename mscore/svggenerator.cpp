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
#include "libmscore/mscore.h"

///////////////////////////////////////////////////////////////////////////////
// FOR GRADIENT FUNCTIONALITY THAT IS NOT IMPLEMENTED (YET):
//
//#if QT_POINTER_SIZE == 8 // 64-bit versions
//
//static uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
//    quint64 t = (((quint64(x)) | ((quint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
//    t += (((quint64(y)) | ((quint64(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
//    t >>= 8;
//    t &= 0x00ff00ff00ff00ff;
//    return (uint(t)) | (uint(t >> 24));
//}
//
//static uint PREMUL(uint x) {
//    uint a = x >> 24;
//    quint64 t = (((quint64(x)) | ((quint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
//    t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080) >> 8;
//    t &= 0x000000ff00ff00ff;
//    return (uint(t)) | (uint(t >> 24)) | (a << 24);
//}
//
//#else // 32-bit versions
//
//static uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
//    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
//    t >>= 8;
//    t &= 0xff00ff;
//
//    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
//    x &= 0xff00ff00;
//    x |= t;
//    return x;
//}
//
//#if defined(Q_CC_RVCT)
//#  pragma push
//#  pragma arm
//#endif
//
//#if defined(Q_CC_RVCT)
//#  pragma pop
//#endif
//
//static uint PREMUL(uint x) {
//    uint a = x >> 24;
//    uint t = (x & 0xff00ff) * a;
//    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
//    t &= 0xff00ff;
//
//    x = ((x >> 8) & 0xff) * a;
//    x = (x + ((x >> 8) & 0xff) + 0x80);
//    x &= 0xff00;
//    x |= t | (a << 24);
//    return x;
//}
//#endif
//
/*#define INV_PREMUL(p)                                   \
    (qAlpha(p) == 0 ? 0 :                               \
    ((qAlpha(p) << 24)                                  \
     | (((255*qRed(p))/ qAlpha(p)) << 16)               \
     | (((255*qGreen(p)) / qAlpha(p))  << 8)            \
     | ((255*qBlue(p)) / qAlpha(p))))
*/
///////////////////////////////////////////////////////////////////////////////


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
    foreach (qreal entry, pattern)
        *pattern_string += QString::fromLatin1("%1,").arg(entry * width);

    pattern_string->chop(1);
}

// Gets the contents of the SVG class attribute, based on element type/name
static QString getClass(const Ms::Element *e)
{
    Ms::ElementType eType;
              QString eName;

    // Add element type as "class"
    if (e == NULL)
        return eName; // e should never be null, this is extra-cautious

    eType = e->type();
    eName = e->name(eType);

    // Future sub-typing code goes here

    return eName;
}

class SvgPaintEnginePrivate
{
public:
    SvgPaintEnginePrivate()
    {
        size = QSize();
        viewBox = QRectF();
        outputDevice = 0;
        resolution = Ms::DPI;

        attributes.title = QLatin1String("MuseScore SVG Document");
        attributes.description = QString("Generated by MuseScore %1").arg(VERSION);
// UNUSED
//        attributes.font_family = QLatin1String("serif");
//        attributes.font_size = QLatin1String("10pt");
//        attributes.font_style = QLatin1String("normal");
//        attributes.font_weight = QLatin1String("normal");
//
//        numGradients = 0;
    }

    QSize size;
    QRectF viewBox;
    QIODevice *outputDevice;
    QTextStream *stream;
    int resolution;

    QString header;
//    QString defs; // NEEDED FOR GRADIENTS
    QString body;

    QBrush brush;
    QPen pen;
    QMatrix matrix;
//    QFont font;  // UNUSED

// GRADIENTS NOT IMPLEMENTED (YET)
//    QString generateGradientName() {
//        ++numGradients;
//        currentGradientName = QString::fromLatin1("gradient%1").arg(numGradients);
//        return currentGradientName;
//    }
//
//    QString currentGradientName;
//    int numGradients;

    struct _attributes {
        QString title;
        QString description;
// UNUSED STRUCT MEMBERS:
//        QString font_weight;
//        QString font_size;
//        QString font_family;
//        QString font_style;
//        QString stroke, strokeOpacity;
//        QString dashPattern, dashOffset;
//        QString fill, fillOpacity;
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
    friend class SvgGenerator; // for setElement()

    Q_DECLARE_PRIVATE(SvgPaintEngine)

private:
    QString     stateString;
    QTextStream stateStream;
    SvgPaintEnginePrivate *d_ptr;

// Qt translates everything. These help avoid SVG transform="translate()".
    qreal _dx;
    qreal _dy;

protected:
// The Ms::Element being generated right now
    const Ms::Element* _element = NULL;

// SVG strings as constants
#define SVG_SPACE    ' '
#define SVG_QUOTE    "\""
#define SVG_COMMA    ","
#define SVG_GT       ">"
#define SVG_PX       "px"
#define SVG_NONE     "none"
#define SVG_EVENODD  "evenodd"
#define SVG_BUTT     "butt"
#define SVG_SQUARE   "square"
#define SVG_ROUND    "round"
#define SVG_MITER    "miter"
#define SVG_BEVEL    "bevel"
#define SVG_ONE      "1"
#define SVG_BLACK    "#000000"

#define SVG_BEGIN    "<svg"
#define SVG_END      "</svg>"

#define SVG_WIDTH    " width=\""
#define SVG_HEIGHT   " height=\""
#define SVG_VIEW_BOX " viewBox=\""

#define SVG_X        " x="
#define SVG_Y        " y="

#define SVG_D_M      " d=\"M"
#define SVG_D        " d=\""
#define SVG_MOVE     'M'
#define SVG_LINE     'L'
#define SVG_CURVE    'C'

#define SVG_CLASS    " class=\""

#define SVG_ELEMENT_END  "/>"
#define SVG_RPAREN_QUOTE ")\""

#define SVG_TITLE_BEGIN "<title>"
#define SVG_TITLE_END   "</title>"
#define SVG_DESC_BEGIN  "<desc>"
#define SVG_DESC_END    "</desc>"

#define SVG_IMAGE       "<image"
#define SVG_PATH        "<path"

#define SVG_PRESERVE_ASPECT " preserveAspectRatio=\""

#define SVG_FILL            " fill=\""
#define SVG_STROKE          " stroke=\""
#define SVG_STROKE_WIDTH    " stroke-width=\""
#define SVG_STROKE_LINECAP  " stroke-linecap=\""
#define SVG_STROKE_LINEJOIN " stroke-linejoin=\""
#define SVG_STROKE_DASHARRAY " stroke-dasharray=\""
#define SVG_STROKE_DASHOFFSET " stroke-dashoffset=\""
#define SVG_STROKE_MITERLIMIT " stroke-miterlimit=\""

#define SVG_OPACITY         " opacity=\""
#define SVG_FILL_OPACITY    " fill-opacity=\""
#define SVG_STROKE_OPACITY  " stroke-opacity=\""

#define SVG_FONT_FAMILY     " font-family=\""
#define SVG_FONT_SIZE       " font-size=\""

#define SVG_FILL_RULE       " fill-rule=\"evenodd\""
#define SVG_VECTOR_EFFECT   " vector-effect=\"non-scaling-stroke\""

#define SVG_MATRIX    " transform=\"matrix("

public:
    SvgPaintEngine()
        : QPaintEngine(svgEngineFeatures()),
          stateStream(&stateString)
    {
        d_ptr = new SvgPaintEnginePrivate;
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

    QString documentTitle() const { return d_func()->attributes.title; }
    void setDocumentTitle(const QString &title) {
        d_func()->attributes.title = title;
    }

    QString documentDescription() const { return d_func()->attributes.description; }
    void setDocumentDescription(const QString &description) {
        d_func()->attributes.description = description;
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

///////////////////////////////////////////////////////////////////////////////
// UNUSED GRADIENT CODE:
//    void saveLinearGradientBrush(const QGradient *g)
//    {
//        QTextStream str(&d_func()->defs, QIODevice::Append);
//        const QLinearGradient *grad = static_cast<const QLinearGradient*>(g);
//        str << QLatin1String("<linearGradient ");
//        saveGradientUnits(str, g);
//        if (grad) {
//            str << QLatin1String("x1=\"") <<grad->start().x()<< QLatin1String("\" ")
//                << QLatin1String("y1=\"") <<grad->start().y()<< QLatin1String("\" ")
//                << QLatin1String("x2=\"") <<grad->finalStop().x() << QLatin1String("\" ")
//                << QLatin1String("y2=\"") <<grad->finalStop().y() << QLatin1String("\" ");
//        }
//
//        str << QLatin1String("id=\"") << d_func()->generateGradientName() << QLatin1String("\">\n");
//        saveGradientStops(str, g);
//        str << QLatin1String("</linearGradient>") <<endl;
//    }
//    void saveRadialGradientBrush(const QGradient *g)
//    {
//        QTextStream str(&d_func()->defs, QIODevice::Append);
//        const QRadialGradient *grad = static_cast<const QRadialGradient*>(g);
//        str << QLatin1String("<radialGradient ");
//        saveGradientUnits(str, g);
//        if (grad) {
//            str << QLatin1String("cx=\"") <<grad->center().x()<< QLatin1String("\" ")
//                << QLatin1String("cy=\"") <<grad->center().y()<< QLatin1String("\" ")
//                << QLatin1String("r=\"") <<grad->radius() << QLatin1String("\" ")
//                << QLatin1String("fx=\"") <<grad->focalPoint().x() << QLatin1String("\" ")
//                << QLatin1String("fy=\"") <<grad->focalPoint().y() << QLatin1String("\" ");
//        }
//        str << QLatin1String("xml:id=\"") <<d_func()->generateGradientName()<< QLatin1String("\">\n");
//        saveGradientStops(str, g);
//        str << QLatin1String("</radialGradient>") << endl;
//    }
//    void saveConicalGradientBrush(const QGradient *)
//    {
//        qWarning("svg's don't support conical gradients!");
//    }
//
//    void saveGradientStops(QTextStream &str, const QGradient *g) {
//        QGradientStops stops = g->stops();
//
//        if (g->interpolationMode() == QGradient::ColorInterpolation) {
//            bool constantAlpha = true;
//            int alpha = stops.at(0).second.alpha();
//            for (int i = 1; i < stops.size(); ++i)
//                constantAlpha &= (stops.at(i).second.alpha() == alpha);
//
//            if (!constantAlpha) {
//                const qreal spacing = qreal(0.02);
//                QGradientStops newStops;
//                QRgb fromColor = PREMUL(stops.at(0).second.rgba());
//                QRgb toColor;
//                for (int i = 0; i + 1 < stops.size(); ++i) {
//                    int parts = qCeil((stops.at(i + 1).first - stops.at(i).first) / spacing);
//                    newStops.append(stops.at(i));
//                    toColor = PREMUL(stops.at(i + 1).second.rgba());
//
//                    if (parts > 1) {
//                        qreal step = (stops.at(i + 1).first - stops.at(i).first) / parts;
//                        for (int j = 1; j < parts; ++j) {
//                            QRgb color = INV_PREMUL(INTERPOLATE_PIXEL_256(fromColor, 256 - 256 * j / parts, toColor, 256 * j / parts));
//                            newStops.append(QGradientStop(stops.at(i).first + j * step, QColor::fromRgba(color)));
//                        }
//                    }
//                    fromColor = toColor;
//                }
//                newStops.append(stops.back());
//                stops = newStops;
//            }
//        }
//
//        foreach(QGradientStop stop, stops) {
//            QString color =
//                QString::fromLatin1("#%1%2%3")
//                .arg(stop.second.red(), 2, 16, QLatin1Char('0'))
//                .arg(stop.second.green(), 2, 16, QLatin1Char('0'))
//                .arg(stop.second.blue(), 2, 16, QLatin1Char('0'));
//            str << QLatin1String("    <stop offset=\"")<< stop.first << QLatin1String("\" ")
//                << QLatin1String("stop-color=\"") << color << QLatin1String("\" ")
//                << QLatin1String("stop-opacity=\"") << stop.second.alphaF() <<QLatin1String("\" />\n");
//        }
//    }
//
//    void saveGradientUnits(QTextStream &str, const QGradient *gradient)
//    {
//        str << QLatin1String("gradientUnits=\"");
//        if (gradient && gradient->coordinateMode() == QGradient::ObjectBoundingMode)
//            str << QLatin1String("objectBoundingBox");
//        else
//            str << QLatin1String("userSpaceOnUse");
//        str << QLatin1String("\" ");
//    }
// END UNUSED GRADIENT CODE
///////////////////////////////////////////////////////////////////////////////

    inline QTextStream &stream()
    {
        return *d_func()->stream;
    }

    //////////////////////////////
    // SvgPaintEngine::qpenToSVG()
    //////////////////////////////
    const QString qpenToSvg(const QPen &spen)
    {
        QString     qs;
        QTextStream qts(&qs);

        QString color, colorOpacity;

        // Set stroke, stroke-dasharray, stroke-dashoffset attributes
        switch (spen.style()) {
        case Qt::NoPen:
            return qs; // Default value for stroke = "none" = Qt::NoPen = NOOP;
            break;

        case Qt::SolidLine:
        case Qt::DashLine:
        case Qt::DotLine:
        case Qt::DashDotLine:
        case Qt::DashDotDotLine:
        case Qt::CustomDashLine: {
            // These values are class variables because they are needed by
            // drawTextItem(). This is the fill color/opacity for text.
            translate_color(spen.color(), &color, &colorOpacity);

            // default stroke="none" is handled by case Qt::NoPen above
            qts << SVG_STROKE << color << SVG_QUOTE;

            // stroke-opacity is seldom used, usually set to default 1
            if (colorOpacity != SVG_ONE)
                qts << SVG_STROKE_OPACITY << colorOpacity << SVG_QUOTE;

            // If it's a solid line, were done for now
            if (spen.style() == Qt::SolidLine)
                break;

            // It's a dashed line
            qreal penWidth = spen.width() == 0 ? qreal(1) : spen.widthF();

            QString dashPattern, dashOffset;
            translate_dashPattern(spen.dashPattern(), penWidth, &dashPattern);
            dashOffset = QString::number(spen.dashOffset() * penWidth); // SVG uses absolute offset

            qts << SVG_STROKE_DASHARRAY  << dashPattern << SVG_QUOTE;
            qts << SVG_STROKE_DASHOFFSET << dashOffset  << SVG_QUOTE;
            break; }
        default:
            qWarning("Unsupported pen style");
            break;
        }
        // Set stroke-width attribute, unless it's zero or 1 (default is 1)
        if (spen.widthF() > 0 && spen.widthF() != 1) {
            // Formatting for vertical alignment of elements
            qts.setRealNumberPrecision(2); // with DPI=72 only 2 decimals necessary
            qts.setRealNumberNotation(QTextStream::FixedNotation);
            qts << SVG_STROKE_WIDTH << spen.widthF() << SVG_QUOTE;
            qts.setRealNumberNotation(QTextStream::SmartNotation);
        }
        // Set stroke-linecap attribute
        switch (spen.capStyle()) {
        case Qt::FlatCap:
            // This is the default stroke-linecap value
            //qts << SVG_STROKE_LINECAP << SVG_BUTT << SVG_QUOTE;
            break;
        case Qt::SquareCap:
            qts << SVG_STROKE_LINECAP << SVG_SQUARE << SVG_QUOTE;
            break;
        case Qt::RoundCap:
            qts << SVG_STROKE_LINECAP << SVG_ROUND << SVG_QUOTE;
            break;
        default:
            qWarning("Unhandled cap style");
            break;
        }
        // Set stroke-linejoin, stroke-miterlimit attributes
        switch (spen.joinStyle()) {
        case Qt::MiterJoin:
        case Qt::SvgMiterJoin:
            qts << SVG_STROKE_LINEJOIN   << SVG_MITER         << SVG_QUOTE
                << SVG_STROKE_MITERLIMIT << spen.miterLimit() << SVG_QUOTE;
            break;
        case Qt::BevelJoin:
            qts << SVG_STROKE_LINEJOIN   << SVG_BEVEL << SVG_QUOTE;
            break;
        case Qt::RoundJoin:
            qts << SVG_STROKE_LINEJOIN   << SVG_ROUND << SVG_QUOTE;
            break;
        default:
            qWarning("Unhandled join style");
            break;
        }
        // An uncommon, possibly non-existent in MuseScore, effect
        if (spen.isCosmetic())
            qts << SVG_VECTOR_EFFECT;

        return qs;
    }

    /////////////////////////////////
    // SvgPaintEngine::qbrushToSVG()
    /////////////////////////////////
    const QString  qbrushToSvg(const QBrush &sbrush)
    {
        QString     qs;
        QTextStream qts(&qs);

        QString color, colorOpacity;

        switch (sbrush.style()) {
        case Qt::SolidPattern:
            translate_color(sbrush.color(), &color, &colorOpacity);

            // Default fill color is black
            if (color != SVG_BLACK)
                qts << SVG_FILL << color << SVG_QUOTE;

            // Default fill-opacity is 100%
            if (colorOpacity != SVG_ONE)
                qts << SVG_FILL_OPACITY << colorOpacity << SVG_QUOTE;

            break;

        case Qt::NoBrush:
            qts << SVG_FILL << SVG_NONE <<  SVG_QUOTE;
            break;

///////////////////////////////////////////////////////////////////////////////
// OLD GRADIENT CODE: MuseScore does not support gradients (yet)
//
//      case Qt::LinearGradientPattern:
//          saveLinearGradientBrush(sbrush.gradient());
//          d_func()->attributes.fill = QString::fromLatin1("url(#%1)").arg(d_func()->currentGradientName);
//          d_func()->attributes.fillOpacity = QString();
//          stateStream << QLatin1String("fill=\"url(#") << d_func()->currentGradientName << QLatin1String(")\" ");
//          break;
//      case Qt::RadialGradientPattern:
//          saveRadialGradientBrush(sbrush.gradient());
//          d_func()->attributes.fill = QString::fromLatin1("url(#%1)").arg(d_func()->currentGradientName);
//          d_func()->attributes.fillOpacity = QString();
//          stateStream << QLatin1String("fill=\"url(#") << d_func()->currentGradientName << QLatin1String(")\" ");
//          break;
//      case Qt::ConicalGradientPattern:
//          saveConicalGradientBrush(sbrush.gradient());
//          d_func()->attributes.fill = QString::fromLatin1("url(#%1)").arg(d_func()->currentGradientName);
//          d_func()->attributes.fillOpacity = QString();
//          stateStream << QLatin1String("fill=\"url(#") << d_func()->currentGradientName << QLatin1String(")\" ");
//          break;
//      case Qt::NoBrush:
//          stateStream << QLatin1String("fill=\"none\" ");
//          d_func()->attributes.fill = QLatin1String("none");
//          d_func()->attributes.fillOpacity = QString();
//          return;
//          break;
///////////////////////////////////////////////////////////////////////////////

        default:
           break;
        }
        return qs;
    }

///////////////////////////////////////////////////////////////////////////////
// THIS FUNCTION IS NEVER USED and in the future should probably be replaced by
// a drawTextItem() override instead
//
//    void qfontToSvg(const QFont &sfont)
//    {
//        Q_D(SvgPaintEngine);
//
//        d->font = sfont;
//
//        if (d->font.pixelSize() == -1)
//            d->attributes.font_size = QString::number(d->font.pointSizeF() * d->resolution / 72);
//        else
//            d->attributes.font_size = QString::number(d->font.pixelSize());
//
//        int svgWeight = d->font.weight();
//        switch (svgWeight) {
//        case QFont::Light:
//            svgWeight = 100;
//            break;
//        case QFont::Normal:
//            svgWeight = 400;
//            break;
//        case QFont::Bold:
//            svgWeight = 700;
//            break;
//        default:
//            svgWeight *= 10;
//        }
//
//        d->attributes.font_weight = QString::number(svgWeight);
//        d->attributes.font_family = d->font.family();
//        d->attributes.font_style = d->font.italic() ? QLatin1String("italic") : QLatin1String("normal");
//
//        stateStream << "font-family=\"" << d->attributes.font_family << "\" "
//                       "font-size=\"" << d->attributes.font_size << "\" "
//                       "font-weight=\"" << d->attributes.font_weight << "\" "
//                       "font-style=\"" << d->attributes.font_style << "\" "
//                    << endl;
//    }
///////////////////////////////////////////////////////////////////////////////
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
        return qRound(d->engine->size().height() / Ms::DPMM);
    case QPaintDevice::PdmWidthMM:
        return qRound(d->engine->size().width()  / Ms::DPMM);
    case QPaintDevice::PdmNumColors:
        return 0xffffffff;
    case QPaintDevice::PdmPhysicalDpiX:
        return d->engine->resolution();
    case QPaintDevice::PdmPhysicalDpiY:
        return d->engine->resolution();
    case QPaintDevice::PdmDevicePixelRatio:
    case QPaintDevice::PdmDevicePixelRatioScaled:
        return 1;
    default:
        qWarning("SvgGenerator::metric(), unhandled metric %d\n", metric);
        break;
    }
    return 0;
}

/*!
    setElement() function
    Sets the _element variable in SvgPaintEngine.
    Called by saveSVG() in mscore/file.cpp.
*/
void SvgGenerator::setElement(const Ms::Element* e) {
    static_cast<SvgPaintEngine*>(paintEngine())->_element = e;
}

/*****************************************************************************
 * class SvgPaintEngine
 */

bool SvgPaintEngine::begin(QPaintDevice *)
{
    Q_D(SvgPaintEngine);

    // Check for errors
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

    // Stream the headers
    d->stream = new QTextStream(&d->header);
    stream() << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl << SVG_BEGIN;
    if (d->viewBox.isValid()) {
        // viewBox has floating point values, size width/height is integer
        stream() << SVG_WIDTH    << d->viewBox.width()  << SVG_PX << SVG_QUOTE
                 << SVG_HEIGHT   << d->viewBox.height() << SVG_PX << SVG_QUOTE;

        stream() << SVG_VIEW_BOX << d->viewBox.left()
                 << SVG_SPACE    << d->viewBox.top()
                 << SVG_SPACE    << d->viewBox.width()
                 << SVG_SPACE    << d->viewBox.height() << SVG_QUOTE << endl;
    }
    stream() << " xmlns=\"http://www.w3.org/2000/svg\""
                " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
                " version=\"1.2\" baseProfile=\"tiny\">" << endl;
    if (!d->attributes.title.isEmpty()) {
        stream() << SVG_TITLE_BEGIN << d->attributes.title.toHtmlEscaped() << SVG_TITLE_END << endl;
    }
    if (!d->attributes.description.isEmpty()) {
        stream() << SVG_DESC_BEGIN  << d->attributes.description.toHtmlEscaped() << SVG_DESC_END << endl;
    }

// <defs> is currently empty. It's necessary for gradients.
//    d->stream->setString(&d->defs);
//    *d->stream << "<defs>\n";

    // Point the stream at the body string, for other functions to populate
    d->stream->setString(&d->body);
    return true;
}

bool SvgPaintEngine::end()
{
    Q_D(SvgPaintEngine);

// <defs> is currently empty
//    d->stream->setString(&d->defs);
//    stream() << "</defs>\n";

    // Point the stream at the real output device (the .svg file)
    d->stream->setDevice(d->outputDevice);

#ifndef QT_NO_TEXTCODEC
    d->stream->setCodec(QTextCodec::codecForName("UTF-8"));
#endif

    // Stream our strings out to the device, in order
    stream() << d->header;
//    stream() << d->defs;
    stream() << d->body;
    stream() << SVG_END << endl;

    delete d->stream;
    return true;
}

void SvgPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm,
                                 const QRectF &sr)
{
    drawImage(r, pm.toImage(), sr);
}

void SvgPaintEngine::drawImage(const QRectF  &r, const QImage &image,
                               const QRectF &sr, Qt::ImageConversionFlag flags)
{
    Q_UNUSED(sr);
    Q_UNUSED(flags);

    stream() << SVG_IMAGE           << stateString
             << SVG_X << SVG_QUOTE  << r.x() + _dx << SVG_QUOTE
             << SVG_Y << SVG_QUOTE  << r.y() + _dy << SVG_QUOTE
             << SVG_WIDTH           << r.width()   << SVG_QUOTE
             << SVG_HEIGHT          << r.height()  << SVG_QUOTE
             << SVG_PRESERVE_ASPECT << SVG_NONE    << SVG_QUOTE;

    QByteArray      data;
    QBuffer buffer(&data);

    buffer.open(QBuffer::ReadWrite);
    image.save(&buffer, "PNG");
    buffer.close();

    stream() << " xlink:href=\"data:image/png;base64,"
             << data.toBase64() << SVG_QUOTE << SVG_ELEMENT_END << endl;
}

void SvgPaintEngine::updateState(const QPaintEngineState &s)
{
    // Always start fresh
    stateString.clear();

    // stateString = Attribute Settings

    // SVG class attribute, based on Ms::ElementType
    stateStream << SVG_CLASS << getClass(_element) << SVG_QUOTE;

    // Brush and Pen attributes
    stateStream << qbrushToSvg(s.brush());
    stateStream <<   qpenToSvg(s.pen());

// TBD:  "opacity" attribute: Is it ever used?
//       Or is opacity determined by fill-opacity & stroke-opacity instead?
// PLUS: qFuzzyIsNull() is not officially supported in Qt.
//       Should probably use QFuzzyCompare() instead.
    if (!qFuzzyIsNull(s.opacity() - 1))
        stateStream << SVG_OPACITY << s.opacity() << SVG_QUOTE;

    // Translations, SVG transform="translate()", are handled separately from
    // other transformations such as rotation. Qt translates everything, but
    // other transformations do occur, and must be handled here.
    QTransform t = s.transform();

    // Tablature Note Text:
    // m11 and m22 have floating point flotsam, for example: 1.000000629
    // Both values should be == integer 1, because no scaling is intended.
    // So round them to three decimal places, as MuseScore does elsewhere.
    const qreal m11 = qRound(t.m11() * 1000) / 1000.0;
    const qreal m22 = qRound(t.m22() * 1000) / 1000.0;

    if (m11 == 1 && m22 == 1   // No scaling
      && t.m12() == t.m21()) { // No rotation, etc.
          // No transformation except translation
          _dx = t.m31();
          _dy = t.m32();
    }
    else {
          // Other transformations are more straightforward with a full matrix
          _dx = 0;
          _dy = 0;
          stateStream << SVG_MATRIX << t.m11() << SVG_COMMA
                                    << t.m12() << SVG_COMMA
                                    << t.m21() << SVG_COMMA
                                    << t.m22() << SVG_COMMA
                                    << t.m31() << SVG_COMMA
                                    << t.m32() << SVG_RPAREN_QUOTE;
    }

}

void SvgPaintEngine::drawPath(const QPainterPath &p)
{
    stream() << SVG_PATH << stateString;

    // fill-rule is here because UpdateState() doesn't have a QPainterPath arg
    // Majority of <path>s use the default value: fill-rule="nonzero"
    if (p.fillRule() == Qt::OddEvenFill)
        stream() << SVG_FILL_RULE;

    // Path data
    stream() << SVG_D;
    for (int i = 0; i < p.elementCount(); ++i) {
        const QPainterPath::Element &e = p.elementAt(i);
                               qreal x = e.x + _dx;
                               qreal y = e.y + _dy;
        switch (e.type) {
        case QPainterPath::MoveToElement:
            stream() << SVG_MOVE  << x << SVG_COMMA << y;
            break;
        case QPainterPath::LineToElement:
            stream() << SVG_LINE  << x << SVG_COMMA << y;
            break;
        case QPainterPath::CurveToElement:
            stream() << SVG_CURVE << x << SVG_COMMA << y;
            ++i;
            while (i < p.elementCount()) {
                const QPainterPath::Element &ee = p.elementAt(i);
                if (ee.type == QPainterPath::CurveToDataElement) {
                    stream() << SVG_SPACE << ee.x + _dx
                             << SVG_COMMA << ee.y + _dy;
                    ++i;
                }
                else {
                    --i;
                    break;
                }
            }
            break;
        default:
            break;
        }
        if (i <= p.elementCount() - 1)
            stream() << SVG_SPACE;
    }
    stream() << SVG_QUOTE << SVG_ELEMENT_END << endl;
}

void SvgPaintEngine::drawPolygon(const QPointF *points, int pointCount,
                                  PolygonDrawMode mode)
{
    Q_ASSERT(pointCount >= 2);

    QPainterPath path(points[0]);
    for (int i=1; i<pointCount; ++i)
        path.lineTo(points[i]);

    if (mode == PolylineMode) {
        stream() << SVG_PATH << stateString
                 << SVG_D_M;
        for (int i = 0; i < pointCount; ++i) {
            const QPointF &pt = points[i];
            stream() << pt.x() + _dx << SVG_COMMA << pt.y() + _dy;
            if (i != pointCount - 1)
                stream() << SVG_SPACE;
        }
        stream() << SVG_QUOTE << SVG_ELEMENT_END <<endl;
    }
    else {
        path.closeSubpath();
        drawPath(path);
    }
}
