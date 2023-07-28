/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "image.h"

#include "io/file.h"
#include "io/fileinfo.h"

#include "draw/types/pixmap.h"
#include "draw/types/transform.h"
#include "draw/svgrenderer.h"

#include "imageStore.h"
#include "masterscore.h"
#include "mscore.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   propertyList
//---------------------------------------------------------

static bool defaultAutoScale        = false;
static bool defaultLockAspectRatio  = true;
static bool defaultSizeIsSpatium    = true;

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(EngravingItem* parent)
    : BSymbol(ElementType::IMAGE, parent, ElementFlag::MOVABLE)
{
    m_imageType        = ImageType::NONE;
    m_size            = SizeF(0.0, 0.0);
    m_storeItem       = 0;
    m_dirty           = false;
    m_lockAspectRatio = defaultLockAspectRatio;
    m_autoScale       = defaultAutoScale;
    m_sizeIsSpatium   = defaultSizeIsSpatium;
    m_linkIsValid     = false;
}

Image::Image(const Image& img)
    : BSymbol(img)
{
    m_imageType        = img.m_imageType;
    m_buffer           = img.m_buffer;
    m_size            = img.m_size;
    m_lockAspectRatio = img.m_lockAspectRatio;
    m_autoScale       = img.m_autoScale;
    m_dirty           = img.m_dirty;
    m_storeItem       = img.m_storeItem;
    m_sizeIsSpatium   = img.m_sizeIsSpatium;
    if (m_storeItem) {
        m_storeItem->reference(this);
    }
    m_linkPath        = img.m_linkPath;
    m_linkIsValid     = img.m_linkIsValid;
    if (m_imageType == ImageType::RASTER) {
        m_rasterDoc = img.m_rasterDoc ? std::make_shared<Pixmap>(*img.m_rasterDoc) : nullptr;
    } else if (m_imageType == ImageType::SVG) {
        m_svgDoc = img.m_svgDoc ? new SvgRenderer(m_storeItem->buffer()) : 0;
    }
    setZ(img.z());
}

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::~Image()
{
    if (m_storeItem) {
        m_storeItem->dereference(this);
    }
    if (m_imageType == ImageType::SVG) {
        delete m_svgDoc;
    }
}

//---------------------------------------------------------
//   setImageType
//---------------------------------------------------------

void Image::setImageType(ImageType t)
{
    m_imageType = t;
    if (m_imageType == ImageType::SVG) {
        m_svgDoc = 0;
    } else if (m_imageType == ImageType::RASTER) {
        m_rasterDoc.reset();
    } else {
        LOGD("illegal image type");
    }
}

//---------------------------------------------------------
//   imageSize
//---------------------------------------------------------

SizeF Image::imageSize() const
{
    if (!isValid()) {
        return SizeF();
    }

    if (m_imageType == ImageType::RASTER) {
        Size rasterSize = m_rasterDoc->size();
        return SizeF(rasterSize.width(), rasterSize.height());
    }

    return m_svgDoc->defaultSize();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Image::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    bool emptyImage = false;
    if (m_imageType == ImageType::SVG) {
        if (!m_svgDoc) {
            emptyImage = true;
        } else {
            m_svgDoc->render(painter, bbox());
        }
    } else if (m_imageType == ImageType::RASTER) {
        if (m_rasterDoc == nullptr) {
            emptyImage = true;
        } else {
            painter->save();
            SizeF s;
            if (m_sizeIsSpatium) {
                s = m_size * spatium();
            } else {
                s = m_size * DPMM;
            }
            if (score() && score()->printing() && !MScore::svgPrinting) {
                // use original image size for printing, but not for svg for reasonable file size.
                painter->scale(s.width() / m_rasterDoc->width(), s.height() / m_rasterDoc->height());
                painter->drawPixmap(PointF(0, 0), *m_rasterDoc);
            } else {
                Transform t = painter->worldTransform();
                Size ss = Size(s.width() * t.m11(), s.height() * t.m22());
                t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
                painter->setWorldTransform(t);
                if ((m_buffer.size() != ss || m_dirty) && m_rasterDoc && !m_rasterDoc->isNull()) {
                    m_buffer = imageProvider()->scaled(*m_rasterDoc, ss);
                    m_dirty = false;
                }
                if (m_buffer.isNull()) {
                    emptyImage = true;
                } else {
                    painter->drawPixmap(PointF(0.0, 0.0), m_buffer);
                }
            }
            painter->restore();
        }
    }

    if (emptyImage) {
        painter->setBrush(mu::draw::BrushStyle::NoBrush);
        painter->setPen(engravingConfiguration()->defaultColor());
        painter->drawRect(bbox());
        painter->drawLine(0.0, 0.0, bbox().width(), bbox().height());
        painter->drawLine(bbox().width(), 0.0, 0.0, bbox().height());
    }
    if (selected() && !(score() && score()->printing())) {
        painter->setBrush(mu::draw::BrushStyle::NoBrush);
        painter->setPen(engravingConfiguration()->selectionColor());
        painter->drawRect(bbox());
    }
}

//---------------------------------------------------------
//   isImageFramed
//---------------------------------------------------------

bool Image::isImageFramed() const
{
    if (!explicitParent()) {
        return false;
    }

    return explicitParent()->isBox();
}

//---------------------------------------------------------
//   imageAspectRatio
//---------------------------------------------------------

double Image::imageAspectRatio() const
{
    return m_size.width() / m_size.height();
}

//---------------------------------------------------------
//   setImageHeight
//---------------------------------------------------------

void Image::updateImageHeight(const double& height)
{
    double aspectRatio = imageAspectRatio();

    m_size.setHeight(height);

    if (m_lockAspectRatio) {
        m_size.setWidth(height * aspectRatio);
    }
}

//---------------------------------------------------------
//   setImageWidth
//---------------------------------------------------------

void Image::updateImageWidth(const double& width)
{
    double aspectRatio = imageAspectRatio();

    m_size.setWidth(width);

    if (m_lockAspectRatio) {
        m_size.setHeight(width / aspectRatio);
    }
}

//---------------------------------------------------------
//   imageHeight
//---------------------------------------------------------

double Image::imageHeight() const
{
    return m_size.height();
}

//---------------------------------------------------------
//   imageWidth
//---------------------------------------------------------

double Image::imageWidth() const
{
    return m_size.width();
}

bool Image::load()
{
    // once all paths are read, load img or retrieve it from store
    // loading from file is tried first to update the stored image, if necessary

    io::path_t path;
    bool loaded = false;
    // if a store path is given, attempt to get the image from the store
    if (!m_storePath.isEmpty()) {
        m_storeItem = imageStore.getImage(m_storePath);
        if (m_storeItem) {
            m_storeItem->reference(this);
            loaded = true;
        }
        // if no image in store, attempt to load from path (for backward compatibility)
        else {
            loaded = load(m_storePath);
        }
        path = m_storePath;
    }
    // if no success from store path, attempt loading from link path (for .mscx files)
    if (!loaded) {
        loaded = load(m_linkPath);
        m_linkIsValid = loaded;
        path = m_linkPath;
    }

    if (path.withSuffix("svg")) {
        setImageType(ImageType::SVG);
    } else {
        setImageType(ImageType::RASTER);
    }

    return loaded;
}

//---------------------------------------------------------
//   load
//    load image from file and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::load(const io::path_t& ss)
{
    if (ss.empty()) {
        return false;
    }

    io::path_t path(ss);
    // if file path is relative, prepend score path
    FileInfo fi(path);
    if (fi.isRelative()) {
        path = masterScore()->fileInfo()->absoluteDirPath() + '/' + path;
        fi = FileInfo(path);
    }

    m_linkIsValid = false;                       // assume link fname is invalid
    File f(path);
    if (!f.open(IODevice::ReadOnly)) {
        LOGD() << "failed load file: " << path;
        return false;
    }
    ByteArray ba = f.readAll();
    f.close();

    m_linkIsValid = true;
    m_linkPath = fi.canonicalFilePath();
    m_storeItem = imageStore.add(m_linkPath, ba);
    m_storeItem->reference(this);
    if (path.withSuffix("svg")) {
        setImageType(ImageType::SVG);
    } else {
        setImageType(ImageType::RASTER);
    }
    return true;
}

//---------------------------------------------------------
//   loadFromData
//    load image from data and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::loadFromData(const path_t& name, const ByteArray& ba)
{
    m_linkIsValid = false;
    m_linkPath = u"";
    m_storeItem = imageStore.add(name, ba);
    m_storeItem->reference(this);
    if (name.withSuffix("svg")) {
        setImageType(ImageType::SVG);
    } else {
        setImageType(ImageType::RASTER);
    }
    return true;
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void Image::startEditDrag(EditData& data)
{
    BSymbol::startEditDrag(data);
    ElementEditDataPtr eed = data.getData(this);

    eed->pushProperty(Pid::SIZE);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Image::editDrag(EditData& ed)
{
    double ratio = m_size.width() / m_size.height();
    double dx = ed.delta.x();
    double dy = ed.delta.y();
    if (m_sizeIsSpatium) {
        double _spatium = spatium();
        dx /= _spatium;
        dy /= _spatium;
    } else {
        dx /= DPMM;
        dy /= DPMM;
    }
    if (ed.curGrip == Grip::START) {
        m_size.setWidth(m_size.width() + dx);
        if (m_lockAspectRatio) {
            m_size.setHeight(m_size.width() / ratio);
        }
    } else {
        m_size.setHeight(m_size.height() + dy);
        if (m_lockAspectRatio) {
            m_size.setWidth(m_size.height() * ratio);
        }
    }

    rendering()->layoutItem(this);
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> Image::gripsPositions(const EditData&) const
{
    RectF r(pageBoundingRect());
    return {
        PointF(r.x() + r.width(), r.y() + r.height() * .5),
        PointF(r.x() + r.width() * .5, r.y() + r.height())
    };
}

//---------------------------------------------------------
//   pixel2Size
//---------------------------------------------------------

SizeF Image::pixel2size(const SizeF& s) const
{
    return s / (m_sizeIsSpatium ? spatium() : DPMM);
}

//---------------------------------------------------------
//   size2pixel
//---------------------------------------------------------

SizeF Image::size2pixel(const SizeF& s) const
{
    return s * (m_sizeIsSpatium ? spatium() : DPMM);
}

void Image::init()
{
    if (m_imageType == ImageType::SVG && !m_svgDoc) {
        if (m_storeItem) {
            m_svgDoc = new SvgRenderer(m_storeItem->buffer());
        }
    } else if (m_imageType == ImageType::RASTER && !m_rasterDoc) {
        if (m_storeItem) {
            m_rasterDoc = imageProvider()->createPixmap(m_storeItem->buffer());
            if (!m_rasterDoc->isNull()) {
                m_dirty = true;
            }
        }
    }
    if (m_size.isNull()) {
        m_size = pixel2size(imageSize());
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Image::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::AUTOSCALE:
        return autoScale();
    case Pid::SIZE:
        return PropertyValue::fromValue(size());
    case Pid::IMAGE_HEIGHT:
        return imageHeight();
    case Pid::IMAGE_WIDTH:
        return imageWidth();
    case Pid::IMAGE_FRAMED:
        return isImageFramed();
    case Pid::LOCK_ASPECT_RATIO:
        return lockAspectRatio();
    case Pid::SIZE_IS_SPATIUM:
        return sizeIsSpatium();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Image::setProperty(Pid propertyId, const PropertyValue& v)
{
    bool rv = true;
    score()->addRefresh(canvasBoundingRect());
    switch (propertyId) {
    case Pid::AUTOSCALE:
        setAutoScale(v.toBool());
        break;
    case Pid::SIZE:
        setSize(v.value<SizeF>());
        break;
    case Pid::IMAGE_HEIGHT:
        updateImageHeight(v.toDouble());
        break;
    case Pid::IMAGE_WIDTH:
        updateImageWidth(v.toDouble());
        break;
    case Pid::IMAGE_FRAMED:
        break;
    case Pid::LOCK_ASPECT_RATIO:
        setLockAspectRatio(v.toBool());
        break;
    case Pid::SIZE_IS_SPATIUM: {
        SizeF s = size2pixel(m_size);
        setSizeIsSpatium(v.toBool());
        m_size = pixel2size(s);
    }
    break;
    default:
        rv = EngravingItem::setProperty(propertyId, v);
        break;
    }
    setGenerated(false);
    m_dirty = true;
    triggerLayout();
    return rv;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Image::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::AUTOSCALE:
        return defaultAutoScale;
    case Pid::SIZE:
        return PropertyValue::fromValue(pixel2size(imageSize()));
    case Pid::IMAGE_HEIGHT:
        return pixel2size(imageSize()).height();
    case Pid::IMAGE_WIDTH:
        return pixel2size(imageSize()).width();
    case Pid::IMAGE_FRAMED:
        return isImageFramed();
    case Pid::LOCK_ASPECT_RATIO:
        return defaultLockAspectRatio;
    case Pid::SIZE_IS_SPATIUM:
        return defaultSizeIsSpatium;
    default:
        return EngravingItem::propertyDefault(id);
    }
}
}
